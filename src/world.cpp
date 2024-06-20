#include "world.hpp"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <unordered_set>
#include "chunk.hpp"
#include "chunk_renderer.hpp"
#include "chunk_worker.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "world_gen.hpp"

World::World() {
  world_gen = std::make_unique<WorldGen>(*this);

  for (size_t i = 0; i < 2; i += 1) {
    chunk_terrain_gen_workers.push_back(new ChunkTerrainGenWorker(world_gen));
  }

  lightning_worker = std::make_shared<ChunkLightningWorker>();
}

World::~World() {
  // for (size_t i = 0; i < chunk_mesh_workers.size(); i += 1) {
  //   delete chunk_mesh_workers[i];
  // }

  for (size_t i = 0; i < chunk_terrain_gen_workers.size(); i += 1) {
    delete chunk_terrain_gen_workers[i];
  }
}

const Player* World::get_player() const {
  return player;
}

void World::set_cube(CubePos cube_pos, CubeId to) {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
  assert(is_local_pos_valid(local_pos));

  const Chunk* chunk = get_chunk(chunk_pos);
  if (!chunk) { return; }
  ((Chunk*)chunk)->set_cube(local_pos, to);
}

CubeId World::get_cube(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
  assert(is_local_pos_valid(local_pos));

  const Chunk* chunk = get_chunk(chunk_pos);
  if (!chunk) { return CubeId::AIR; }
  return ((Chunk*)chunk)->get_cube(local_pos);
}

void World::create_new_chunk(ChunkPos chunk_pos) {
  chunks.emplace(chunk_pos, std::make_unique<Chunk>(*this, chunk_pos));
  auto& chunk = chunks.at(chunk_pos);
  chunks.at(chunk_pos)->renderer = new ChunkRenderer(*chunk.get());
}

bool World::is_solid(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
  assert(is_local_pos_valid(local_pos));

  const Chunk* chunk = get_chunk(chunk_pos);
  if (!chunk) { return false; }
  return chunk->is_solid(local_pos);
}

bool World::has_solid_neigbour(CubePos world_position) const {
  return (is_solid(world_position + CubePos{-1, 0, 0}) || is_solid(world_position + CubePos{1, 0, 0}) || is_solid(world_position + CubePos{0, -1, 0}) ||
          is_solid(world_position + CubePos{0, 1, 0}) || is_solid(world_position + CubePos{0, 0, -1}) || is_solid(world_position + CubePos{0, 0, 1}));
}

bool World::is_solid(glm::vec<3, float> world_position_f) const {
  return is_solid(CubePos{std::floor(world_position_f.x), std::floor(world_position_f.y), std::floor(world_position_f.z)});
}

WorldPos World::query_raycast_solid(glm::vec<3, float> from, glm::vec<3, float> to) const {
  const float length = glm::length((to - from));
  const int steps = std::ceil(length) * 25;

  std::optional<glm::vec<3, float>> prev_pos{};

  for (int s = 0; s <= steps; s += 1) {
    glm::vec<3, float> pos = {
        std::lerp(from.x, to.x, (float)s / (float)steps),
        std::lerp(from.y, to.y, (float)s / (float)steps),
        std::lerp(from.z, to.z, (float)s / (float)steps)};

    if (is_solid(pos)) {
      return prev_pos.value_or(pos);
    }

    prev_pos = pos;
  }
  return to;
}

// Get sunlight of a cube by checking how much space there is above it
// FIXME: this isn't really accurate as we should be using inverse of heightmap (lowmap?) to get the CLOSEST cube to our cube
uint8_t World::get_sunlight(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);

  if (!chunks.contains(chunk_pos)) { return 0; }
  // if (!chunks.at(chunk_pos).get_heightmap(local_pos.x, local_pos.z).has_value()) { return 0; }
  if (chunks.at(chunk_pos)->get_heightmap(local_pos.x, local_pos.z).value_or(0) > local_pos.y) { return chunks.at(chunk_pos)->get_heightmap(local_pos.x, local_pos.z).value() - local_pos.y - 1; }
  int16_t value = CHUNK_SIZE - 1 - local_pos.y;

  for (size_t i = 1; i < 10; i += 1) {
    // The chunk is empty, can't block light
    if (!chunks.contains(chunk_pos + ChunkPos{0, i, 0})) {
      value += 16;
      continue;
    }
    // The chunk's column is empty, can't block light
    if (!chunks.at(chunk_pos + ChunkPos{0, i, 0})->get_heightmap(local_pos.x, local_pos.z).has_value()) {
      value += 16;
      continue;
    }
    // Chunk is blocking light, add distance from bottom of the chunk to the blocking cube and return
    value += chunks.at(chunk_pos + ChunkPos{0, i, 0})->get_heightmap(local_pos.x, local_pos.z).value();
    return std::clamp(value - 16, 0, 255);
  }

  return 255;
}

// The y position is used to determine the chunk
std::optional<uint16_t> World::get_heightmap(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);

  if (!chunks.contains(chunk_pos)) { return 0; }
  return chunks.at(chunk_pos)->get_heightmap(local_pos.x, local_pos.z);
}

LightLevel World::get_lightmap(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
  assert(is_local_pos_valid(local_pos));

  if (!chunks.contains(chunk_pos)) { return LightLevel{}; }
  return chunks.at(chunk_pos)->get_lightmap(local_pos);
}

// Returns cube position of first cuube in raycast, or empty optional if no solid cube encountered
std::optional<CubePos> World::raycast_get_solid_cube(glm::vec<3, float> from, glm::vec<3, float> to) const {
  const float length = glm::length((to - from));
  const int steps = std::ceil(length) * 50;

  std::optional<glm::vec<3, float>> prev_pos{};

  for (int s = 0; s <= steps; s += 1) {
    glm::vec<3, float> pos = {
        std::lerp(from.x, to.x, (float)s / (float)steps),
        std::lerp(from.y, to.y, (float)s / (float)steps),
        std::lerp(from.z, to.z, (float)s / (float)steps)};

    if (is_solid(pos)) {
      return floor_position(pos);
    }

    prev_pos = pos;
  }
  return {};
}

// Returns world positions intersecting the raycast. They can be out of bounds.
std::unordered_set<CubePos, Vec3Hasher> World::raycast_get_overlapping_cubes(WorldPos from, WorldPos to) const {
  // FIXME: use DDA here
  // FIXME: add tolerance argument to return only cubes that are close to ray
  const float length = glm::length((to - from));
  const int steps = std::ceil(length) * 50;

  std::unordered_set<CubePos, Vec3Hasher> set;

  for (int s = 0; s <= steps; s += 1) {
    glm::vec<3, float> pos = {
        std::lerp(from.x, to.x, (float)s / (float)steps),
        std::lerp(from.y, to.y, (float)s / (float)steps),
        std::lerp(from.z, to.z, (float)s / (float)steps)};

    set.emplace(pos);
  }
  return set;
}

const Chunk* World::get_chunk(ChunkPos chunk_pos) const {
  const auto& chunk_it = chunks.find(chunk_pos);
  if (chunk_it == chunks.end()) {
    return nullptr;
  }
  auto& chunk = chunk_it->second;
  if (!chunk->flags.ready) {
    return nullptr;
  }

  return chunk.get();
}

void World::update_light(CubePos cube_pos) {
  return;
  // ChunkPos chunk_pos = world_pos_to_chunk_pos(cube_pos);
  // int world_y = get_heightmap(cube_pos).value_or(0) * chunk_pos.y;

  // spread_light({cube_pos.x, world_y, cube_pos.z});
  // spread_light({cube_pos.x, cube_pos.y, cube_pos.z});
}

void World::spread_light(const CubePos light_cube_pos, std::unordered_set<CubePos, Vec3Hasher>& _) {
  uint8_t light = get_sunlight(light_cube_pos);

  std::unordered_set<CubePos, Vec3Hasher> visited_cubes;
  std::unordered_set<CubePos, Vec3Hasher> queued_cubes;

  queued_cubes.insert(light_cube_pos + CubePos{-1, 0, 0});
  queued_cubes.insert(light_cube_pos + CubePos{1, 0, 0});
  queued_cubes.insert(light_cube_pos + CubePos{0, -1, 0});
  queued_cubes.insert(light_cube_pos + CubePos{0, 1, 0});
  queued_cubes.insert(light_cube_pos + CubePos{0, 0, -1});
  queued_cubes.insert(light_cube_pos + CubePos{0, 0, 1});
  int failsafe = 5000;

  while (!queued_cubes.empty() && failsafe-- >= 0) {
    CubePos cube_pos = *queued_cubes.begin();
    queued_cubes.erase(queued_cubes.begin());
    visited_cubes.emplace(cube_pos);

    auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
    Chunk* chunk = const_cast<Chunk*>(get_chunk(chunk_pos));
    if (!chunk) { continue; }
    if (is_solid(cube_pos)) { continue; }
    int distance = std::abs(cube_pos.x - light_cube_pos.x) + std::abs(cube_pos.y - light_cube_pos.y) + std::abs(cube_pos.z - light_cube_pos.z);
    if (distance > 7) { continue; }

    LightLevel current_light_level = chunk->get_lightmap(local_pos);
    LightLevel new_light_level = {light - std::min((int)light, distance * 32), 0, 0};
    if (current_light_level.x >= new_light_level.x) { continue; }
    chunk->set_lightmap(local_pos, new_light_level);

    if (!visited_cubes.contains(cube_pos + CubePos(-1, 0, 0))) {
      queued_cubes.emplace(cube_pos + CubePos(-1, 0, 0));
    }
    if (!visited_cubes.contains(cube_pos + CubePos(1, 0, 0))) {
      queued_cubes.emplace(cube_pos + CubePos(1, 0, 0));
    }
    if (!visited_cubes.contains(cube_pos + CubePos(0, -1, 0))) {
      queued_cubes.emplace(cube_pos + CubePos(0, -1, 0));
    }
    if (!visited_cubes.contains(cube_pos + CubePos(0, 1, 0))) {
      queued_cubes.emplace(cube_pos + CubePos(0, 1, 0));
    }
    if (!visited_cubes.contains(cube_pos + CubePos(0, 0, -1))) {
      queued_cubes.emplace(cube_pos + CubePos(0, 0, -1));
    }
    if (!visited_cubes.contains(cube_pos + CubePos(0, 0, 1))) {
      queued_cubes.emplace(cube_pos + CubePos(0, 0, 1));
    }
  }
}

void World::request_player_chunk_light_update() {
  std::unordered_set<CubePos, Vec3Hasher> visited_cubes;
  if (!get_player()) { return; }
  auto chunk_pos = world_pos_to_chunk_pos(get_player()->world_pos);
  auto chunk_it = chunks.find(chunk_pos);
  if (chunk_it == chunks.end()) { return; }

  request_chunk_light_update(chunk_pos);
}

void World::request_chunk_light_update(ChunkPos chunk_pos) {
  Chunk* chunk = const_cast<Chunk*>(get_chunk(chunk_pos));
  if (!chunk) { return; }
  lightning_worker->add_to_queue(chunk);
}

std::unordered_map<uint32_t, CubeId> World::get_neigbours(CubePos _cube_pos, bool edges, bool corners) const {
  std::unordered_map<uint32_t, CubeId> map;

  for (int x = -1; x <= 1; x += 1) {
    for (int y = -1; y <= 1; y += 1) {
      for (int z = -1; z <= 1; z += 1) {
        if (x == 0 && y == 0 && z == 0) { continue; }
        if (!corners && x != 0 && y != 0 && z != 0) { continue; }
        if (!edges && x != 0 && y != 0 && z == 0) { continue; }
        if (!edges && x != 0 && z != 0 && y == 0) { continue; }
        if (!edges && y != 0 && z != 0 && x == 0) { continue; }

        LocalPos cube_pos_neigbour = _cube_pos + LocalPos{x, y, z};

        uint32_t dir = 0;
        if (x == -1) { dir += Dir::LEFT; }
        if (x == 1) { dir += Dir::RIGHT; }
        if (y == -1) { dir += Dir::BOTTOM; }
        if (y == 1) { dir += Dir::TOP; }
        if (z == -1) { dir += Dir::FRONT; }
        if (z == 1) { dir += Dir::BACK; }

        map.insert({dir, get_cube(cube_pos_neigbour)});
      }
    }
  }

  return map;
}

bool World::is_chunk_ready(ChunkPos chunk_pos) {
  const Chunk* chunk = get_chunk(chunk_pos);
  if (!chunk) { return false; }
  return chunk->flags.ready;
}

std::unordered_map<uint32_t, CubeId> World::get_neigbours(const Chunk& chunk, LocalPos _local_pos, bool edges, bool corners) const {
  std::unordered_map<uint32_t, CubeId> map;

  auto _get_neigbour = [&](int x, int y, int z) {
    if (!corners && x != 0 && y != 0 && z != 0) { return; }
    if (!edges) {
      if (x != 0 && y != 0 && z == 0) { return; }
      if (x != 0 && z != 0 && y == 0) { return; }
      if (y != 0 && z != 0 && x == 0) { return; }
    }

    uint32_t dir = 0;
    if (x == -1) { dir += Dir::LEFT; }
    if (x == 1) { dir += Dir::RIGHT; }
    if (y == -1) { dir += Dir::BOTTOM; }
    if (y == 1) { dir += Dir::TOP; }
    if (z == -1) { dir += Dir::FRONT; }
    if (z == 1) { dir += Dir::BACK; }

    LocalPos local_pos_neigbour = _local_pos + LocalPos{x, y, z};

    if (is_local_pos_valid(local_pos_neigbour)) {
      map.insert({dir, chunk.get_cube(local_pos_neigbour)});
    } else {
      CubePos cube_pos_neigbour = local_pos_to_cube_pos(chunk.position, local_pos_neigbour);
      map.insert({dir, get_cube(cube_pos_neigbour)});
    }
  };

  _get_neigbour(-1, -1, -1);
  _get_neigbour(-1, -1, 0);
  _get_neigbour(-1, -1, 1);
  _get_neigbour(-1, 0, -1);
  _get_neigbour(-1, 0, 0);
  _get_neigbour(-1, 0, 1);
  _get_neigbour(-1, 1, -1);
  _get_neigbour(-1, 1, 0);
  _get_neigbour(-1, 1, 1);

  _get_neigbour(0, -1, -1);
  _get_neigbour(0, -1, 0);
  _get_neigbour(0, -1, 1);
  _get_neigbour(0, 0, -1);
  _get_neigbour(0, 0, 1);
  _get_neigbour(0, 1, -1);
  _get_neigbour(0, 1, 0);
  _get_neigbour(0, 1, 1);

  _get_neigbour(1, -1, -1);
  _get_neigbour(1, -1, 0);
  _get_neigbour(1, -1, 1);
  _get_neigbour(1, 0, -1);
  _get_neigbour(1, 0, 0);
  _get_neigbour(1, 0, 1);
  _get_neigbour(1, 1, -1);
  _get_neigbour(1, 1, 0);
  _get_neigbour(1, 1, 1);

  return map;
}

template <typename T>
void sort_vector_by_distance(std::vector<glm::vec<3, T>>& vector, glm::vec<3, T> pos) {
  using Vec3T = glm::vec<3, T>;
  std::sort(vector.begin(), vector.end(), [&](const Vec3T a, const Vec3T b) {
    Vec3T first = pos - a;
    Vec3T second = pos - b;
    return std::abs(first.x) + std::abs(first.y) + std::abs(first.z) < std::abs(second.x) + std::abs(second.y) + std::abs(second.z);
  });
}

void World::update() {
  lightning_worker->run_job();
  // player = nullptr;

  // Entity update
  for (auto&& entity : entities) {
    entity.get()->update();

    // Find player
    if (Player* _player = dynamic_cast<Player*>(entity.get())) {
      player = _player;
    }
  }

  // Unlock chunks locked by terrain gen worker threads
  for (const auto& worker : chunk_terrain_gen_workers) {
    worker->update();
  }

  // Load chunks near player
  ChunkPos chunk_load_center = (player) ? world_pos_to_chunk_pos(player->world_pos) : ChunkPos{0, 0, 0};
  for (int x = -chunk_load_distance; x <= chunk_load_distance; x += 1) {
    for (int z = -chunk_load_distance; z <= chunk_load_distance; z += 1) {
      for (int y = -chunk_load_distance; y <= chunk_load_distance; y += 1) {
        ChunkPos chunk_pos{x, y, z};
        if (!chunks.contains(chunk_load_center + chunk_pos)) {
          create_new_chunk(chunk_load_center + chunk_pos);
          break;
        }
      }
    }
  }

  //  if (!(chunk.flags.is_write_locked() {chunks.erase(chunk_pos)}

  // Get list of chunks that were modified, so we can update their meshes
  std::vector<ChunkPos> chunks_not_ready;
  std::vector<ChunkPos> chunks_to_be_unloaded;

  for (auto& [chunk_pos, chunk] : chunks) {
    if (!chunk->flags.ready) {
      chunks_not_ready.emplace_back(chunk_pos);
      continue;
    }

    bool chunk_is_too_far = (std::abs(chunk_load_center.x - chunk_pos.x) > chunk_load_distance + 1) || (std::abs(chunk_load_center.y - chunk_pos.y) > chunk_load_distance + 1) || (std::abs(chunk_load_center.z - chunk_pos.z) > chunk_load_distance + 1);

    if (chunk_is_too_far && (!chunk->flags.is_write_locked())) {
      chunks_to_be_unloaded.emplace_back(chunk_pos);
      continue;
    }

    // Set cubes, which chunk neigbours requested to set
    std::unordered_map<CubePos, CubeId, Vec3Hasher> chunks_neigbour_chunks_failed_cubes;
    for (auto [cube_pos, cube_id] : chunk->neigbour_chunks_cubes_to_set) {
      auto [neigb_chunk_pos, neigb_local_pos] = cube_to_local(cube_pos);
      auto neigb_chunk_it = chunks.find(neigb_chunk_pos);

      // Chunk doesn't exist
      if (neigb_chunk_it == chunks.end()) {
        chunks_neigbour_chunks_failed_cubes.insert({cube_pos, cube_id});
        continue;
      }

      auto neigb_chunk = neigb_chunk_it->second.get();

      // Chunk is locked
      if (neigb_chunk->flags.is_write_locked()) {
        chunks_neigbour_chunks_failed_cubes.insert({cube_pos, cube_id});
        continue;
      }

      // Can only replace air block
      if (neigb_chunk->get_cube(neigb_local_pos) != CubeId::AIR) {
        continue;
      }

      neigb_chunk->set_cube(neigb_local_pos, cube_id);
    }
    chunk->neigbour_chunks_cubes_to_set = chunks_neigbour_chunks_failed_cubes;

    chunk->update();
  }

  sort_vector_by_distance(chunks_not_ready, world_pos_to_chunk_pos(player->world_pos));

  size_t i = 0;
  for (const ChunkPos chunk_pos : chunks_not_ready) {
    auto& worker = chunk_terrain_gen_workers[i % chunk_terrain_gen_workers.size()];
    worker->add_to_queue(chunks.at(chunk_pos).get());
    i += 1;
  }

  for (const ChunkPos chunk_pos : chunks_to_be_unloaded) {
    chunks.erase(chunk_pos);
  }
}

std::vector<CubePos> World::aabb_get_overlapping_cubes(AABB aabb, WorldPos world_pos) {
  std::vector<CubePos> overlapping_cubes;
  CubePos min = floor_position(world_pos - aabb.half_extents + aabb.offset);
  CubePos max = floor_position(world_pos + aabb.half_extents + aabb.offset);

  for (auto x = min.x; x <= max.x; x += 1) {
    for (auto y = min.y; y <= max.y; y += 1) {
      for (auto z = min.z; z <= max.z; z += 1) {
        CubePos overlapping_pos = CubePos{x, y, z};
        overlapping_cubes.emplace_back(overlapping_pos);
      }
    }
  }

  return overlapping_cubes;
}

static constexpr AABB full_cube_aabb = AABB{.half_extents = {0.5, 0.5, 0.5}, .offset = {0.0, 0.0, 0.0}};
bool World::aabb_is_overlapping_cube(AABB aabb, WorldPos world_pos, CubePos cube_pos) {
  return AABB::test(
      aabb, world_pos,
      full_cube_aabb, WorldPos{cube_pos} + WorldPos{0.5, 0.5, 0.5});
}

std::vector<CubePos> World::aabb_get_solid_cubes(AABB aabb, WorldPos world_pos) {
  std::vector<CubePos> solid_cubes;
  std::vector<CubePos> overlapping_cubes = aabb_get_overlapping_cubes(aabb, world_pos);

  for (auto cube_pos : overlapping_cubes) {
    auto properties = cube_properties.at(static_cast<size_t>(get_cube(cube_pos)));
    bool collider_overlapping = std::any_of(properties.collider_aabb.cbegin(), properties.collider_aabb.cend(), [&](auto& cube_aabb) {
      return AABB::test(aabb, world_pos, cube_aabb, WorldPos{cube_pos} + WorldPos{0.5, 0.5, 0.5});
    });
    if (collider_overlapping) {
      solid_cubes.emplace_back(cube_pos);
    }
  }

  return solid_cubes;
}