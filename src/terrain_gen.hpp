#include <algorithm>
#include <array>
#include <execution>
#include "chunk.hpp"
#include "fast_noise_lite.h"

namespace TerrainGen {

static FastNoiseLite noise3d{};
static FastNoiseLite noise3d_high{};
static FastNoiseLite noise_height{};
static FastNoiseLite noise_height_high{};

static std::array<size_t, CHUNK_CUBES> indices;

static bool was_initialized = false;
static const float SCALE = 1.25f;
static const float HEIGHT = 2.1f;

static void init() {
  std::iota(indices.begin(), indices.end(), 0);

  noise3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise3d.SetSeed(555);
  noise3d.SetFrequency(0.0211f / SCALE);

  noise3d_high.SetSeed(444);
  noise3d_high.SetFrequency(0.069f / SCALE);

  noise_height.SetSeed(77);
  noise_height.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
  // noise_height.SetFractalOctaves(2);
  noise_height.SetFrequency(0.0026f / SCALE);
  noise_height.SetFractalGain(2.0f);

  noise_height_high.SetSeed(452345);
  noise_height_high.SetFractalOctaves(2);
  noise_height_high.SetFrequency(0.021f / SCALE);
  noise_height_high.SetFractalGain(2.0f);

  was_initialized = true;
}

bool is_cube(float x, float y, float z) {
  if (!was_initialized) { init(); }
  float value_height = 0.8f;

  float noise3d_high_value = noise3d_high.GetNoise(x, y, z);

  float value_3d = (noise3d.GetNoise(x, y, z) + 1.0f) * 0.5f;

  if (value_height * 80.0f * (1.0f + value_3d * 0.5f + noise3d_high_value * 0.08f) * HEIGHT > y) {
    return true;
  }
  return false;
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