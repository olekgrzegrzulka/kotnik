#include "biome_map.hpp"
#include <cmath>
#include <vector>
#include "biome.hpp"

using Biomes::BlendedBiome;

const BlendedBiome BiomeMap::create_blended_biome(float humidity, float temperature) {
  BlendedBiome blended_biome{};

  float strongest_biome_strength = 0.0;
  Biomes::Biome* strongest_biome;

  for (const auto& [biome, biome_info] : biomes) {
    float distance = std::hypot(
        std::abs(humidity - biome_info.humidity),
        std::abs(temperature - biome_info.temperature));
    distance /= biome_info.power;

    float strength = 1.0f / std::pow((distance + 0.001f), 3.0f);

    if (strength > strongest_biome_strength) {
      strongest_biome_strength = strength;
      strongest_biome = biome.get();
    }

    blended_biome.add(strength, biome.get());
  }

  assert(strongest_biome);
  // blended_biome.add(strongest_biome_strength, strongest_biome);
  blended_biome.compute_values();
  return blended_biome;
}

void BiomeMap::generate_biome_map() {
  for (size_t humidity_x = 0; humidity_x < resolution; humidity_x += 1) {
    for (size_t temperature_y = 0; temperature_y < resolution; temperature_y += 1) {
      float humidity = (float)humidity_x / (resolution - 1);
      float temperature = (float)temperature_y / (resolution - 1);

      auto blended_biome = create_blended_biome(humidity, temperature);
      biome_map.emplace_back(blended_biome);
    }
  }
}

BiomeMap::BiomeMap() {
  biome_map.reserve(resolution * resolution);

  add_biome<Biomes::BiomeFlatlands>(0.3f, 0.2f, 1.0f);
  add_biome<Biomes::BiomeHighlands>(0.4f, 0.1f, 1.15f);
  add_biome<Biomes::BiomeHillylands>(0.35f, 0.15f, 1.0f);
  add_biome<Biomes::BiomeDesert>(0.1f, 0.4f, 1.0f);
  add_biome<Biomes::BiomeOcean>(1.0f, 1.0f, 2.8f);

  generate_biome_map();
}

size_t BiomeMap::get_resolution() const {
  return resolution;
}

const BlendedBiome& BiomeMap::get_biome(float humidity, float temperature) const {
  size_t humidity_x_min = static_cast<size_t>(std::floor(humidity * resolution));
  size_t temperature_y_min = static_cast<size_t>(std::floor(temperature * resolution));

  return biome_map[humidity_x_min + temperature_y_min * resolution];
}