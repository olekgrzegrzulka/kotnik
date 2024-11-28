#pragma once

#include <optional>
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
  void generate_chunk_only_water(Chunk* chunk) const;
  biomes::Biome get_blended_biome(WorldPos world_pos) const;

  float get_heightmap_noise(WorldPos pos, const biomes::Biome& blended_biome) const;
  float get_3d_noise(WorldPos pos, const biomes::Biome& blended_biome) const;

  bool is_ground(WorldPos pos, const biomes::Biome& blended_biome,
                 std::optional<float> noise_heightmap_value = std::nullopt, std::optional<float> noise_3d_value = std::nullopt) const;

  bool is_ground(WorldPos pos, const biomes::Biome& blended_biome) const;
  bool is_ground(WorldPos pos) const;

private:
  World& world;

  FastNoiseLite noise_heightmap;
  FastNoiseLite noise_3d;
  FastNoiseLite noise_humidity;
  FastNoiseLite noise_temperature;
};