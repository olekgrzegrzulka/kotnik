#pragma once
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include "chunk.hpp"
#include "common.hpp"
#include "entity.hpp"

struct ChunkMeshWorker;
struct ChunkTerrainGenWorker;
class Player;

class World {
public:
  struct {
    float gravity = -0.017f;
    float air_friction = 0.01f;

  } physical_properties;

private:
  std::unordered_map<ChunkPos, Chunk, Vec3Hasher> chunks;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_mesh_update;
  std::vector<ChunkPos> chunks_to_keep_loaded;
  std::vector<std::unique_ptr<Entity>> entities;

  std::vector<ChunkMeshWorker*> chunk_mesh_workers;
  std::vector<ChunkTerrainGenWorker*> chunk_terrain_gen_workers;

  Player* player = nullptr;

public:
  World();

  ~World();

  const Player* get_player() const;

  template <class T>
  void add_entity(WorldPos world_pos) {
    static_assert(std::is_base_of<Entity, T>());

    entities.emplace_back(std::make_unique<T>(*this));

    entities.back().get()->world_pos = world_pos;
  }

  void set_cube(CubePos cube_pos, CubeId to);

  void create_new_chunk(ChunkPos chunk_pos);

  CubeId get_cube(CubePos cube_pos) const;

  bool is_solid(CubePos cube_pos) const;

  bool has_solid_neigbour(CubePos world_position) const;

  bool is_solid(glm::vec<3, float> world_position_f) const;

  WorldPos query_raycast_solid(glm::vec<3, float> from, glm::vec<3, float> to) const;

  // Get sunlight of a cube by checking how much space there is above it
  // FIXME: this isn't really accurate as we should be using inverse of heightmap (lowmap?) to get the CLOSEST cube to our cube
  uint8_t get_sunlight(CubePos cube_pos) const;

  // The y position is used to determine the chunk
  std::optional<uint16_t> get_heightmap(CubePos cube_pos) const;

  LightLevel get_lightmap(CubePos cube_pos) const;

  std::optional<CubePos> raycast_get_solid_cube(glm::vec<3, float> from, glm::vec<3, float> to) const;

  // Returns world positions intersecting the raycast. They can be out of bounds.
  std::unordered_set<CubePos, Vec3Hasher> raycast_get_overlapping_cubes(WorldPos from, WorldPos to) const;

  const Chunk* get_chunk(ChunkPos chunk_pos) const;

  void update_light(CubePos cube_pos);

  void spread_light(const CubePos light_cube_pos, std::unordered_set<CubePos, Vec3Hasher>& visited_cubes);

  void generate_lightmap_all();

  std::unordered_map<uint32_t, CubeId> get_neigbours(CubePos _cube_pos, bool edges = false, bool corners = false) const;

  std::unordered_map<uint32_t, CubeId> get_neigbours(const Chunk& chunk, LocalPos _local_pos, bool edges = false, bool corners = false) const;

  bool is_chunk_ready(ChunkPos chunk_pos);

  void update();

  void draw(WorldPos camera_pos, const glm::mat4& camera_matrix);
};