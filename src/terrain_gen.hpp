#include <algorithm>
#include <array>
#include <cmath>
#include <execution>
#include "chunk.hpp"
#include "fast_noise_lite.h"

namespace TerrainGen {

static FastNoiseLite noise_landform{};
static FastNoiseLite noise_heightmap{};
static FastNoiseLite noise_heightmap_high{};
static FastNoiseLite noise3d{};
static FastNoiseLite noise3d_high{};

static std::array<size_t, CHUNK_CUBES> indices;

static bool was_initialized = false;
static const float SCALE = 0.8f;

static void init() {
  std::iota(indices.begin(), indices.end(), 0);

  noise_heightmap.SetSeed(555);
  noise_heightmap.SetFrequency(0.00641f / SCALE);
  noise_heightmap.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap.SetFractalOctaves(3);

  noise_heightmap_high.SetSeed(666);
  noise_heightmap_high.SetFrequency(0.0314f / SCALE);
  noise_heightmap_high.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap_high.SetFractalOctaves(2);

  noise3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise3d.SetSeed(555);
  noise3d.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise3d.SetFractalOctaves(3);
  noise3d.SetFrequency(0.01711f / SCALE);

  noise3d_high.SetSeed(444);
  noise3d_high.SetFrequency(0.059f / SCALE);

  noise_landform.SetSeed(2137);
  noise_landform.SetFrequency(0.0041f / SCALE);

  was_initialized = true;
}

struct Landform {
  float base_height = 0.0f;
  float height_multiplier = 0.0f;
  float height_high_multiplier = 0.0f;
  float noise_3d_multiplier = 0.0f;
  float high_noise_3d_multiplier = 0.0f;
  float cliff_factor = 0.0f;
};

static constexpr auto landforms = std::to_array<const Landform>({

    Landform /* Roughlands */ {
        .base_height = 8.0f,
        .height_multiplier = 60.0f,
        .height_high_multiplier = 5.0f,
        .noise_3d_multiplier = 40.0f,
        .high_noise_3d_multiplier = 4.0f,
        .cliff_factor = 0.0f,
    },

    Landform /* Clifflands */ {
        .base_height = 4.0f,
        .height_multiplier = 30.0f,
        .height_high_multiplier = 0.5f,
        .noise_3d_multiplier = 10.0f,
        .high_noise_3d_multiplier = 1.0f,
        .cliff_factor = 1.0f,
    },

    Landform /* Hillylands */ {
        .base_height = 0.0f,
        .height_multiplier = 25.0f,
        .height_high_multiplier = 0.4f,
        .noise_3d_multiplier = 5.0f,
        .high_noise_3d_multiplier = 0.4f,
        .cliff_factor = 0.0f,
    },

    Landform /* Flatlands */ {
        .base_height = 0.0f,
        .height_multiplier = 3.0f,
        .height_high_multiplier = 0.4f,
        .noise_3d_multiplier = 8.0f,
        .high_noise_3d_multiplier = 0.4f,
        .cliff_factor = 0.0f,
    },
});

bool is_cube_landform(float x, float y, float z, Landform blended_landform) {
  if (!was_initialized) { init(); }

  float value_height = noise_heightmap.GetNoise(x, z) * blended_landform.height_multiplier;
  value_height += noise_heightmap_high.GetNoise(x, z) * blended_landform.height_high_multiplier;

  float value_3d = noise3d.GetNoise(x, y, z) * blended_landform.noise_3d_multiplier;
  value_3d += noise3d_high.GetNoise(x, y, z) * blended_landform.high_noise_3d_multiplier;

  float value = blended_landform.base_height + value_height + value_3d * (1.0f - blended_landform.cliff_factor * 0.7f);

  if (value > 15.0f / (0.01f + blended_landform.cliff_factor * 0.8f)) {
    float cliff_height = (noise_heightmap.GetNoise(x * 0.04f, z * 0.04f) + 1.0f) * 35.0f;
    value = cliff_height + (value * 0.4f);
  }

  if (value > y) {
    return true;
  }
  return false;
}

Landform get_landform_blended_properties(float x, float z) {
  if (!was_initialized) { init(); }

  static constexpr float landform_blending_size = 0.75f;
  static_assert(landform_blending_size >= 0.01f && landform_blending_size <= 1.0f);

  float value = (noise_landform.GetNoise(x, z) + 1.0f) * 0.5f;
  value *= (float)landforms.size();

  size_t landform_index_left = std::floor(value);
  size_t landform_index_right = std::ceil(value);
  if (landform_index_right >= landforms.size()) { landform_index_right = landforms.size() - 1; }

  float lerp_value = value - (int32_t)value;
  lerp_value = ((lerp_value - 0.5f) / landform_blending_size) + 0.5f;
  lerp_value = std::clamp(lerp_value, 0.0f, 1.0f);

  auto landform_left = landforms.at(landform_index_left);
  auto landform_right = landforms.at(landform_index_right);

  return Landform{
      .base_height = std::lerp(landform_left.base_height, landform_right.base_height, lerp_value),
      .height_multiplier = std::lerp(landform_left.height_multiplier, landform_right.height_multiplier, lerp_value),
      .height_high_multiplier = std::lerp(landform_left.height_high_multiplier, landform_right.height_high_multiplier, lerp_value),
      .noise_3d_multiplier = std::lerp(landform_left.noise_3d_multiplier, landform_right.noise_3d_multiplier, lerp_value),
      .high_noise_3d_multiplier = std::lerp(landform_left.high_noise_3d_multiplier, landform_right.high_noise_3d_multiplier, lerp_value),
      .cliff_factor = std::lerp(landform_left.cliff_factor, landform_right.cliff_factor, lerp_value),
  };
}

bool is_cube(float x, float y, float z) {
  auto blended_landform = get_landform_blended_properties(x, z);

  return is_cube_landform(x, y, z, blended_landform);
}

void generate_chunk(Chunk* chunk) {
  if (!was_initialized) { init(); }

  std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
    auto pos = index_to_local_pos(i) + chunk->position * (int32_t)CHUNK_SIZE;

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;

    bool is_cube_neg_1 = is_cube(x, y - 1.0f, z);
    bool is_cube_0 = is_cube(x, y, z);

    if (is_cube_0) {
      // bool is_cube_1 = is_cube(x, y + 1.0f, z);
      // bool is_cube_2 = is_cube(x, y + 2.0f, z);
      // bool is_cube_3 = is_cube(x, y + 3.0f, z);
      // bool is_cube_4 = is_cube(x, y + 4.0f, z);

      std::vector<int> is_cube_parallel = {1, 2, 3, 4};
      std::for_each(std::execution::par_unseq, is_cube_parallel.begin(), is_cube_parallel.end(), [&](int& n) {
        n = (int)is_cube(x, y + n, z);
      });

      bool is_cube_1 = (bool)is_cube_parallel[0];
      bool is_cube_2 = (bool)is_cube_parallel[1];
      bool is_cube_3 = (bool)is_cube_parallel[2];
      bool is_cube_4 = (bool)is_cube_parallel[3];

      if (is_cube_1 && is_cube_2 && is_cube_3 && is_cube_4) {
        chunk->set_cube_index_no_lock(i, CubeId::STONE);
      } else if (is_cube_1) {
        chunk->set_cube_index_no_lock(i, CubeId::DIRT);
      } else {
        chunk->set_cube_index_no_lock(i, CubeId::GRASS);
      }
    } else if (is_cube_neg_1 && noise3d_high.GetNoise(x * 4.0f, y * 4.0f, z * 4.0f) > 0.7f) {
      chunk->set_cube_index_no_lock(i, CubeId::GRASS_PLANT);
    }
  });
}

} // namespace TerrainGen