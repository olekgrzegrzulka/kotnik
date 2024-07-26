#pragma once

#include "common.hpp"
#include "fast_noise_lite.h"

namespace biomes {
struct Biome;
}

class BiomeMap;
class Chunk;
class World;

class WorldGen {
public:
  WorldGen(World& w, i32 seed);

  void generate_chunk(Chunk* chunk) const;
  biomes::Biome get_blended_biome(WorldPos world_pos) const;
  bool is_ground(WorldPos pos, const biomes::Biome& blended_biome) const;
  bool is_ground(WorldPos pos) const;
  float get_cube_rng(WorldPos pos) const;

private:
  World& world;

  FastNoiseLite noise_heightmap;
  FastNoiseLite noise_3d;
  FastNoiseLite noise_humidity;
  FastNoiseLite noise_temperature;
  FastNoiseLite noise_rng;
};