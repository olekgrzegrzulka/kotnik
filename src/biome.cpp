#include "biome.hpp"
#include <algorithm>
#include <vector>
#include "common.hpp"
#include "cubes.hpp"

using BiomeList = std::array<biomes::Biome, (size_t)biomes::BiomeId::BIOME_ID_SIZE>;

const BiomeList init_biome_list() {
  using namespace biomes;

  BiomeList biome_list;

  // Flatlands
  Biome biome_flatlands{
      .name = "Flatlands",
      .foliage_color = rgb{143, 149, 50},
      .base_height = 0.0f,
      .noise_height_multiplier = 10.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
      .tree_density = 0.001f,
      .oak_tree_chance = 0.2f,
      .birch_tree_chance = 0.05f,
      .spruce_tree_chance = 0.0f,
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

  // Flatlands
  Biome biome_forest{
      .name = "Forest",
      .foliage_color = rgb{90, 132, 41},
      .base_height = 0.0f,
      .noise_height_multiplier = 10.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
      .tree_density = 0.8f,
      .oak_tree_chance = 1.0f,
      .birch_tree_chance = 0.2f,
      .spruce_tree_chance = 0.02f,
  };

  biome_forest.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth == 0) { return CubeId::GRASS; }
    if (depth <= 3) { return CubeId::DIRT; }
    return CubeId::STONE;
  };

  biome_forest.get_foliage_cube = [](i32, float rng) -> CubeId {
    if (rng > 0.98f) { return CubeId::FLOWER; }
    if (rng > 0.87f) { return CubeId::GRASS_PLANT; }
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::FOREST] = biome_forest;

  // Desert
  Biome biome_desert{
      .name = "Desert",
      .foliage_color = rgb{163, 124, 33},
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

  // Desert Highlands
  Biome biome_desert_highlands{
      .name = "Desert Highlands",
      .foliage_color = rgb{163, 124, 33},
      .base_height = 50.0f,
      .noise_height_multiplier = 10.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 10.0f,
  };

  biome_desert_highlands.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth <= 4) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_desert_highlands.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::DESERT_HIGHLANDS] = biome_desert_highlands;

  // Shallow Waters
  Biome biome_shallow_waters{
      .name = "Shallow Waters",
      .foliage_color = rgb{133, 139, 83},
      .base_height = -2.0f,
      .noise_height_multiplier = 2.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
      .tree_density = 0.4f,
      .oak_tree_chance = 3.0f,
      .birch_tree_chance = 0.5f,
      .spruce_tree_chance = 0.1f,
  };

  biome_shallow_waters.get_ground_cube = [](i32, i32 depth, float) -> CubeId {
    if (depth <= 3) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_shallow_waters.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::SHALLOW_WATERS] = biome_shallow_waters;

  // Ocean
  Biome biome_ocean{
      .name = "Ocean",
      .foliage_color = rgb{133, 139, 80},
      .base_height = -8.0f,
      .noise_height_multiplier = 8.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_ocean.get_ground_cube = [](i32 y, i32 depth, float) -> CubeId {
    if (y > 2) {
      if (depth == 0) { return CubeId::GRASS; }
      if (depth <= 3) { return CubeId::DIRT; }
      return CubeId::STONE;
    }
    if (depth <= 3) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_ocean.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::OCEAN] = biome_ocean;

  // Ocean (but for Desert biome, replacing grass on higher altitudes with sand)
  Biome biome_ocean_desert = Biome{biome_ocean};

  biome_ocean_desert.get_ground_cube = [](i32 y, i32 depth, float) -> CubeId {
    if (y > 2) {
      if (depth <= 4) { return CubeId::SAND; }
      return CubeId::STONE;
    }
    if (depth <= 3) { return CubeId::SAND; }
    return CubeId::STONE;
  };

  biome_list[(size_t)BiomeId::OCEAN_DESERT] = biome_ocean_desert;

  // Deep Ocean
  Biome biome_deep_ocean{
      .name = "Deep Ocean",
      .foliage_color = rgb{133, 139, 80},
      .base_height = -32.0f,
      .noise_height_multiplier = 8.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
  };

  biome_deep_ocean.get_ground_cube = [](i32 y, i32 depth, float) -> CubeId {
    if (y > 2) {
      if (depth == 0) { return CubeId::GRASS; }
      if (depth <= 3) { return CubeId::DIRT; }
      return CubeId::STONE;
    }
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
      .foliage_color = rgb{133, 139, 83},
      .base_height = 0.0f,
      .noise_height_multiplier = 4.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
      .tree_density = 0.0f,
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
      .foliage_color = rgb{90, 142, 75},
      .base_height = 65.0f,
      .noise_height_multiplier = 15.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 35.0f,
      .tree_density = 0.7f,
      .oak_tree_chance = 6.0f,
      .birch_tree_chance = 1.0f,
      .spruce_tree_chance = 1.5f,
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

  // Plateau
  Biome biome_plateau{
      .name = "Plateau",
      .base_height = 70.0f,
      .noise_height_multiplier = 4.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
      .strength = 10000.0f,
  };

  biome_plateau.get_ground_cube = [](i32, i32 depth, float y) -> CubeId {
    if (y < 67) { return CubeId::STONE; }

    if (depth == 0) { return CubeId::GRASS; }
    if (depth <= 3) { return CubeId::DIRT; }
    return CubeId::STONE;
  };

  biome_plateau.get_foliage_cube = [](i32, float rng) -> CubeId {
    if (rng > 0.997f) { return CubeId::FLOWER; }
    if (rng > 0.91f) { return CubeId::GRASS_PLANT; }
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::PLATEAU] = biome_plateau;

  // Hillylands
  Biome biome_hillylands{
      .name = "Hillylands",
      .foliage_color = rgb{125, 145, 50},
      .base_height = 10.0f,
      .noise_height_multiplier = 20.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 8.0f,
      .tree_density = 0.55f,
      .oak_tree_chance = 5.0f,
      .birch_tree_chance = 1.0f,
      .spruce_tree_chance = 1.0f,
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

  // Stony Plain
  Biome biome_stony_shores{
      .name = "Stony Shores",
      .foliage_color = rgb{133, 139, 83},
      .base_height = 0.0f,
      .noise_height_multiplier = 4.0f,
      .noise_spiky_multiplier = 0.0f,
      .noise_3d_multiplier = 0.0f,
      .tree_density = 0.0f,
  };

  biome_stony_shores.get_ground_cube = [](i32, i32, float) -> CubeId {
    return CubeId::STONE;
  };

  biome_stony_shores.get_foliage_cube = [](i32, float) -> CubeId {
    return CubeId::AIR;
  };

  biome_list[(size_t)BiomeId::STONY_SHORES] = biome_stony_shores;

  debug_log("Initialized biome list");

  return biome_list;
}

namespace biomes {
const Biome& get_biome(BiomeId id) {
  static const BiomeList biome_list = init_biome_list();
  return biome_list[(size_t)id];
}

Biome biome_lerp(const biomes::Biome& biome_a, const biomes::Biome& biome_b, float t) {
  const Biome& stronger_biome = (t <= 0.5f) ? biome_a : biome_b;

  float foliage_color_r = (float)biome_a.foliage_color.r * (1.0f - t) + (float)biome_b.foliage_color.r * t;
  float foliage_color_g = (float)biome_a.foliage_color.g * (1.0f - t) + (float)biome_b.foliage_color.g * t;
  float foliage_color_b = (float)biome_a.foliage_color.b * (1.0f - t) + (float)biome_b.foliage_color.b * t;
  foliage_color_r = std::clamp(foliage_color_r, 0.0f, 255.0f);
  foliage_color_g = std::clamp(foliage_color_g, 0.0f, 255.0f);
  foliage_color_b = std::clamp(foliage_color_b, 0.0f, 255.0f);

  return Biome{
      .name = stronger_biome.name,
      .foliage_color = rgb{(u8)foliage_color_r, (u8)foliage_color_g, (u8)foliage_color_b},
      .base_height = biome_a.base_height * (1.0f - t) + biome_b.base_height * t,
      .noise_height_multiplier = biome_a.noise_height_multiplier * (1.0f - t) + biome_b.noise_height_multiplier * t,
      .noise_spiky_multiplier = biome_a.noise_spiky_multiplier * (1.0f - t) + biome_b.noise_spiky_multiplier * t,
      .noise_3d_multiplier = biome_a.noise_3d_multiplier * (1.0f - t) + biome_b.noise_3d_multiplier * t,
      .get_ground_cube = stronger_biome.get_ground_cube,
      .get_foliage_cube = stronger_biome.get_foliage_cube,
  };
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
  glm::vec<3, float> foliage_color{};

  for (size_t i = 0; i < biomes.size(); i += 1) {
    const Biome& b = biomes[i];
    float w = weights[i] * b.strength;

    total_weight += w;

    blended_biome.base_height += b.base_height * w;
    blended_biome.noise_height_multiplier += b.noise_height_multiplier * w;
    blended_biome.noise_spiky_multiplier += b.noise_spiky_multiplier * w;
    blended_biome.noise_3d_multiplier += b.noise_3d_multiplier * w;
    blended_biome.strength += b.strength * w;
    blended_biome.tree_density += b.tree_density * w;
    blended_biome.oak_tree_chance += b.oak_tree_chance * w;
    blended_biome.birch_tree_chance += b.birch_tree_chance * w;
    blended_biome.spruce_tree_chance += b.spruce_tree_chance * w;
    foliage_color += glm::vec<3, float>(b.foliage_color.r, b.foliage_color.g, b.foliage_color.b) * w;

    if (weights[i] > weights[strongest_biome_index]) {
      strongest_biome_index = i;
    }
  }

  blended_biome.base_height /= total_weight;
  blended_biome.noise_height_multiplier /= total_weight;
  blended_biome.noise_spiky_multiplier /= total_weight;
  blended_biome.noise_3d_multiplier /= total_weight;
  blended_biome.strength /= total_weight;
  blended_biome.tree_density /= total_weight;
  blended_biome.oak_tree_chance /= total_weight;
  blended_biome.birch_tree_chance /= total_weight;
  blended_biome.spruce_tree_chance /= total_weight;
  foliage_color /= total_weight;
  blended_biome.foliage_color = rgb{(u8)foliage_color.r, (u8)foliage_color.g, (u8)foliage_color.b};

  const Biome& strongest_biome = biomes[strongest_biome_index];
  blended_biome.name = strongest_biome.name;
  blended_biome.get_ground_cube = strongest_biome.get_ground_cube;
  blended_biome.get_foliage_cube = strongest_biome.get_foliage_cube;

  return blended_biome;
}
} // namespace biomes
