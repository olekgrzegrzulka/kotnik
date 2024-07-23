#pragma once
#include <array>
#include <atomic>
#include <vector>
#include "cubes.hpp"

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
  void rebuild_mesh(std::vector<Chunk*> chunk_list);
  void swap_buffers();
  void draw() const;
  void draw_translucent() const;
  ChunkRenderer(Chunk& _chunk);
};