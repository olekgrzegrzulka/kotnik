#include "biome_map.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
#include "biome.hpp"

using Biomes::BlendedBiome;

const BlendedBiome BiomeMap::create_blended_biome(float humidity, float temperature) {
  BlendedBiome blended_biome{};

  float strongest_biome_strength;

  for (const auto& [biome, biome_info] : biomes) {
    float distance = std::abs(temperature - biome_info.temperature) + std::abs(humidity - biome_info.humidity);
    distance /= biome_info.power;

    float strength = 1.0f / std::pow((distance + 0.001f), 2.0f);

    if (strength > strongest_biome_strength) {
      strongest_biome_strength = strength;
    }

    blended_biome.add(strength, biome.get());
  }

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

  add_biome<Biomes::BiomeFlatlands>(0.5f, 0.5f, 1.0f);
  add_biome<Biomes::BiomeHighlands>(0.6f, 0.4f, 1.15f);
  add_biome<Biomes::BiomeHillylands>(0.2f, 0.4f, 1.0f);
  add_biome<Biomes::BiomeDesert>(0.2f, 0.8f, 1.0f);

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