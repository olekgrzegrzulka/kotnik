#include "world.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <unordered_set>
#include "chunk.hpp"
#include "chunk_worker.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "random.hpp"
#include "world_gen.hpp"

World::World() {
  i32 seed = StaticRandom::get().next<i32>();
  debug_log("Created world with seed ", seed);
  world_gen = std::make_unique<WorldGen>(*this, seed);

  for (size_t i = 0; i < 1; i += 1) {
    chunk_terrain_gen_workers.push_back(std::make_unique<ChunkTerrainGenWorker>(world_gen));
  }
}

World::~World() {
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

NeigbourCubeIds World::get_neigbour_ids(CubePos cube_pos, bool edges, bool corners) const {

  auto get_neigbour_ = [&](CubePos offset_pos) -> CubeId {
    auto [chunk_pos, local_pos] = cube_to_local(cube_pos + offset_pos);

    Chunk* c = get_chunk(chunk_pos);

    if (!c) return CubeId::AIR;

    return c->get_cube(local_pos);
  };

  NeigbourCubeIds neigbours{};

  neigbours.center = get_neigbour_({0, 0, 0});

  // Straight neigbours
  neigbours.left = get_neigbour_({-1, 0, 0});
  neigbours.right = get_neigbour_({+1, 0, 0});
  neigbours.bottom = get_neigbour_({0, -1, 0});
  neigbours.top = get_neigbour_({0, +1, 0});
  neigbours.front = get_neigbour_({0, 0, -1});
  neigbours.back = get_neigbour_({0, 0, +1});

  // Edge neighbours
  if (edges) {
    neigbours.left_bottom = get_neigbour_({-1, -1, 0});
    neigbours.right_bottom = get_neigbour_({+1, -1, 0});
    neigbours.front_bottom = get_neigbour_({0, -1, -1});
    neigbours.back_bottom = get_neigbour_({0, -1, +1});

    neigbours.left_top = get_neigbour_({-1, +1, 0});
    neigbours.right_top = get_neigbour_({+1, +1, 0});
    neigbours.front_top = get_neigbour_({0, +1, -1});
    neigbours.back_top = get_neigbour_({0, +1, +1});

    neigbours.left_front = get_neigbour_({-1, 0, -1});
    neigbours.right_front = get_neigbour_({+1, 0, -1});
    neigbours.left_back = get_neigbour_({-1, 0, +1});
    neigbours.right_back = get_neigbour_({+1, 0, +1});
  }

  // Corner neighbours
  if (corners) {
    neigbours.left_bottom_front = get_neigbour_({-1, -1, -1});
    neigbours.left_bottom_back = get_neigbour_({-1, -1, +1});
    neigbours.left_top_front = get_neigbour_({-1, +1, -1});
    neigbours.left_top_back = get_neigbour_({-1, +1, +1});
    neigbours.right_bottom_front = get_neigbour_({+1, -1, -1});
    neigbours.right_bottom_back = get_neigbour_({+1, -1, +1});
    neigbours.right_top_front = get_neigbour_({+1, +1, -1});
    neigbours.right_top_back = get_neigbour_({+1, +1, +1});
  }

  return neigbours;
}

bool World::create_new_chunk(ChunkPos chunk_pos) {
  if (chunks.contains(chunk_pos)) { return false; }
  if (chunks_awaiting_generation.contains(chunk_pos)) { return false; }
  if (chunks_being_generated.contains(chunk_pos)) { return false; }
  chunks_awaiting_generation.emplace(chunk_pos);
  return true;
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
  int16_t value = Chunk::chunk_size - 1 - local_pos.y;

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

Chunk* World::get_chunk(ChunkPos chunk_pos) const {
  const auto& chunk_it = chunks.find(chunk_pos);
  if (chunk_it == chunks.end()) {
    return nullptr;
  }
  auto& chunk = chunk_it->second;
  return chunk.get();
}

std::unordered_map<u32, CubeId> World::get_neigbours(CubePos _cube_pos, bool edges, bool corners) const {
  std::unordered_map<u32, CubeId> map;

  for (int x = -1; x <= 1; x += 1) {
    for (int y = -1; y <= 1; y += 1) {
      for (int z = -1; z <= 1; z += 1) {
        if (x == 0 && y == 0 && z == 0) { continue; }
        if (!corners && x != 0 && y != 0 && z != 0) { continue; }
        if (!edges && x != 0 && y != 0 && z == 0) { continue; }
        if (!edges && x != 0 && z != 0 && y == 0) { continue; }
        if (!edges && y != 0 && z != 0 && x == 0) { continue; }

        LocalPos cube_pos_neigbour = _cube_pos + LocalPos{x, y, z};

        u32 dir = 0;
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

std::unordered_map<u32, CubeId> World::get_neigbours(const Chunk& chunk, LocalPos _local_pos, bool edges, bool corners) const {
  std::unordered_map<u32, CubeId> map;

  auto _get_neigbour = [&](int x, int y, int z) {
    if (!corners && x != 0 && y != 0 && z != 0) { return; }
    if (!edges) {
      if (x != 0 && y != 0 && z == 0) { return; }
      if (x != 0 && z != 0 && y == 0) { return; }
      if (y != 0 && z != 0 && x == 0) { return; }
    }

    u32 dir = 0;
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

void World::update() {
  // player = nullptr;

  // Entity update
  for (auto&& entity : entities) {
    entity.get()->update();

    // Find player
    if (Player* _player = dynamic_cast<Player*>(entity.get())) {
      player = _player;
    }
  }

  // Load chunks near player
  ChunkPos player_chunk_pos = (player) ? world_pos_to_chunk_pos(player->world_pos) : ChunkPos{0, 0, 0};
  for (int x = -chunk_load_distance; x <= chunk_load_distance; x += 1) {
    for (int z = -chunk_load_distance; z <= chunk_load_distance; z += 1) {
      for (int y = -chunk_load_distance; y <= chunk_load_distance; y += 1) {
        ChunkPos chunk_pos{x, y, z};
        create_new_chunk(player_chunk_pos + chunk_pos);
      }
    }
  }

  std::multimap<i32, ChunkPos, std::greater<i32>> chunks_awaiting_generation_sorted;

  for (ChunkPos chunk_pos : chunks_awaiting_generation) {
    i32 distance_to_chunk = manhattan_distance(chunk_pos, player_chunk_pos);
    chunks_awaiting_generation_sorted.emplace(distance_to_chunk, chunk_pos);
  }
  chunks_awaiting_generation.clear();

  {
    size_t i = 0;
    for (auto& [_, chunk_pos] : chunks_awaiting_generation_sorted) {
      if (chunks_being_generated.contains(chunk_pos)) { continue; }
      size_t worker_index = i % chunk_terrain_gen_workers.size();
      chunk_terrain_gen_workers[worker_index]->add_to_queue(chunk_pos);
      chunks_being_generated.emplace(chunk_pos);
      i += 1;
    }
  }

  // Move chunks generated by workers to the world
  for (auto&& worker : chunk_terrain_gen_workers) {
    for (auto& chunk : worker->collect_finished_chunks()) {
      ChunkPos chunk_pos = chunk->position;
      ensure(!chunks_awaiting_generation.contains(chunk_pos));
      ensure(chunks_being_generated.contains(chunk_pos));
      chunks_being_generated.erase(chunk_pos);
      chunks.emplace(chunk_pos, std::move(chunk));
    }
  }

  std::vector<ChunkPos> chunks_to_be_unloaded;
  for (auto& [chunk_pos, chunk] : chunks) {
    bool chunk_is_too_far = (std::abs(player_chunk_pos.x - chunk_pos.x) > chunk_load_distance + 1) || (std::abs(player_chunk_pos.y - chunk_pos.y) > chunk_load_distance + 1) || (std::abs(player_chunk_pos.z - chunk_pos.z) > chunk_load_distance + 1);

    if (chunk_is_too_far) {
      chunks_to_be_unloaded.emplace_back(chunk_pos);
      continue;
    }

    // Set cubes, which chunk neigbours requested to set
    std::vector<std::pair<CubePos, CubeId>> chunks_neigbour_chunks_failed_cubes;
    for (auto [cube_pos, cube_id] : chunk->neigbour_chunks_cubes_to_set) {
      auto [neigb_chunk_pos, neigb_local_pos] = cube_to_local(cube_pos);
      auto neigb_chunk_it = chunks.find(neigb_chunk_pos);

      // Chunk doesn't exist
      if (neigb_chunk_it == chunks.end()) {
        chunks_neigbour_chunks_failed_cubes.push_back({cube_pos, cube_id});
        continue;
      }

      auto neigb_chunk = neigb_chunk_it->second.get();

      bool replace_leaves_with_wood = neigb_chunk->get_cube(neigb_local_pos) == CubeId::LEAVES && cube_id == CubeId::WOOD;

      // Can only replace air block
      if (!replace_leaves_with_wood && neigb_chunk->get_cube(neigb_local_pos) != CubeId::AIR) {
        continue;
      }

      neigb_chunk->set_cube(neigb_local_pos, cube_id);
    }
    chunk->neigbour_chunks_cubes_to_set = chunks_neigbour_chunks_failed_cubes;

    chunk->update();
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

bool World::aabb_is_overlapping_cube(AABB aabb, WorldPos world_pos, CubePos cube_pos) {
  static constexpr AABB full_cube_aabb = AABB{.half_extents = {0.5, 0.5, 0.5}, .offset = {0.0, 0.0, 0.0}};
  return AABB::test(
      aabb, world_pos,
      full_cube_aabb, WorldPos{cube_pos} + WorldPos{0.5, 0.5, 0.5});
}

std::vector<CubePos> World::aabb_get_solid_cubes(AABB aabb, WorldPos world_pos) {
  std::vector<CubePos> solid_cubes;
  std::vector<CubePos> overlapping_cubes = aabb_get_overlapping_cubes(aabb, world_pos);

  for (auto cube_pos : overlapping_cubes) {
    auto cube = cubes::get(get_cube(cube_pos));
    bool collider_overlapping = std::any_of(cube.collider_aabbs.begin(), cube.collider_aabbs.end(), [&](auto& cube_aabb) {
      return AABB::test(aabb, world_pos, cube_aabb, WorldPos{cube_pos} /* + WorldPos{0.5, 0.5, 0.5} */);
    });
    if (collider_overlapping) {
      solid_cubes.emplace_back(cube_pos);
    }
  }

  return solid_cubes;
}