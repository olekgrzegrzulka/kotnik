#pragma once
#include <functional>
#include <string>
#include "common.hpp"
#include "cubes.hpp"

enum class CubeId : u16;

namespace biomes {

enum class BiomeId : u16 {
  FLATLANDS,
  FOREST,
  DESERT,
  DESERT_HIGHLANDS,
  OCEAN,
  OCEAN_DESERT,
  DEEP_OCEAN,
  BEACH,
  HIGHLANDS,
  HILLYLANDS,
  SHALLOW_WATERS,
  STONY_SHORES,
  PLATEAU,
  // "Rocky Shore"
  // "Forest"
  // "Deep Forest"
  // "Swamplands"
  // "Deep Ocean"
  BIOME_ID_SIZE,
};

struct Biome final {
  std::string name = "Biome";

  float base_height = 0.0f;
  float noise_height_multiplier = 0.0f;
  float noise_spiky_multiplier = 0.0f;
  float noise_3d_multiplier = 0.0f;
  float strength = 1.0f;

  float tree_density = 0.0f; // from 0 to 1
  float oak_tree_chance = 0.0f;
  float birch_tree_chance = 0.0f;
  float spruce_tree_chance = 0.0f;

  std::function<CubeId(i32 y, i32 depth, float rng)> get_ground_cube = [](i32, i32, float) -> CubeId {
    return CubeId::DIRT;
  };
  std::function<CubeId(i32 y, float rng)> get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  Biome operator/(float div) {
    return Biome{
        .name = name,
        .base_height = base_height / div,
        .noise_height_multiplier = noise_height_multiplier / div,
        .noise_spiky_multiplier = noise_spiky_multiplier / div,
        .noise_3d_multiplier = noise_3d_multiplier / div,
        .strength = strength / div,
    };
  }

  Biome operator*(float mul) {
    return Biome{
        .name = name,
        .base_height = base_height * mul,
        .noise_height_multiplier = noise_height_multiplier * mul,
        .noise_spiky_multiplier = noise_spiky_multiplier * mul,
        .noise_3d_multiplier = noise_3d_multiplier * mul,
        .strength = strength * mul,
    };
  }
};

const Biome& get_biome(BiomeId id);

Biome biome_lerp(const biomes::Biome& biome_a, const biomes::Biome& biome_b, float t);

Biome biome_lerp(BiomeId biome_a, BiomeId biome_b, float t);

Biome biome_weighted_average(std::span<float> weights, std::span<BiomeId> biome_ids);

Biome biome_weighted_average(std::span<float> weights, std::span<Biome> biomes);

}; // namespace biomes