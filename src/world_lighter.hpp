#pragma once
#include <memory>
#include "chunk.hpp"

class World;

class WorldLighter {
public:
  WorldLighter(World& world_) : world{world_} {
  }

  World& world;

  std::vector<std::unique_ptr<Chunk>> chunks_to_be_lit;
  std::vector<std::unique_ptr<Chunk>> chunks_finished;

  void update();

  void spread_light(i32 chunk_x, i32 chunk_z);
};