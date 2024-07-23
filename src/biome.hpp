#pragma once

#include <optional>
#include <string>
#include "common.hpp"
#include "cubes.hpp"

namespace Biomes {

struct Biome {
  virtual constexpr std::string get_name() const = 0;

  virtual constexpr float get_base_height() const = 0;
  virtual constexpr float get_noise_height_multiplier() const = 0;
  virtual constexpr float get_noise_spiky_multiplier() const = 0;
  virtual constexpr float get_noise_3d_multiplier() const = 0;

  virtual constexpr std::optional<i32> get_water_level() const = 0;
  virtual constexpr CubeId get_ground_cube(i32 y, i32 depth, float rng) const = 0;
  virtual constexpr CubeId get_foliage_cube(i32 y, float rng) const = 0;
  virtual constexpr CubeId get_air_cube(i32 y, float rng) const = 0;

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

  constexpr std::optional<i32>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube([[maybe_unused]] i32 y, i32 depth, [[maybe_unused]] float rng) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 1) {
      return CubeId::DIRT;
    } else if (depth == 0) {
      return CubeId::GRASS;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_foliage_cube([[maybe_unused]] i32 y, float rng) const override {
    if (rng > 0.7f) {
      return CubeId::GRASS_PLANT;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_air_cube([[maybe_unused]] i32 y, [[maybe_unused]] float rng) const override {
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

  constexpr std::optional<i32>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(i32, i32 depth, float) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 0) {
      return CubeId::SAND;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_foliage_cube(i32, float) const override {
    return CubeId::AIR;
  }

  constexpr CubeId get_air_cube(i32, float) const override {
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
  get_noise_3d_multiplier() const override { return 40.0f; }

  constexpr std::optional<i32>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(i32, i32 depth, float) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 1) {
      return CubeId::DIRT;
    } else if (depth == 0) {
      return CubeId::GRASS;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_foliage_cube(i32, float rng) const override {
    if (rng > 0.7f) {
      return CubeId::GRASS_PLANT;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_air_cube(i32, float) const override {
    return CubeId::AIR;
  }
};

struct BiomeHillylands final : public Biome {
  constexpr std::string
  get_name() const override { return "Hillylands"; }

  constexpr float
  get_base_height() const override { return 0.0f; }

  constexpr float
  get_noise_height_multiplier() const override { return 20.0f; }

  constexpr float
  get_noise_spiky_multiplier() const override { return 0.0f; }

  constexpr float
  get_noise_3d_multiplier() const override { return 10.0f; }

  constexpr std::optional<i32>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(i32, i32 depth, float) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 1) {
      return CubeId::DIRT;
    } else if (depth == 0) {
      return CubeId::GRASS;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_foliage_cube(i32, float rng) const override {
    if (rng > 0.7f) {
      return CubeId::GRASS_PLANT;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_air_cube(i32, float) const override {
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

  constexpr std::optional<i32>
  get_water_level() const override { return {}; }

  constexpr CubeId get_ground_cube(i32, i32 depth, float) const override {
    if (depth >= 4) {
      return CubeId::STONE;
    } else if (depth >= 0) {
      return CubeId::SAND;
    }
    return CubeId::AIR;
  }

  constexpr CubeId get_foliage_cube(i32, float) const override {
    return CubeId::AIR;
  }

  constexpr CubeId get_air_cube(i32, float) const override {
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

  constexpr std::string get_name() const override {
    return strongest_biome ? (strongest_biome->get_name()) : "";
  }

  constexpr float get_base_height() const override { return base_height; }

  constexpr float get_noise_height_multiplier() const override { return noise_height_multiplier; }

  constexpr float get_noise_spiky_multiplier() const override { return noise_spiky_multiplier; }

  constexpr float get_noise_3d_multiplier() const override { return noise_3d_multiplier; }

  constexpr std::optional<i32> get_water_level() const override {
    return (strongest_biome) ? (strongest_biome->get_water_level()) : (std::optional<i32>{});
  }

  constexpr CubeId get_ground_cube(i32 y, i32 depth, float rng) const override {
    return (strongest_biome) ? (strongest_biome->get_ground_cube(y, depth, rng)) : CubeId::AIR;
  }

  constexpr CubeId get_foliage_cube(i32 y, float rng) const override {
    return (strongest_biome) ? (strongest_biome->get_foliage_cube(y, rng)) : CubeId::AIR;
  }

  constexpr CubeId get_air_cube(i32 y, float rng) const override {
    return (strongest_biome) ? (strongest_biome->get_air_cube(y, rng)) : CubeId::AIR;
  }

private:
  float base_height;
  float noise_height_multiplier;
  float noise_spiky_multiplier;
  float noise_3d_multiplier;
};
}; // namespace Biomes