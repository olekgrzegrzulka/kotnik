#pragma once
#include <array>
#include <atomic>
#include <vector>
#include <stdint.h>
#include "cubes.hpp" // for Vertex

class Chunk;

class ChunkRenderer {
public:
  Chunk& chunk;

  int drawing_index = 0;
  int building_index = 1;
  std::array<std::vector<Vertex>, 2> vertices{};
  std::array<uint32_t, 2> vbos{};
  std::array<uint32_t, 2> vaos{};

  std::atomic<bool> can_swap_buffers;
  std::atomic<bool> is_running;

  void recreate_geometry(std::vector<Chunk*> chunk_list);
  void swap_buffers();
  void draw(WorldPos camera_pos, const glm::mat4& camera_matrix) const;

public:
  ChunkRenderer(Chunk& _chunk);
};