#include "biome.hpp"
#include <vector>
#include "cubes.hpp"

using BiomeList = std::array<biomes::Biome, (size_t)biomes::BiomeId::BIOME_ID_SIZE>;

const BiomeList init_biome_list() {
  using namespace biomes;

  BiomeList biome_list;

  // Flatlands

  Biome biome_flatlands{
      .name = "Flatlands",
      .base_height = 0.0f,
      .noise_height_multiplier = 10.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_flatlands.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth == 0) { return CubeId::GRASS; }
    if (depth <= 3) { return CubeId::DIRT; }
    return CubeId::STONE;
  };

  biome_flatlands.get_foliage_cube = [](i32, float rng) -> CubeId {
    if (rng > 0.98f) { return CubeId::FLOWER; }
    if (rng > 0.87f) { return CubeId::GRASS_PLANT; }
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::FLATLANDS] = biome_flatlands;

  // Desert

  Biome biome_desert{
      .name = "Desert",
      .base_height = 0.0f,
      .noise_height_multiplier = 12.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_desert.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth <= 4) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_desert.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::DESERT] = biome_desert;

  // Ocean

  Biome biome_ocean{
      .name = "Ocean",
      .base_height = -8.0f,
      .noise_height_multiplier = 8.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_ocean.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth <= 3) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_ocean.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::OCEAN] = biome_ocean;

  // Deep Ocean

  Biome biome_deep_ocean{
      .name = "Deep Ocean",
      .base_height = -32.0f,
      .noise_height_multiplier = 8.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_deep_ocean.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth <= 3) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_deep_ocean.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::DEEP_OCEAN] = biome_deep_ocean;

  // Beach

  Biome biome_beach{
      .name = "Beach",
      .base_height = 0.0f,
      .noise_height_multiplier = 4.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_beach.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth <= 3) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_beach.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::BEACH] = biome_beach;

  // Highlands

  Biome biome_highlands{
      .name = "Highlands",
      .base_height = 20.0f,
      .noise_height_multiplier = 60.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 40.0f,
  };

  biome_highlands.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth == 0) { return CubeId::GRASS; }
    if (depth <= 3) { return CubeId::DIRT; }
    return CubeId::STONE;
  };

  biome_highlands.get_foliage_cube = [](i32, float rng) -> CubeId {
    if (rng > 0.994f) { return CubeId::FLOWER; }
    if (rng > 0.87f) { return CubeId::GRASS_PLANT; }
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::HIGHLANDS] = biome_highlands;

  // Hillylands

  Biome biome_hillylands{
      .name = "Hillylands",
      .base_height = 0.0f,
      .noise_height_multiplier = 20.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 15.0f,
  };

  biome_hillylands.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth == 0) { return CubeId::GRASS; }
    if (depth <= 3) { return CubeId::DIRT; }
    return CubeId::STONE;
  };

  biome_hillylands.get_foliage_cube = [](i32, float rng) -> CubeId {
    if (rng > 0.988f) { return CubeId::FLOWER; }
    if (rng > 0.87f) { return CubeId::GRASS_PLANT; }
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::HILLYLANDS] = biome_hillylands;

  debug_log("Initialized biome list");
  return biome_list;
}

namespace biomes {
const Biome& get_biome(BiomeId id) {
  static const BiomeList biome_list = init_biome_list();
  return biome_list[(size_t)id];
}

Biome biome_lerp(const biomes::Biome& biome_a, const biomes::Biome& biome_b, float t) {
  if (t < 0.0f || t > 1.0f) {
    debug_error("biomes::blend_two_biomes(): t value out of range");
  }

  Biome blended_biome{
      .base_height = biome_a.base_height * (1.0f - t) + biome_b.base_height * t,
      .noise_height_multiplier = biome_a.noise_height_multiplier * (1.0f - t) + biome_b.noise_height_multiplier * t,
      .noise_spiky_multiplier = biome_a.noise_spiky_multiplier * (1.0f - t) + biome_b.noise_spiky_multiplier * t,
      .noise_3d_multiplier = biome_a.noise_3d_multiplier * (1.0f - t) + biome_b.noise_3d_multiplier * t,
  };

  const Biome& stronger_biome = (t <= 0.5f) ? biome_a : biome_b;
  blended_biome.name = stronger_biome.name;
  blended_biome.get_ground_cube = stronger_biome.get_ground_cube;
  blended_biome.get_foliage_cube = stronger_biome.get_foliage_cube;

  return blended_biome;
}

Biome biome_lerp(BiomeId biome_a, BiomeId biome_b, float t) {
  return biome_lerp(biomes::get_biome(biome_a), biomes::get_biome(biome_b), t);
}

Biome biome_weighted_average(std::span<float> weights, std::span<BiomeId> biome_ids) {

  std::vector<Biome> biomes;
  for (auto biome_id : biome_ids) {
    biomes.emplace_back(get_biome(biome_id));
  }

  return biome_weighted_average(weights, biomes);
}

Biome biome_weighted_average(std::span<float> weights, std::span<Biome> biomes) {
  if (weights.size() != biomes.size()) {
    debug_error("biome_weighted_average(): argument array sizes mismatch");
    return Biome{};
  }

  if (biomes.empty()) {
    debug_error("biome_weighted_average(): argument arrays are empty");
    return Biome{};
  }

  Biome blended_biome{};
  float total_weight = 0.0f;
  size_t strongest_biome_index = 0;

  for (size_t i = 0; i < biomes.size(); i += 1) {
    const Biome& b = biomes[i];
    float w = weights[i];

    total_weight += w;

    blended_biome.base_height += b.base_height * w;
    blended_biome.noise_height_multiplier += b.noise_height_multiplier * w;
    blended_biome.noise_spiky_multiplier += b.noise_spiky_multiplier * w;
    blended_biome.noise_3d_multiplier += b.noise_3d_multiplier * w;

    if (weights[i] > weights[strongest_biome_index]) {
      strongest_biome_index = i;
    }
  }

  blended_biome.base_height /= total_weight;
  blended_biome.noise_height_multiplier /= total_weight;
  blended_biome.noise_spiky_multiplier /= total_weight;
  blended_biome.noise_3d_multiplier /= total_weight;

  const Biome& strongest_biome = biomes[strongest_biome_index];
  blended_biome.name = strongest_biome.name;
  blended_biome.get_ground_cube = strongest_biome.get_ground_cube;
  blended_biome.get_foliage_cube = strongest_biome.get_foliage_cube;

  return blended_biome;
}
} // namespace biomes
