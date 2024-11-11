#pragma once

#include <functional>
#include <string>
#include "common.hpp"

enum class CubeId : u16;

namespace biomes {

enum class BiomeId : u16 {
  FLATLANDS,
  DESERT,
  OCEAN,
  DEEP_OCEAN,
  BEACH,
  HIGHLANDS,
  HILLYLANDS,
  // "Rocky Shore"
  // "Forest"
  // "Deep Forest"
  // "Swamplands"
  // "Deep Ocean"
  BIOME_ID_SIZE,
};

struct Biome final {
  std::string name = "Biome";

  float base_height = 0;
  float noise_height_multiplier = 0;
  float noise_spiky_multiplier = 0;
  float noise_3d_multiplier = 0;

  std::function<CubeId(i32 y, i32 depth, float rng)> get_ground_cube;
  std::function<CubeId(i32 y, float rng)> get_foliage_cube;
  std::function<CubeId(i32 y, float rng)> get_air_cube;
};

const Biome& get_biome(BiomeId id);

Biome biome_lerp(const biomes::Biome& biome_a, const biomes::Biome& biome_b, float t);

Biome biome_lerp(BiomeId biome_a, BiomeId biome_b, float t);

Biome biome_weighted_average(std::span<float> weights, std::span<BiomeId> biome_ids);

Biome biome_weighted_average(std::span<float> weights, std::span<Biome> biomes);

}; // namespace biomes