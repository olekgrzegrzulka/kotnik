#pragma once
#include <array>
#include <atomic>
#include <vector>
#include "chunk.hpp"
#include "cubes.hpp"

class World;

// Copy of 3x3 chunk area for mesh building
struct ChunkMeshData {
public:
  ChunkMeshData(ChunkPos, World&);

  CubeId get_cube_id(LocalPos at);

private:
  std::array<std::array<CubeId, CHUNK_CUBES>, 27> data;

  static constexpr size_t neigbour_chunk_offset_to_data_index(ChunkPos offset) {
    ensure(std::abs(offset.x) <= 1 && std::abs(offset.y) <= 1 && std::abs(offset.z) <= 1);
    return (offset.x + 1) + (offset.y + 1) * 3 + (offset.z + 1) * 9;
  }
};

class Chunk;

class ChunkRenderer {
public:
  Chunk& chunk;

  int drawing_index = 0;
  int building_index = 1;

  std::array<std::vector<cubes::CompactVertex>, 2> vertices{};
  std::array<std::vector<cubes::CompactVertex>, 2> vertices_translucent{};

  std::array<u32, 2> vbos{};
  std::array<u32, 2> vaos{};

  std::array<u32, 2> vbos_translucent{};
  std::array<u32, 2> vaos_translucent{};

  std::atomic<bool> can_swap_buffers;
  std::atomic<bool> is_running;

public:
  void rebuild_mesh(ChunkMeshData);
  void swap_buffers();
  void draw() const;
  void draw_translucent() const;
  ChunkRenderer(Chunk& _chunk);
};