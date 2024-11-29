#include <iterator>
#include <map>
#include <memory>
#include <tuple>
#include <glm/gtx/norm.hpp>
#include <sys/types.h>
#include "biome.hpp"
#include "common.hpp"

#include "stb_image.h"

using rgb = std::tuple<u8, u8, u8>;
using Biome = biomes::Biome;
using BiomeId = biomes::BiomeId;

static constexpr int biome_blending_sample_count = 2;
static constexpr int biome_blending_step_size = 2;

struct BiomeMap {
  std::vector<Biome> data{};
  i32 extents{};

  const biomes::Biome& get_biome(float humidity, float temperature) const {
    i32 x = std::min<i32>(extents * humidity, extents - 1);
    i32 y = std::min<i32>(extents * temperature, extents - 1);
    return data.at(x + y * extents);
  }
};

static const std::map<rgb, BiomeId>
    bitmap_color_to_biome_id{
        {{18, 64, 132}, BiomeId::DEEP_OCEAN},
        {{40, 92, 196}, BiomeId::OCEAN},
        {{245, 197, 120}, BiomeId::BEACH},
        // {{139, 147, 175}, BiomeId::ROCKY_SHORE},
        {{156, 219, 67}, BiomeId::FLATLANDS},
        // {{31, 163, 80}, BiomeId::FOREST},
        // {{26, 122, 62}, BiomeId::DEEP_FOREST},
        {{74, 84, 98}, BiomeId::HIGHLANDS},
        {{26, 122, 62}, BiomeId::HILLYLANDS},
        // {{106, 152, 42}, BiomeId::SWAMPLANDS},
        {{251, 170, 42}, BiomeId::DESERT},
    };

const std::unique_ptr<BiomeMap> init_biome_map() {
  auto st = ScopeTimer{"Generating the biome map"};

  int width, height, channels;
  stbi_uc* data = stbi_load("./assets/biomemap.png", &width, &height, &channels, STBI_rgb_alpha);

  if (!data) {
    debug_error("biomemap: couldn't open biomemap.png");
  }

  if (width != height) {
    debug_error("biomemap: aspect ratio must be 1:1");
  }

  auto biome_map = std::make_unique<BiomeMap>();
  biome_map->extents = width;

  auto get_bitmap_color = [&](int x, int y) -> rgb {
    if (x < 0 || x >= width || y < 0 || y >= height) {
      debug_error("biomemap: bitmap coordinates out of bounds");
      return rgb{};
    }

    int offset = (x + y * width) * channels;
    rgb color = {(u8)data[offset + 0], (u8)data[offset + 1], (u8)data[offset + 2]};
    return color;
  };

  auto color_to_biome_id = [&](rgb color) -> BiomeId {
    auto it = bitmap_color_to_biome_id.find(color);
    if (it == bitmap_color_to_biome_id.end()) {
      debug_warn("biomemap: unknown biome color rgb(", (int)std::get<0>(color), ", ", (int)std::get<1>(color), ", ", (int)std::get<2>(color), "), defaulting to Flatlands");
      return biomes::BiomeId::FLATLANDS;
    }
    return (*it).second;
  };

  auto blend_biome_from_bitmap = [&](int x, int y) -> Biome {
    std::vector<float> biome_weights;
    std::vector<BiomeId> biome_ids;

    for (int oy = -biome_blending_sample_count * biome_blending_step_size; oy <= biome_blending_sample_count * biome_blending_step_size; oy += biome_blending_step_size) {
      for (int ox = -biome_blending_sample_count * biome_blending_step_size; ox <= biome_blending_sample_count * biome_blending_step_size; ox += biome_blending_step_size) {
        if ((x + ox) < 0 || (x + ox) >= width || (y + oy) < 0 || (y + oy) >= height) { continue; }

        // float weight = 1.0f / (1.0f + std::abs(ox) + std::abs(oy));
        float weight = 1.0f;

        BiomeId sampled_biome = color_to_biome_id(get_bitmap_color(x + ox, y + oy));

        if (auto it = std::find(biome_ids.begin(), biome_ids.end(), sampled_biome); it != biome_ids.end()) {
          size_t i = std::distance(biome_ids.begin(), it);
          biome_weights[i] += weight;
        } else {
          biome_ids.emplace_back(sampled_biome);
          biome_weights.emplace_back(weight);
        }
      }
    }

    return biomes::biome_weighted_average(biome_weights, biome_ids);
  };

  for (int y = 0; y < height; y += 1) {
    for (int x = 0; x < width; x += 1) {
      biome_map->data.emplace_back(blend_biome_from_bitmap(x, y));
    }
  }

  stbi_image_free(data);

  return biome_map;
}

namespace biomemap {
const Biome& get_biome(float humidity, float temperature) {
  static const std::unique_ptr<BiomeMap> biome_map = init_biome_map();

  return biome_map->get_biome(humidity, temperature);
}
} // namespace biomemap