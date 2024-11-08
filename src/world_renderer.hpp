#pragma once
#include <unordered_set>
#include <vector>
#include "common.hpp"

class World;
struct ChunkMeshWorker;

class WorldRenderer final {
public:
  static constexpr bool ambient_occlusion_enabled = true;

private:
  World& world;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_mesh_update;
  std::vector<ChunkMeshWorker*> chunk_mesh_workers;

public:
  WorldRenderer(World& _world);

  ~WorldRenderer();

  void update(WorldPos camera_pos, const glm::mat4& camera_matrix);
};