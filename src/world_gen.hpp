#pragma once

#include "biome.hpp"
#include "biome_map.hpp"
#include "common.hpp"
#include "fast_noise_lite.h"

class BiomeMap;
class Chunk;
class World;

class WorldGen {
public:
  WorldGen(World& w, i32 seed);

  void generate_chunk(Chunk* chunk) const;

  Biomes::BlendedBiome get_blended_biome(WorldPos world_pos) const;
  bool is_ground(WorldPos pos, const Biomes::BlendedBiome& blended_biome) const;
  bool is_ground(WorldPos pos) const;
  float get_cube_rng(WorldPos pos) const;

private:
  World& world;
  BiomeMap biome_map{};

  FastNoiseLite noise_heightmap;
  FastNoiseLite noise_3d;
  FastNoiseLite noise_humidity;
  FastNoiseLite noise_temperature;
  FastNoiseLite noise_rng;
};