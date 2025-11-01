#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"

class World;

// Copy of 3x3x3 chunk area for mesh building
struct ChunkMeshData {
public:
  ChunkMeshData(ChunkPos, World&);

  CubeId get_cube_id(LocalPos at, bool check_if_neigbour_chunk = true);
  u8 get_lightmap(LocalPos at, bool check_if_neigbour_chunk = true);

  bool is_valid() const { return valid; }
  ChunkPos get_chunk_pos() const { return chunk_pos; }

  rgb foliage_color_neg_x_neg_z{};
  rgb foliage_color_neg_x_pos_z{};
  rgb foliage_color_pos_x_neg_z{};
  rgb foliage_color_pos_x_pos_z{};

private:
  std::array<decltype(Chunk::cubes), 27> cubes;
  std::array<decltype(Chunk::lightmap), 27> lightmaps;
  bool valid = false;
  ChunkPos chunk_pos;

  static constexpr size_t neigbour_chunk_offset_to_data_index(ChunkPos offset) {
    ensure(std::abs(offset.x) <= 1 && std::abs(offset.y) <= 1 && std::abs(offset.z) <= 1);
    return (offset.x + 1) + (offset.y + 1) * 3 + (offset.z + 1) * 9;
  }
};

class ChunkMesh {
public:
  std::vector<CompactVertex> vertices;
  u32 vbo = 0;
  u32 vao = 0;

  std::vector<CompactVertex> vertices_translucent;
  u32 vbo_translucent = 0;
  u32 vao_translucent = 0;

  bool ready = false;

public:
  ChunkMesh();
  ChunkMesh(std::unique_ptr<ChunkMeshData>);
  ~ChunkMesh();
  void draw();
  void draw_translucent();

private:
  void initialize();
};