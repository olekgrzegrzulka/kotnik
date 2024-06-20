#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "biome.hpp"

/*
 (0, 0)
    *-------------------► humidity
    |                   |
    |                   |
    |                   |
    |                   |
    |                   |
    |                   |
    |                   |
    |                   |
    ▼-------------------*
 temperature           (1, 1)
*/

class BiomeMap final {

public:
  BiomeMap();

  const Biomes::BlendedBiome& get_biome(float humidity, float temperature) const;

  size_t get_resolution() const;

private:
  struct BiomeInfo {
    float humidity = 1.0f;
    float temperature = 1.0f;
    float power = 1.0f;
  };

  static constexpr size_t resolution = 1024;
  std::vector<Biomes::BlendedBiome> biome_map;
  std::vector<std::pair<std::unique_ptr<Biomes::Biome>, BiomeInfo>> biomes;

  template <class T>
    requires std::is_base_of_v<Biomes::Biome, T>
  void add_biome(float humidity, float temperature, float power = 1.0f) {
    std::unique_ptr<Biomes::Biome> biome = std::make_unique<T>(T{});

    biomes.emplace_back(std::make_pair(
        std::move(biome),
        BiomeInfo{.humidity = humidity, .temperature = temperature, .power = power}));
  }

  const Biomes::BlendedBiome create_blended_biome(float humidity, float temperature);

  void generate_biome_map();
};