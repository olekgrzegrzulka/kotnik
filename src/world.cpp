#include "world.hpp"
#include <unordered_set>
#include "chunk_renderer2.hpp"
#include "chunk_worker.hpp"
#include "entity.hpp"
#include "player.hpp"

World::World() {
  for (size_t i = 0; i < 4; i += 1) {
    chunk_mesh_workers.push_back(new ChunkMeshWorker);
  }

  for (size_t i = 0; i < 4; i += 1) {
    chunk_terrain_gen_workers.push_back(new ChunkTerrainGenWorker);
  }
}

World::~World() {
  for (size_t i = 0; i < chunk_mesh_workers.size(); i += 1) {
    delete chunk_mesh_workers[i];
  }

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
  chunks.emplace(chunk_pos, Chunk{*this, chunk_pos});
  chunks.at(chunk_pos).renderer = new ChunkRenderer(chunks.at(chunk_pos));
  Chunk& c = chunks.at(chunk_pos);
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
  if (chunks.at(chunk_pos).get_heightmap(local_pos.x, local_pos.z).value_or(0) > local_pos.y) { return chunks.at(chunk_pos).get_heightmap(local_pos.x, local_pos.z).value() - local_pos.y - 1; }
  int16_t value = CHUNK_SIZE - 1 - local_pos.y;

  for (size_t i = 1; i < 10; i += 1) {
    // The chunk is empty, can't block light
    if (!chunks.contains(chunk_pos + ChunkPos{0, i, 0})) {
      value += 16;
      continue;
    }
    // The chunk's column is empty, can't block light
    if (!chunks.at(chunk_pos + ChunkPos{0, i, 0}).get_heightmap(local_pos.x, local_pos.z).has_value()) {
      value += 16;
      continue;
    }
    // Chunk is blocking light, add distance from bottom of the chunk to the blocking cube and return
    value += chunks.at(chunk_pos + ChunkPos{0, i, 0}).get_heightmap(local_pos.x, local_pos.z).value();
    return std::clamp(value - 16, 0, 255);
  }

  return 255;
}

// The y position is used to determine the chunk
std::optional<uint16_t> World::get_heightmap(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);

  if (!chunks.contains(chunk_pos)) { return 0; }
  return chunks.at(chunk_pos).get_heightmap(local_pos.x, local_pos.z);
}

LightLevel World::get_lightmap(CubePos cube_pos) const {
  auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
  assert(is_local_pos_valid(local_pos));

  if (!chunks.contains(chunk_pos)) { return LightLevel{}; }
  return chunks.at(chunk_pos).get_lightmap(local_pos);
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
  if (!chunk.flags.ready) {
    return nullptr;
  }

  return &chunk;
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

void World::generate_lightmap_all() {
  std::unordered_set<CubePos, Vec3Hasher> visited_cubes;

  for (auto& [chunk_pos, chunk] : chunks) {
    {
      chunk.clear_lightmap();
      for (int _x = 0; _x < CHUNK_SIZE; _x += 1) {
        for (int _z = 0; _z < CHUNK_SIZE; _z += 1) {
          auto _y = chunk.get_heightmap(_x, _z);
          if (!_y.has_value()) { continue; }

          // All air cubes over _y are sunlit
          for (int __y = _y.value(); __y < CHUNK_SIZE; __y += 1) {
            chunk.set_lightmap({_x, __y, _z}, {255, 0, 0});
            auto local_pos = LocalPos{_x, __y, _z};
            auto cube_pos = local_pos_to_cube_pos(chunk_pos, local_pos);
            spread_light(cube_pos, visited_cubes);
          }
        }
      }
    }
  }
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
  player = nullptr;

  // Entity update
  for (auto&& entity : entities) {
    entity.get()->update();

    // Find player
    if (Player* _player = dynamic_cast<Player*>(entity.get())) {
      player = _player;
    }
  }

  // Unlock chunks locked by mesh worker threads
  for (const auto& worker : chunk_mesh_workers) {
    if (worker->try_collecting()) {
    }
  }

  // Unlock chunks locked by terrain gen worker threads
  for (const auto& worker : chunk_terrain_gen_workers) {
    if (worker->try_collecting()) {
    }
  }

  // Load chunks near player
  ChunkPos chunk_load_center = world_pos_to_chunk_pos(player->world_pos);
  chunks_to_keep_loaded.clear();
  for (int x = -9; x <= 9; x += 1) {
    for (int z = -9; z <= 9; z += 1) {
      for (int y = -4; y <= 4; y += 1) {
        chunks_to_keep_loaded.emplace_back(chunk_load_center + ChunkPos{x, y, z});
      }
    }
  }

  sort_vector_by_distance(chunks_to_keep_loaded, world_pos_to_chunk_pos(player->world_pos));

  int i = 0;
  for (const ChunkPos chunk_pos : chunks_to_keep_loaded) {
    if (chunks.contains(chunk_pos)) { continue; }
    if (i++ >= 2) { break; }
    create_new_chunk(chunk_pos);
  }

  // Unload chunks far from player
  std::vector<ChunkPos> chunks_to_remove;
  for (const auto& [chunk_pos, chunk] : chunks) {
    if (std::find(chunks_to_keep_loaded.begin(), chunks_to_keep_loaded.end(), chunk_pos) == chunks_to_keep_loaded.end()) {
      if (!chunk.flags.locked && chunk.flags.ready) {
        chunks_to_remove.emplace_back(chunk_pos);
      }
    }
  }
  for (const auto chunk_pos : chunks_to_remove) {
    chunks.erase(chunk_pos);
  }

  // Get list of chunks that were modified, so we can update their geometry
  std::vector<ChunkPos> chunks_not_ready;

  for (auto& [chunk_pos, chunk] : chunks) {
    if (!chunk.flags.ready) {
      chunks_not_ready.emplace_back(chunk_pos);
      continue;
    }

    chunk.update();

    if (chunk.renderer->can_swap_buffers) {
      chunk.renderer->swap_buffers();
    }

    if (chunk.flags.update_geometry) {
      // FIXME: update chunks that were actually modified
      chunks_awaiting_mesh_update.emplace(chunk_pos);
      chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(-1, 0, 0));
      chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(1, 0, 0));
      chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, -1, 0));
      chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 1, 0));
      chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 0, -1));
      chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 0, 1));

      chunk.flags.update_geometry = false;
    }
  }

  sort_vector_by_distance(chunks_not_ready, world_pos_to_chunk_pos(player->world_pos));

  for (const ChunkPos chunk_pos : chunks_not_ready) {
    for (const auto& worker : chunk_terrain_gen_workers) {
      if (worker->run_job(&(chunks.at(chunk_pos)))) {
        break;
      }
    }
  }

  // Threaded mesh update
  std::unordered_set<ChunkPos, Vec3Hasher> new_chunks_awaiting_mesh_update;

  std::vector<ChunkPos> chunks_awaiting_mesh_update_sorted_by_distance;
  for (auto x : chunks_awaiting_mesh_update) {
    chunks_awaiting_mesh_update_sorted_by_distance.emplace_back(x);
  }
  sort_vector_by_distance(chunks_awaiting_mesh_update_sorted_by_distance, world_pos_to_chunk_pos(player->world_pos));

  for (const ChunkPos chunk_pos : chunks_awaiting_mesh_update_sorted_by_distance) {
    const Chunk* chunk = get_chunk(chunk_pos);
    if (chunk == nullptr) { continue; }
    if (!chunk->flags.ready) { continue; }

    bool updated = false;
    for (const auto& worker : chunk_mesh_workers) {
      if (worker->run_job(const_cast<Chunk*>(chunk))) {
        updated = true;
        break;
      }
    }

    if (!updated) {
      new_chunks_awaiting_mesh_update.emplace(chunk->position);
    }
  }

  chunks_awaiting_mesh_update = new_chunks_awaiting_mesh_update;
}

void World::draw(const glm::mat4& matrix) {
  for (auto& it : chunks) {
    if (!it.second.flags.ready) { continue; }
    it.second.renderer->draw(matrix);
  }
}
