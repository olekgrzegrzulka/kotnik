#include "biome.hpp"

namespace Biomes {

void BlendedBiome::compute_values() {
  // Normalize strengths, so their sum will converge to 1.0
  if (biomes.empty()) { return; }

  float sum = 0.0;
  float biggest_strength = 0.0f;

  for (const auto& [strength, biome] : biomes) {
    sum += strength;
    if (strength > biggest_strength) {
      biggest_strength = strength;
      strongest_biome = biome;
    }
  }

  strongest_biome_strength = biggest_strength / sum;

  for (auto& [strength, biome] : biomes) {
    strength /= sum;

    base_height += biome->get_base_height() * strength;
    noise_height_multiplier += biome->get_noise_height_multiplier() * strength;
    noise_spiky_multiplier += biome->get_noise_spiky_multiplier() * strength;
    noise_3d_multiplier += biome->get_noise_3d_multiplier() * strength;
  }
}

}; // namespace Biomes