#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include "common.hpp"

namespace Biomes {

struct Biome {
  virtual constexpr std::string get_name() const = 0;

  virtual constexpr float get_base_height() const = 0;
  virtual constexpr float get_noise_height_multiplier() const = 0;
  virtual constexpr float get_noise_spiky_multiplier() const = 0;
  virtual constexpr float get_noise_3d_multiplier() const = 0;

  virtual constexpr std::optional<int32_t> get_water_level() const = 0;
  virtual constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const = 0;

  virtual ~Biome() = default;
};

struct BiomeFlatlands final : public Biome {
  constexpr std::string
  get_name() const override { return "Flatlands"; }

  constexpr float
  get_base_height() const override { return 0.0f; }

  constexpr float
  get_noise_height_multiplier() const override { return 10.0f; }

  constexpr float
  get_noise_spiky_multiplier() const override { return 0.0f; }

  constexpr float
  get_noise_3d_multiplier() const override { return 2.0f; }

  constexpr std::optional<int32_t>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 1) {
      return CubeId::DIRT;
    } else if (depth == 0) {
      return CubeId::GRASS;
    } else if (depth == -1 && rng > 0.7f) {
      return CubeId::GRASS_PLANT;
    }
    return CubeId::AIR;
  }
};

struct BiomeDesert final : public Biome {
  constexpr std::string
  get_name() const override { return "Desert"; }

  constexpr float
  get_base_height() const override { return 0.0f; }

  constexpr float
  get_noise_height_multiplier() const override { return 15.0f; }

  constexpr float
  get_noise_spiky_multiplier() const override { return 0.0f; }

  constexpr float
  get_noise_3d_multiplier() const override { return 3.0f; }

  constexpr std::optional<int32_t>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 0) {
      return CubeId::SAND;
    }
    return CubeId::AIR;
  }
};

struct BiomeHighlands final : public Biome {
  constexpr std::string
  get_name() const override { return "Highlands"; }

  constexpr float
  get_base_height() const override { return 0.0f; }

  constexpr float
  get_noise_height_multiplier() const override { return 70.0f; }

  constexpr float
  get_noise_spiky_multiplier() const override { return 0.0f; }

  constexpr float
  get_noise_3d_multiplier() const override { return 60.0f; }

  constexpr std::optional<int32_t>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 1) {
      return CubeId::DIRT;
    } else if (depth == 0) {
      return CubeId::GRASS;
    } else if (depth == -1 && rng > 0.7f) {
      return CubeId::GRASS_PLANT;
    }
    return CubeId::AIR;
  }
};

struct BiomeHillylands final : public Biome {
  constexpr std::string
  get_name() const override { return "Hillylands"; }

  constexpr float
  get_base_height() const override { return 0.0f; }

  constexpr float
  get_noise_height_multiplier() const override { return 25.0f; }

  constexpr float
  get_noise_spiky_multiplier() const override { return 0.0f; }

  constexpr float
  get_noise_3d_multiplier() const override { return 12.0f; }

  constexpr std::optional<int32_t>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 1) {
      return CubeId::DIRT;
    } else if (depth == 0) {
      return CubeId::GRASS;
    } else if (depth == -1 && rng > 0.7f) {
      return CubeId::GRASS_PLANT;
    }
    return CubeId::AIR;
  }
};

struct BiomeOcean final : public Biome {
  constexpr std::string
  get_name() const override { return "Ocean"; }

  constexpr float
  get_base_height() const override { return -50.0f; }

  constexpr float
  get_noise_height_multiplier() const override { return 5.0f; }

  constexpr float
  get_noise_spiky_multiplier() const override { return 0.0f; }

  constexpr float
  get_noise_3d_multiplier() const override { return 1.0f; }

  constexpr std::optional<int32_t>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 0) {
      return CubeId::SAND;
    }
    return CubeId::AIR;
  }
};

struct BlendedBiome final : public Biome {
  float strongest_biome_strength;

  void add(float strength, Biome* biome) {
    biomes.emplace_back(std::make_pair(strength, biome));
  }

  void compute_values();

  std::vector<std::pair<float, Biome*>> biomes;

  const Biome* strongest_biome;

  virtual constexpr std::string get_name() const override {
    return strongest_biome ? (strongest_biome->get_name()) : "";
  }

  virtual constexpr float get_base_height() const override { return base_height; }

  virtual constexpr float get_noise_height_multiplier() const override { return noise_height_multiplier; }

  virtual constexpr float get_noise_spiky_multiplier() const override { return noise_spiky_multiplier; }

  virtual constexpr float get_noise_3d_multiplier() const override { return noise_3d_multiplier; }

  virtual constexpr std::optional<int32_t> get_water_level() const override {
    return (strongest_biome) ? (strongest_biome->get_water_level()) : (std::optional<int32_t>{});
  }

  virtual constexpr CubeId get_ground_cube(int32_t y, int32_t depth, float rng) const override {
    return (strongest_biome) ? (strongest_biome->get_ground_cube(y, depth, rng)) : CubeId::AIR;
  }

private:
  float base_height;
  float noise_height_multiplier;
  float noise_spiky_multiplier;
  float noise_3d_multiplier;
};
}; // namespace Biomes