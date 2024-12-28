#pragma once
#include <memory>
#include <unordered_set>
#include <vector>
#include <glm/mat4x4.hpp>
#include "common.hpp"

struct ChunkMeshWorker;
class Shader;
class Texture;
class World;

class WorldRenderer final {
public:
  static constexpr bool ambient_occlusion_enabled = true;
  static constexpr u8 ambient_occlusion_intensity = 105;

private:
  World& world;
  Shader& cube_shader;
  Texture& atlas_texture;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_mesh_update;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_being_meshed;
  std::vector<std::unique_ptr<ChunkMeshWorker>> chunk_mesh_workers;

  void add_chunk_for_mesh_update(ChunkPos);

public:
  WorldRenderer(World&, Shader& cube_shader_, Texture& atlas_texture_);

  ~WorldRenderer();

  void update(WorldPos camera_pos, const glm::mat4& camera_matrix);
};