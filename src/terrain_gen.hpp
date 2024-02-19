#include <algorithm>
#include <array>
#include <cmath>
#include <execution>
#include "chunk.hpp"
#include "fast_noise_lite.h"

namespace TerrainGen {

static FastNoiseLite noise_landform{};
static FastNoiseLite noise_heightmap{};
static FastNoiseLite noise3d{};
static FastNoiseLite noise3d_high{};

static constexpr auto indices = [] {
  std::array<size_t, CHUNK_CUBES> _indices{};
  std::iota(_indices.begin(), _indices.end(), 0);
  return _indices;
}();

static bool was_initialized = false;

static void init() {
  constexpr int seed = 11;

  noise_heightmap.SetSeed(seed);
  noise_heightmap.SetFrequency(0.0081f);
  noise_heightmap.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap.SetFractalOctaves(3);
  noise_heightmap.SetFractalGain(0.6f);

  noise3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise3d.SetSeed(seed);
  noise3d.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise3d.SetFractalOctaves(3);
  noise3d.SetFrequency(0.022625f);

  noise3d_high.SetSeed(seed);
  noise3d_high.SetFrequency(0.08775f);

  noise_landform.SetSeed(seed);
  noise_landform.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Value);
  noise_landform.SetFrequency(0.006f);
  noise_landform.SetFractalOctaves(2);
  noise_landform.SetFractalGain(0.6f);

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
    Landform /* Clifflands */ {
        .base_height = 8.0f,
        .height_multiplier = 16.0f,
        .height_high_multiplier = 0.0f,
        .noise_3d_multiplier = 2.0f,
        .high_noise_3d_multiplier = 0.0f,
        .cliff_factor = 1.0f,
    },

    Landform /* Highlands */ {
        .base_height = 16.0f,
        .height_multiplier = 75.0f,
        .height_high_multiplier = 0.5f,
        .noise_3d_multiplier = 18.0f,
        .high_noise_3d_multiplier = 0.5f,
        .cliff_factor = 0.0f,
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
        .height_multiplier = 8.0f,
        .height_high_multiplier = 0.0f,
        .noise_3d_multiplier = 0.3f,
        .high_noise_3d_multiplier = 0.0f,
        .cliff_factor = 1.0f,
    },
});

bool is_cube_landform(float x, float y, float z, Landform blended_landform) {
  if (!was_initialized) { init(); }

  float value_height = noise_heightmap.GetNoise(x, z) * blended_landform.height_multiplier;

  float value_3d = noise3d.GetNoise(x, y, z) * blended_landform.noise_3d_multiplier;
  value_3d += noise3d_high.GetNoise(x, y, z) * blended_landform.high_noise_3d_multiplier;

  float value = blended_landform.base_height + value_height + value_3d * (1.0f - blended_landform.cliff_factor * 0.5f);

  if (value > 12.0f / (0.01f + blended_landform.cliff_factor * 0.8f)) {
    float cliff_height = (noise_heightmap.GetNoise(x * 0.05f, z * 0.05f) + 1.0f) * 60.0f;
    value = cliff_height + (value * 0.35f);
  }

  if (value > y) {
    return true;
  }
  return false;
}

Landform get_landform_blended_properties(float x, float z) {
  if (!was_initialized) { init(); }

  static constexpr float landform_blending_size = 0.25f;
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

void tree_gen(Chunk* chunk, size_t cube_index) {
  CubePos cube_pos = index_to_local_pos(cube_index) + chunk->position * (int32_t)CHUNK_SIZE;

  int max_height = 0;
  for (; max_height < 16; max_height += 1) {
    CubePos cube_pos_height = cube_pos + CubePos{0, max_height + 1, 0};
    if (is_cube((float)cube_pos_height.x, (float)cube_pos_height.y, (float)cube_pos_height.z)) { break; }
  }

  max_height = std::clamp(max_height - 2, 0, 6 + (int)(noise3d_high.GetNoise((float)cube_pos.x, (float)cube_pos.z) * 3.0f));

  if (max_height <= 4) { return; }

  for (int i = 0; i < max_height; i += 1) {
    auto [chunk_pos, local_pos] = cube_to_local(cube_pos + CubePos{0, i, 0});
    if (chunk->position == chunk_pos) {
      chunk->set_cube_no_lock(local_pos, CubeId::WOOD);
    } else {
      chunk->set_cube_neigbour(chunk_pos, local_pos, CubeId::WOOD);
    }
  }

  CubePos cube_pos_crown = cube_pos + CubePos{0, max_height, 0};

  for (int x = -2; x <= 2; x += 1) {
    for (int z = -2; z <= 2; z += 1) {
      if (std::abs(x) == 2 && std::abs(z) == 2) { continue; }
      for (int y = -2; y <= 1; y += 1) {
        if (y == 1 && (std::abs(x) == 2 || std::abs(z) == 2)) { continue; }
        auto [chunk_pos, local_pos] = cube_to_local(cube_pos_crown + CubePos{x, y, z});
        if (chunk->position == chunk_pos && chunk->get_cube(local_pos) == CubeId::AIR) {
          chunk->set_cube_no_lock(local_pos, CubeId::LEAVES);
        } else {
          chunk->set_cube_neigbour(chunk_pos, local_pos, CubeId::LEAVES);
        }
      }
    }
  }
}

struct TerrainGenArray {
public:
  static const uint32_t lip_negative_x = 0;
  static const uint32_t lip_positive_x = 0;

  static const uint32_t lip_negative_y = 1;
  static const uint32_t lip_positive_y = 4;

  static const uint32_t lip_negative_z = 0;
  static const uint32_t lip_positive_z = 0;

  static constexpr auto indices = [] {
    constexpr size_t size = (CHUNK_SIZE + lip_negative_x + lip_positive_x) * (CHUNK_SIZE + lip_negative_y + lip_positive_y) * (CHUNK_SIZE + lip_negative_z + lip_positive_z);
    std::array<size_t, size> _indices{};
    std::iota(_indices.begin(), _indices.end(), 0);
    return _indices;
  }();

  TerrainGenArray(CubePos _begin, CubePos _end) {
    begin = _begin - CubePos{lip_negative_x, lip_negative_y, lip_negative_z};
    end = _end + CubePos{lip_positive_x, lip_positive_y, lip_positive_z};

    cubes.resize(indices.size());

    std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
      LocalPos local_pos = index_to_local_pos(i);
      assert(i == get_index(local_pos));
      cubes[i] = is_cube(begin.x + local_pos.x, begin.y + local_pos.y, begin.z + local_pos.z);
    });
  }

  bool is_solid(LocalPos local_pos) {
    size_t index = get_index(local_pos);
    assert(index < cubes.size());
    return cubes.at(index);
  }

  size_t get_index(LocalPos local_pos) {
    size_t index = 0;
    index += local_pos.x + lip_negative_x;
    index += (CHUNK_SIZE + lip_negative_x + lip_positive_x) * (local_pos.y + lip_negative_y);
    index += (CHUNK_SIZE + lip_negative_x + lip_positive_x) * (CHUNK_SIZE + lip_negative_y + lip_positive_y) * (local_pos.z + lip_negative_z);

    return index;
  }

  LocalPos index_to_local_pos(size_t index) {
    LocalPos local_pos;
    local_pos.z = (index / ((CHUNK_SIZE + lip_negative_x + lip_positive_x) * (CHUNK_SIZE + lip_negative_y + lip_positive_y))) % (CHUNK_SIZE + lip_negative_z + lip_positive_z);
    local_pos.z -= lip_negative_z;
    // index -= local_pos.z;

    local_pos.y = (index / ((CHUNK_SIZE + lip_negative_x + lip_positive_x))) % (CHUNK_SIZE + lip_negative_y + lip_positive_y);
    local_pos.y -= lip_negative_y;

    // index -= local_pos.y;

    local_pos.x = (index) % (CHUNK_SIZE + lip_negative_x + lip_positive_x);
    local_pos.x -= lip_negative_x;

    // index -= local_pos.x;

    // assert(index == 0);

    return local_pos;
  }

private:
  CubePos begin;
  CubePos end;
  std::vector<uint8_t> cubes;
};

void generate_chunk(Chunk* chunk) {
  if (!was_initialized) { init(); }

  auto chunk_world_pos = CubePos{chunk->position} * CubePos{CHUNK_SIZE};
  TerrainGenArray array(chunk_world_pos, chunk_world_pos + CubePos{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE});

  std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {
    WorldPos pos = index_to_local_pos(i) + chunk->position * (int32_t)CHUNK_SIZE;
    LocalPos local_pos = index_to_local_pos(i);

    bool is_cube_neg_1 = array.is_solid(local_pos + LocalPos{0, -1, 0});
    bool is_cube_0 = array.is_solid(local_pos);
    bool is_cube_1 = array.is_solid(local_pos + LocalPos{0, 1, 0});
    bool is_cube_2 = array.is_solid(local_pos + LocalPos{0, 2, 0});
    bool is_cube_3 = array.is_solid(local_pos + LocalPos{0, 3, 0});
    bool is_cube_4 = array.is_solid(local_pos + LocalPos{0, 4, 0});

    if (is_cube_0) {
      if (is_cube_1 && is_cube_2 && is_cube_3 && is_cube_4) {
        chunk->set_cube_index_no_lock(i, CubeId::STONE);
      } else if (is_cube_1) {
        chunk->set_cube_index_no_lock(i, CubeId::DIRT);
      } else {
        chunk->set_cube_index_no_lock(i, CubeId::GRASS);
      }
    } else if (float tree_noise = noise3d_high.GetNoise(pos.x * 4.0f, pos.y * 4.0f, pos.z * 4.0f);
               is_cube_neg_1 && tree_noise > 0.4f && (int)(pos.x) % (int)(4 + tree_noise * 2.5f) == 0 && (int)(pos.z) % (int)(4 + tree_noise * 2.5f) == 0) {
      tree_gen(chunk, i);
    } else if (is_cube_neg_1 && noise3d_high.GetNoise(pos.x * 4.0f, pos.y * 4.0f, pos.z * 4.0f) > 0.5f) {
      chunk->set_cube_index_no_lock(i, CubeId::GRASS_PLANT);
    }
  });
}

} // namespace TerrainGen