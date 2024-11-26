#pragma once
#include <unordered_set>
#include <vector>
#include "common.hpp"

class World;
struct ChunkMeshWorker;

class WorldRenderer final {
public:
  static constexpr bool ambient_occlusion_enabled = true;
  static constexpr u8 ambient_occlusion_intensity = 105;

private:
  World& world;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_mesh_update;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_being_meshed;
  std::vector<ChunkMeshWorker*> chunk_mesh_workers;

  void add_chunk_for_mesh_update(ChunkPos);

public:
  WorldRenderer(World& _world);

  ~WorldRenderer();

  void update(WorldPos camera_pos, const glm::mat4& camera_matrix);
};