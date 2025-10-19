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

  std::vector<std::unique_ptr<Chunk>> generate_chunk_column(glm::vec<2, i32>) const;
  void generate_chunk_only_water(Chunk* chunk) const;

  float get_humidity(WorldPos world_pos) const;
  float get_temperature(WorldPos world_pos) const;
  biomes::Biome get_blended_biome(float humidity, float temperature) const;
  biomes::Biome get_blended_biome(WorldPos world_pos) const;

  float get_heightmap_noise(WorldPos pos, const biomes::Biome& blended_biome) const;
  float get_3d_noise(WorldPos pos, const biomes::Biome& blended_biome) const;

  bool is_ground(WorldPos pos, const biomes::Biome& blended_biome, float noise_heightmap_value, float noise_3d_value) const;
  bool is_ground(WorldPos pos, const biomes::Biome& blended_biome) const;
  bool is_ground(WorldPos pos) const;

private:
  World& world;

  FastNoiseLite noise_heightmap;
  FastNoiseLite noise_3d;
  FastNoiseLite noise_humidity;
  FastNoiseLite noise_temperature;
};