#include "world_gen.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <optional>
#include <vector>
#include "array3d.hpp"
#include "biome.hpp"
#include "biome_map.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "fast_noise_lite.h"
#include "random.hpp"
#include "world.hpp"

struct ChunkGenArray {
public:
  static constexpr auto noise_grid_points_x = std::to_array({0, 5, 10, 15, 20, 25, 31});
  static constexpr auto noise_grid_points_y = std::to_array({-1, 4, 9, 14, 19, 24, 29, 35});
  static constexpr auto noise_grid_points_z = std::to_array({0, 5, 10, 15, 20, 25, 31});

  static constexpr auto biome_grid_points_x = std::to_array({0, 5, 10, 15, 20, 25, 31});
  static constexpr auto biome_grid_points_z = std::to_array({0, 5, 10, 15, 20, 25, 31});

  static constexpr i32 lip_negative_y = 1;
  static constexpr i32 lip_positive_y = 4;
  static constexpr CubePos begin = CubePos{0, -lip_negative_y, 0};
  static constexpr CubePos end = CubePos{Chunk::chunk_size, Chunk::chunk_size, Chunk::chunk_size} + CubePos{0, lip_positive_y, 0};
  static constexpr glm::vec<3, size_t> array_size = {
      Chunk::chunk_size,
      Chunk::chunk_size + lip_negative_y + lip_positive_y,
      Chunk::chunk_size,
  };

  const WorldGen& world_gen;

private:
  Array3D<bool, array_size.x, array_size.y, array_size.z> ground_array{begin.x, begin.y, begin.z};

public:
  ChunkPos chunk_pos{};
  bool initialized = false;
  Array3D<biomes::Biome, array_size.x, 1, array_size.z> biome_array{begin.x, 0, begin.z};
  Array3D<float, array_size.x, 1, array_size.z> noise_heightmap_array{begin.x, 0, begin.z};

private:
  bool empty = true;

public:
  void create_biome_array() {
    // Get biomes at grid points
    for (i32 x : biome_grid_points_x) {
      for (i32 z : biome_grid_points_z) {
        CubePos cube_pos = chunk_pos * Chunk::chunk_size + LocalPos{x, 0, z};
        biome_array.set({x, 0, z}, world_gen.get_blended_biome(cube_pos));
      }
    }

    // Interpolate biome array
    {
      size_t grid_point_x_high_i = 1;
      for (i32 x = begin.x; x < end.x; x += 1) {

        while (x > biome_grid_points_x[grid_point_x_high_i]) {
          grid_point_x_high_i += 1;
        }

        size_t grid_point_z_high_i = 1;
        for (i32 z = begin.z; z < end.z; z += 1) {
          while (z > biome_grid_points_z[grid_point_z_high_i]) {
            grid_point_z_high_i += 1;
          }

          const glm::vec<2, i32> grid_point_low{
              biome_grid_points_x[grid_point_x_high_i - 1],
              biome_grid_points_z[grid_point_z_high_i - 1],
          };

          const glm::vec<2, i32> grid_point_high{
              biome_grid_points_x[grid_point_x_high_i],
              biome_grid_points_z[grid_point_z_high_i],
          };

          const bool is_grid_point_x = (x == grid_point_low.x) || (x == grid_point_high.x);
          const bool is_grid_point_z = (z == grid_point_low.y) || (z == grid_point_high.y);
          if (is_grid_point_x && is_grid_point_z) { continue; }

          float x_coefficient = (x - grid_point_low.x) / (float)(grid_point_high.x - grid_point_low.x);
          float z_coefficient = (z - grid_point_low.y) / (float)(grid_point_high.y - grid_point_low.y);

          using biomes::Biome;

          std::array<Biome, 4> biomes = {
              biome_array.at({grid_point_low.x, 0, grid_point_low.y}),
              biome_array.at({grid_point_low.x, 0, grid_point_high.y}),
              biome_array.at({grid_point_high.x, 0, grid_point_low.y}),
              biome_array.at({grid_point_high.x, 0, grid_point_high.y}),
          };

          std::array<float, 4> weights = {
              biomes[0].strength * (1.0f - x_coefficient) * (1.0f - z_coefficient),
              biomes[1].strength * (1.0f - x_coefficient) * z_coefficient,
              biomes[2].strength * x_coefficient * (1.0f - z_coefficient),
              biomes[3].strength * x_coefficient * z_coefficient,
          };

          biome_array.set({x, 0, z}, biomes::biome_weighted_average(weights, biomes));
        }
      }
    }
  }

  void create_heightmap() {
    for (i32 x : noise_grid_points_x) {
      for (i32 z : noise_grid_points_z) {
        float noise_heightmap = world_gen.get_heightmap_noise(chunk_pos * Chunk::chunk_size + LocalPos{x, 0, z}, biome_array.at({x, 0, z}));
        noise_heightmap_array.set({x, 0, z}, noise_heightmap);
      }
    }

    // Interpolate
    size_t grid_point_x_high_i = 1;
    for (i32 x = begin.x; x < end.x; x += 1) {

      while (x > noise_grid_points_x[grid_point_x_high_i]) {
        grid_point_x_high_i += 1;
      }

      size_t grid_point_z_high_i = 1;
      for (i32 z = begin.z; z < end.z; z += 1) {

        while (z > noise_grid_points_z[grid_point_z_high_i]) {
          grid_point_z_high_i += 1;
        }

        const glm::vec<2, i32> grid_point_low{
            noise_grid_points_x[grid_point_x_high_i - 1],
            noise_grid_points_z[grid_point_z_high_i - 1],
        };

        const glm::vec<2, i32> grid_point_high{
            noise_grid_points_x[grid_point_x_high_i],
            noise_grid_points_z[grid_point_z_high_i],
        };

        const bool is_grid_point_x = (x == grid_point_low.x) || (x == grid_point_high.x);
        const bool is_grid_point_z = (z == grid_point_low.y) || (z == grid_point_high.y);
        if (is_grid_point_x && is_grid_point_z) { continue; }

        const float x_coefficient = (x - grid_point_low.x) / (float)(grid_point_high.x - grid_point_low.x);
        const float z_coefficient = (z - grid_point_low.y) / (float)(grid_point_high.y - grid_point_low.y);

        const float noise_heightmap_x_low = noise_heightmap_array.at({grid_point_low.x, 0, grid_point_low.y}) * (1.0f - z_coefficient) + noise_heightmap_array.at({grid_point_low.x, 0, grid_point_high.y}) * z_coefficient;
        const float noise_heightmap_x_high = noise_heightmap_array.at({grid_point_high.x, 0, grid_point_low.y}) * (1.0f - z_coefficient) + noise_heightmap_array.at({grid_point_high.x, 0, grid_point_high.y}) * z_coefficient;

        const float noise_heightmap_value = noise_heightmap_x_low * (1.0f - x_coefficient) + noise_heightmap_x_high * x_coefficient;
        noise_heightmap_array.set({x, 0, z}, noise_heightmap_value);
      }
    }
  }

  ChunkGenArray(const WorldGen& wg) : world_gen(wg) {}

  void initialize(ChunkPos chunk_pos_) {

    Array3D<float, array_size.x, array_size.y, array_size.z> noise_3d_array{begin.x, begin.y, begin.z};
    ground_array = Array3D<bool, array_size.x, array_size.y, array_size.z>{begin.x, begin.y, begin.z};
    empty = true;

    if (!initialized || chunk_pos_.x != chunk_pos.x || chunk_pos_.z != chunk_pos.z) {
      chunk_pos = chunk_pos_;
      create_biome_array();
      create_heightmap();
      initialized = true;
    }

    chunk_pos = chunk_pos_;

    // Fill 3d noise at grid points
    for (i32 x : noise_grid_points_x) {
      for (i32 z : noise_grid_points_z) {
        for (i32 y : noise_grid_points_y) {
          const CubePos cube_pos = chunk_pos * Chunk::chunk_size + LocalPos{x, y, z};
          const float noise_3d = world_gen.get_3d_noise(cube_pos, biome_array.at({x, 0, z}));
          noise_3d_array.set({x, y, z}, noise_3d);
          const bool is_solid = world_gen.is_ground(cube_pos, biome_array.at({x, 0, z}), noise_heightmap_array.at({x, 0, z}), noise_3d);
          if (is_solid) { empty = false; }
          ground_array.set({x, y, z}, is_solid);
        }
      }
    }

    if (empty) { return; }

    // Interpolate 3d noise
    size_t grid_point_x_high_i = 1;
    for (i32 x = begin.x; x < end.x; x += 1) {

      while (x > noise_grid_points_x[grid_point_x_high_i]) {
        grid_point_x_high_i += 1;
      }

      size_t grid_point_y_high_i = 1;
      for (i32 y = begin.y; y < end.y; y += 1) {

        while (y > noise_grid_points_y[grid_point_y_high_i]) {
          grid_point_y_high_i += 1;
        }

        size_t grid_point_z_high_i = 1;
        for (i32 z = begin.z; z < end.z; z += 1) {

          while (z > noise_grid_points_z[grid_point_z_high_i]) {
            grid_point_z_high_i += 1;
          }

          const glm::vec<3, i32> grid_point_low{
              noise_grid_points_x[grid_point_x_high_i - 1],
              noise_grid_points_y[grid_point_y_high_i - 1],
              noise_grid_points_z[grid_point_z_high_i - 1],
          };

          const glm::vec<3, i32> grid_point_high{
              noise_grid_points_x[grid_point_x_high_i],
              noise_grid_points_y[grid_point_y_high_i],
              noise_grid_points_z[grid_point_z_high_i],
          };

          const bool is_grid_point_x = (x == grid_point_low.x) || (x == grid_point_high.x);
          const bool is_grid_point_y = (y == grid_point_low.y) || (y == grid_point_high.y);
          const bool is_grid_point_z = (z == grid_point_low.z) || (z == grid_point_high.z);
          if (is_grid_point_x && is_grid_point_y && is_grid_point_z) { continue; }

          const float x_coefficient = (x - grid_point_low.x) / (float)(grid_point_high.x - grid_point_low.x);
          const float y_coefficient = (y - grid_point_low.y) / (float)(grid_point_high.y - grid_point_low.y);
          const float z_coefficient = (z - grid_point_low.z) / (float)(grid_point_high.z - grid_point_low.z);

          const float noise_3d_x_low_z_low = (noise_3d_array.at({grid_point_low.x, grid_point_low.y, grid_point_low.z}) * (1.0f - y_coefficient) +
                                              noise_3d_array.at({grid_point_low.x, grid_point_high.y, grid_point_low.z}) * y_coefficient);
          const float noise_3d_x_low_z_high = (noise_3d_array.at({grid_point_low.x, grid_point_low.y, grid_point_high.z}) * (1.0f - y_coefficient) +
                                               noise_3d_array.at({grid_point_low.x, grid_point_high.y, grid_point_high.z}) * y_coefficient);
          const float noise_3d_x_high_z_low = (noise_3d_array.at({grid_point_high.x, grid_point_low.y, grid_point_low.z}) * (1.0f - y_coefficient) +
                                               noise_3d_array.at({grid_point_high.x, grid_point_high.y, grid_point_low.z}) * y_coefficient);
          const float noise_3d_x_high_z_high = (noise_3d_array.at({grid_point_high.x, grid_point_low.y, grid_point_high.z}) * (1.0f - y_coefficient) +
                                                noise_3d_array.at({grid_point_high.x, grid_point_high.y, grid_point_high.z}) * y_coefficient);

          const float noise_3d_x_low = noise_3d_x_low_z_low * (1.0f - z_coefficient) + noise_3d_x_low_z_high * z_coefficient;
          const float noise_3d_x_high = noise_3d_x_high_z_low * (1.0f - z_coefficient) + noise_3d_x_high_z_high * z_coefficient;

          const float noise_3d_value = noise_3d_x_low * (1.0f - x_coefficient) + noise_3d_x_high * x_coefficient;
          const float noise_heightmap_value = noise_heightmap_array.at({x, 0, z});
          const CubePos cube_pos = chunk_pos * Chunk::chunk_size + LocalPos{x, y, z};
          ground_array.set({x, y, z}, world_gen.is_ground(cube_pos, biome_array.at({x, 0, z}), noise_heightmap_value, noise_3d_value));
        }
      }
    }
  }

  bool is_solid_unsafe(const LocalPos& local_pos) const {
    return ground_array.at(local_pos);
  }

  std::optional<bool> is_solid(const LocalPos& local_pos) const {
    if (!ground_array.has_index(local_pos)) { return std::nullopt; }
    return is_solid_unsafe(local_pos);
  }

  bool is_empty() const { return empty; }
};

WorldGen::WorldGen(World& w, i32 seed) : world(w) {
  constexpr float freq_biome = 0.00135f * 0.85f;
  constexpr float freq_height = 0.0075f * 0.85f;
  constexpr float freq_3d = 0.00515f * 0.85f;
  noise_heightmap.SetSeed(seed);
  noise_heightmap.SetFrequency(freq_height);
  noise_heightmap.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap.SetFractalOctaves(3);
  noise_heightmap.SetFractalGain(0.4f);
  noise_heightmap.SetFractalLacunarity(2.57f);

  noise_3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_3d.SetFrequency(freq_3d);
  noise_3d.SetSeed(seed);
  noise_3d.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_3d.SetFractalOctaves(3);
  noise_3d.SetFractalGain(0.6085f);
  noise_3d.SetFractalLacunarity(2.481f);
  noise_3d.SetDomainWarpType(FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid);
  noise_3d.SetDomainWarpAmp(80.0f);

  noise_humidity.SetSeed(seed + 1);
  noise_humidity.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_humidity.SetFrequency(freq_biome);
  noise_humidity.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_humidity.SetFractalOctaves(4);
  noise_humidity.SetFractalLacunarity(2.2f);
  noise_humidity.SetFractalGain(0.4f);

  noise_temperature.SetSeed(seed + 2);
  noise_temperature.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_temperature.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_temperature.SetFrequency(freq_biome);
  noise_temperature.SetFractalOctaves(4);
  noise_temperature.SetFractalLacunarity(2.2f);
  noise_temperature.SetFractalGain(0.4f);
}

float WorldGen::get_humidity(WorldPos world_pos) const {
  return std::clamp(noise_humidity.GetNoise(world_pos.x, world_pos.z) * 0.5f + 0.5f, 0.0f, 1.0f);
}

float WorldGen::get_temperature(WorldPos world_pos) const {
  return std::clamp(noise_temperature.GetNoise(world_pos.x, world_pos.z) * 0.5f + 0.5f, 0.0f, 1.0f);
}

biomes::Biome WorldGen::get_blended_biome(WorldPos world_pos) const {
  return biomemap::get_biome(get_humidity(world_pos), get_temperature(world_pos));
}

biomes::Biome WorldGen::get_blended_biome(float humidity, float temperature) const {
  return biomemap::get_biome(humidity, temperature);
}

float WorldGen::get_heightmap_noise(WorldPos pos, const biomes::Biome& blended_biome) const {
  return (noise_heightmap.GetNoise(pos.x, pos.z) + 1.0f) * 0.5f * blended_biome.noise_height_multiplier;
}

float WorldGen::get_3d_noise(WorldPos pos, const biomes::Biome& blended_biome) const {
  return noise_3d.GetNoise(pos.x, pos.y * 1.0f, pos.z) * blended_biome.noise_3d_multiplier;
}

bool WorldGen::is_ground(WorldPos pos, const biomes::Biome& blended_biome, float noise_heightmap_value, float noise_3d_value) const {
  noise_3d_value = 1.0f + noise_3d_value * 0.032f;
  float value = (blended_biome.base_height + noise_heightmap_value) * noise_3d_value;
  return value > pos.y;
}

bool WorldGen::is_ground(WorldPos pos, const biomes::Biome& blended_biome) const {
  return is_ground(pos, blended_biome, get_heightmap_noise(pos, blended_biome), get_3d_noise(pos, blended_biome));
}

bool WorldGen::is_ground(WorldPos pos) const {
  auto blended_biome = get_blended_biome(pos);
  return is_ground(pos, blended_biome, get_heightmap_noise(pos, blended_biome), get_3d_noise(pos, blended_biome));
}

void gen_tree_poplar(Chunk* chunk, LocalPos at, bool birch = false) {
  i32 tree_height = StaticRandom::get().next<i32>(4, 6);

  for (i32 ox = -2; ox <= 2; ox += 1) {
    for (i32 oy = tree_height - 2; oy <= tree_height + 1; oy += 1) {
      for (i32 oz = -2; oz <= 2; oz += 1) {
        bool ox_edge = ox == -2 || ox == 2;
        bool oy_edge = oy == tree_height - 2 || oy == tree_height + 1;
        bool oz_edge = oz == -2 || oz == 2;
        LocalPos local_pos_leaves = {at.x + ox, at.y + oy, at.z + oz};
        if ((int)ox_edge + (int)oy_edge + (int)oz_edge >= 2) { continue; }
        if (is_local_pos_valid(local_pos_leaves) && chunk->get_cube(local_pos_leaves) != CubeId::AIR) {
          continue;
        }
        if (is_local_pos_valid(local_pos_leaves) && chunk->get_cube(local_pos_leaves) != CubeId::AIR) { continue; }
        chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::LEAVES);
      }
    }
  }

  for (i32 i = 0; i <= tree_height; i += 1) {
    LocalPos local_pos_leaves = {at.x, at.y + i, at.z};
    CubeId trunk = CubeId::WOOD;
    if (birch) { trunk = CubeId::WOOD_BIRCH; }
    chunk->set_cube_maybe_neigbour(local_pos_leaves, trunk);
  }
}

void gen_tree_spruce(Chunk* chunk, LocalPos at, bool birch = false) {
  bool spiky = StaticRandom::get().next<int>(0, 1) == 1;
  i32 tree_height = StaticRandom::get().next<i32>(6, 12);

  i32 prev_radius = 0;
  const i32 unique_radius_tries = 4;
  const i32 leaves_dist_from_ground = StaticRandom::get().next<i32>(1, 2);
  const i32 leaves_height_over_trunk = StaticRandom::get().next<i32>(1, 2);

  const i32 start = leaves_dist_from_ground;
  const i32 end = tree_height + leaves_height_over_trunk;
  for (i32 oy = start; oy <= end; oy += 1) {
    i32 min_radius = 0;
    i32 max_radius = 0;
    if ((oy < start + 2)) {
      min_radius = 2;
      max_radius = 3;
    } else if (oy > tree_height - 2 && oy <= tree_height) {
      min_radius = 1;
      max_radius = 2;
    } else if (oy > tree_height) {
      min_radius = 0;
      max_radius = 0;
    } else {
      min_radius = 1;
      max_radius = 3;
    }
    i32 radius = prev_radius;
    for (i32 i_ = 0; i_ < unique_radius_tries; i_ += 1) {
      radius = StaticRandom::get().next<i32>(min_radius, max_radius);
      if (radius != prev_radius) { break; }
    }

    if (prev_radius != 0 && StaticRandom::get().next<int>(0, 10) == 0) {
      radius = 0;
    }

    for (i32 ox = -radius; ox <= radius; ox += 1) {
      for (i32 oz = -radius; oz <= radius; oz += 1) {
        if (std::abs(ox) + std::abs(oz) > radius) { continue; }
        LocalPos local_pos_leaves = {at.x + ox, at.y + oy, at.z + oz};
        if (!spiky && radius > 1 && (std::abs(ox) == radius || std::abs(oz) == radius)) { continue; }
        if (is_local_pos_valid(local_pos_leaves) && chunk->get_cube(local_pos_leaves) != CubeId::AIR) { continue; }
        chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::LEAVES);
      }
    }

    prev_radius = radius;
  }

  for (i32 i = 0; i <= tree_height; i += 1) {
    LocalPos local_pos_leaves = {at.x, at.y + i, at.z};
    CubeId trunk = CubeId::WOOD;
    if (birch) { trunk = CubeId::WOOD_BIRCH; }
    chunk->set_cube_maybe_neigbour(local_pos_leaves, trunk);
  }
}

void WorldGen::generate_chunk_only_water(Chunk* chunk) const {
  for (size_t y_local = 0; y_local < Chunk::chunk_size; y_local += 1) {
    i64 y = chunk->position.y * Chunk::chunk_size + (i32)y_local;
    if (y > 0) { continue; }
    for (size_t x_local = 0; x_local < Chunk::chunk_size; x_local += 1) {
      for (size_t z_local = 0; z_local < Chunk::chunk_size; z_local += 1) {
        chunk->set_cube({x_local, y_local, z_local}, CubeId::WATER);
      }
    }
  }
}

std::vector<std::unique_ptr<Chunk>> WorldGen::generate_chunk_column(glm::vec<2, i32> chunk_column_pos) const {
  BENCHMARK("chunkgen");

  Array3D<int, Chunk::chunk_size, 1, Chunk::chunk_size> tree_map;

  auto tree_map_get_or_false = [&tree_map](i32 at_x, i32 at_y) -> bool {
    if (!tree_map.has_index({at_x, 0, at_y})) { return false; }
    return tree_map.at({at_x, 0, at_y});
  };

  for (size_t i = 0; i < 128; i += 1) {
    // Starting from (1, 1) to prevent two trees sticking on chunk boundaries
    i32 ox = StaticRandom::get().next<i32>(1, Chunk::chunk_size - 1);
    i32 oy = StaticRandom::get().next<i32>(1, Chunk::chunk_size - 1);

    if (tree_map_get_or_false(ox - 1, oy + 1) || tree_map_get_or_false(ox + 0, oy + 1) || tree_map_get_or_false(ox + 1, oy + 1) ||
        tree_map_get_or_false(ox - 1, oy + 0) /*check 8 neigbours if there is a tree*/ || tree_map_get_or_false(ox + 1, oy + 0) ||
        tree_map_get_or_false(ox - 1, oy - 1) || tree_map_get_or_false(ox + 0, oy - 1) || tree_map_get_or_false(ox + 1, oy - 1)) {
      continue;
    }
    tree_map.set({ox, 0, oy}, true);
  }

  Array3D<biomes::Biome, Chunk::chunk_size, 1, Chunk::chunk_size> blended_biomes;
  for (i32 x = 0; x < Chunk::chunk_size; x += 1) {
    for (i32 z = 0; z < Chunk::chunk_size; z += 1) {
      blended_biomes.set({x, 0, z}, get_blended_biome(local_pos_to_cube_pos({chunk_column_pos.x, 0, chunk_column_pos.y}, {x, 0, z})));
    }
  }

  std::vector<std::unique_ptr<Chunk>> chunks;
  auto chunk_solid_cubes_array = ChunkGenArray(*this);

  for (i32 chunk_y = World::max_chunk_y; chunk_y >= World::min_chunk_y; chunk_y -= 1) {
    ChunkPos chunk_pos = {chunk_column_pos.x, chunk_y, chunk_column_pos.y};
    chunk_solid_cubes_array.initialize(chunk_pos);
    auto chunk = std::make_unique<Chunk>(chunk_pos);

    if (chunk_solid_cubes_array.is_empty()) {
      generate_chunk_only_water(chunk.get());
      chunks.emplace_back(std::move(chunk));
      continue;
    }

    for (size_t x_local = 0; x_local < Chunk::chunk_size; x_local += 1) {
      for (size_t z_local = 0; z_local < Chunk::chunk_size; z_local += 1) {
        auto& blended_biome = blended_biomes.at({x_local, 0, z_local});

        for (size_t y_local = 0; y_local < Chunk::chunk_size; y_local += 1) {
          float y = chunk->position.y * Chunk::chunk_size + (int)y_local;
          LocalPos local_pos = {x_local, y_local, z_local};
          auto is_solid = chunk_solid_cubes_array.is_solid_unsafe(local_pos);

          // -1 = air, 0 = at ground level, lower = under ground
          i32 depth = 0;
          while (chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, depth, 0}).value_or(false)) {
            depth += 1;
          }
          depth -= 1;

          bool just_over_ground = (true == chunk_solid_cubes_array.is_solid_unsafe(local_pos + LocalPos{0, -1, 0})) &&
                                  (false == chunk_solid_cubes_array.is_solid_unsafe(local_pos));

          if (is_solid) {
            CubeId ground_cube = blended_biome.get_ground_cube((i32)y, depth, 0.0);

            if (y <= -1 && !chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, 0}).value_or(true) && ground_cube == CubeId::GRASS) {
              ground_cube = CubeId::DIRT;
            }
            chunk->set_cube(local_pos, ground_cube);
          } else if (y <= 0) {
            chunk->set_cube(local_pos, CubeId::WATER);
          } else if (just_over_ground) {
            bool gen_tree = tree_map_get_or_false(x_local, z_local) && (blended_biome.get_ground_cube((i32)y, 0, 0.0) == CubeId::GRASS) &&
                            blended_biome.tree_density >= StaticRandom::get().next<float>(0.0f, 1.0f);

            // Don't spawn trees on steep terrain
            if (chunk_solid_cubes_array.is_solid(local_pos + LocalPos{-1, 1, 0}).value_or(false) ||
                chunk_solid_cubes_array.is_solid(local_pos + LocalPos{+1, 1, 0}).value_or(false) ||
                chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, -1}).value_or(false) ||
                chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, +1}).value_or(false)) {
              gen_tree = false;
            }

            if (gen_tree) {
              float tree_type_random = StaticRandom::get().next<float>(0.0f, blended_biome.oak_tree_chance + blended_biome.birch_tree_chance + blended_biome.spruce_tree_chance);
              if (tree_type_random <= blended_biome.oak_tree_chance) {
                gen_tree_poplar(chunk.get(), {x_local, y_local, z_local}, false);
              } else if (tree_type_random <= blended_biome.oak_tree_chance + blended_biome.birch_tree_chance) {
                gen_tree_poplar(chunk.get(), {x_local, y_local, z_local}, true);
              } else {
                gen_tree_spruce(chunk.get(), {x_local, y_local, z_local}, false);
              }
            } else { // Foliage gen
              float rng = StaticRandom::get().next<float>(0.0f, 1.0f);
              auto foliage_cube = blended_biome.get_foliage_cube((i32)y, rng);
              chunk->set_cube(local_pos, foliage_cube);
            }
          }
        }
      }
    }

    chunks.emplace_back(std::move(chunk));
  }

  return chunks;
}
