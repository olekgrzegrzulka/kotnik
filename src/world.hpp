#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include "aabb.hpp"
#include "chunk.hpp"
#include "chunk_worker.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "entity.hpp"
#include "world_gen.hpp"

struct ChunkMeshWorker;
struct ChunkTerrainGenWorker;
class Player;

struct NeigbourCubeIds {
  std::optional<CubeId> center;
  // Straight neigbours
  std::optional<CubeId> left;
  std::optional<CubeId> right;
  std::optional<CubeId> bottom;
  std::optional<CubeId> top;
  std::optional<CubeId> front;
  std::optional<CubeId> back;

  // Edge neighbours
  std::optional<CubeId> left_bottom;
  std::optional<CubeId> right_bottom;
  std::optional<CubeId> front_bottom;
  std::optional<CubeId> back_bottom;

  std::optional<CubeId> left_top;
  std::optional<CubeId> right_top;
  std::optional<CubeId> front_top;
  std::optional<CubeId> back_top;

  std::optional<CubeId> left_front;
  std::optional<CubeId> right_front;
  std::optional<CubeId> left_back;
  std::optional<CubeId> right_back;

  // Corner neighbours
  std::optional<CubeId> left_bottom_front;
  std::optional<CubeId> left_bottom_back;
  std::optional<CubeId> left_top_front;
  std::optional<CubeId> left_top_back;
  std::optional<CubeId> right_bottom_front;
  std::optional<CubeId> right_bottom_back;
  std::optional<CubeId> right_top_front;
  std::optional<CubeId> right_top_back;
};

class World {
  friend class WorldRenderer;

public:
  static const i32 chunk_load_distance = 6;

  struct {
    double gravity = -0.012;
    double air_friction = 0.01;

    double water_gravity = -0.007;
    double water_friction = 0.17;

  } physical_properties;

private:
  std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, Vec3Hasher> chunks;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_mesh_update;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_generation;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_being_generated;
  std::vector<std::unique_ptr<Entity>> entities;

  std::vector<std::unique_ptr<ChunkTerrainGenWorker>> chunk_terrain_gen_workers;

  Player* player = nullptr;

  std::shared_ptr<WorldGen> world_gen;

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

  bool create_new_chunk(ChunkPos chunk_pos);

  CubeId get_cube(CubePos cube_pos) const;

  NeigbourCubeIds get_neigbour_ids(CubePos cube_pos, bool edges, bool corners) const;

  bool is_solid(CubePos cube_pos) const;

  bool has_solid_neigbour(CubePos world_position) const;

  bool is_solid(glm::vec<3, float> world_position_f) const;

  //
  // Raycast
  //

  WorldPos query_raycast_solid(glm::vec<3, float> from, glm::vec<3, float> to) const;

  std::optional<CubePos> raycast_get_solid_cube(glm::vec<3, float> from, glm::vec<3, float> to) const;

  // Returns world positions intersecting the raycast. They can be out of bounds.
  std::unordered_set<CubePos, Vec3Hasher> raycast_get_overlapping_cubes(WorldPos from, WorldPos to) const;

  //
  // AABB
  //

  // Get cube positions overlapping the AABB
  std::vector<CubePos> aabb_get_overlapping_cubes(AABB aabb, WorldPos world_pos);

  bool aabb_is_overlapping_cube(AABB aabb, WorldPos world_pos, CubePos cube_pos);

  // Get cube positions which AABBs overlap the AABB
  std::vector<CubePos> aabb_get_solid_cubes(AABB aabb, WorldPos world_pos);

  // Get sunlight of a cube by checking how much space there is above it
  // FIXME: this isn't really accurate as we should be using inverse of heightmap (lowmap?) to get the CLOSEST cube to our cube
  uint8_t get_sunlight(CubePos cube_pos) const;

  // The y position is used to determine the chunk
  std::optional<uint16_t> get_heightmap(CubePos cube_pos) const;

  Chunk* get_chunk(ChunkPos chunk_pos) const;

  void update_light(CubePos cube_pos);

  void spread_light(const CubePos light_cube_pos, std::unordered_set<CubePos, Vec3Hasher>& visited_cubes);

  void request_player_chunk_light_update();

  void request_chunk_light_update(ChunkPos chunkchunk_pos);

  std::unordered_map<u32, CubeId> get_neigbours(CubePos _cube_pos, bool edges = false, bool corners = false) const;

  std::unordered_map<u32, CubeId> get_neigbours(const Chunk& chunk, LocalPos _local_pos, bool edges = false, bool corners = false) const;

  void update();
};