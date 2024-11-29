#include "world_gen.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <optional>
#include <vector>
#include "array3d.hpp"
#include "biome.hpp"
#include "biome_map.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "random.hpp"
#include "world.hpp"

struct ChunkGenArray {
public:
  static constexpr i32 lip_negative_y = 1;
  static constexpr i32 lip_positive_y = 4;
  static constexpr i32 noise_step_size = 5;

  // Used for lerping biome samples across chunk to reduce jagginess
  struct SmoothBiomeGrid {
    // (1 + biome_samples_subdivisions) ^ 2 samples will be used
    static constexpr size_t biome_samples_subdivisions = 2;

    static constexpr size_t biome_sample_step_size = Chunk::chunk_size >> biome_samples_subdivisions;
    static constexpr size_t biome_sample_grid_extents = (Chunk::chunk_size / biome_sample_step_size) + 1;
    static_assert(biome_sample_step_size >= 1);

    SmoothBiomeGrid(const WorldGen& world_gen, CubePos begin) {
      for (i32 z = 0; z < (i32)biome_sample_grid_extents; z += 1) {
        for (i32 x = 0; x < (i32)biome_sample_grid_extents; x += 1) {
          data[x + z * biome_sample_grid_extents] = world_gen.get_blended_biome(begin + CubePos{x * biome_sample_step_size, 0, z * biome_sample_step_size});
        }
      }
    }

    biomes::Biome get_biome(i32 x_local, i32 z_local) {
      float coefficient_x = (float)(x_local % biome_sample_step_size) / (float)(biome_sample_step_size);
      float coefficient_z = (float)(z_local % (biome_sample_step_size)) / (float)(biome_sample_step_size);

      int biome_grid_x = x_local / biome_sample_step_size;
      int biome_grid_z = z_local / biome_sample_step_size;

      size_t i_front_left = biome_grid_x + biome_grid_z * biome_sample_grid_extents;
      size_t i_front_right = biome_grid_x + 1 + biome_grid_z * biome_sample_grid_extents;
      size_t i_back_left = biome_grid_x + (biome_grid_z + 1) * biome_sample_grid_extents;
      size_t i_back_right = biome_grid_x + 1 + (biome_grid_z + 1) * biome_sample_grid_extents;

      biomes::Biome blended_biome_front = biomes::biome_lerp(data[i_front_left], data[i_front_right], coefficient_x);
      biomes::Biome blended_biome_back = biomes::biome_lerp(data[i_back_left], data[i_back_right], coefficient_x);

      return biomes::biome_lerp(blended_biome_front, blended_biome_back, coefficient_z);
    }

    std::array<biomes::Biome, biome_sample_grid_extents * biome_sample_grid_extents> data;
  };

  ChunkGenArray(const WorldGen& wg, ChunkPos chunk_pos) : world_gen(wg) {
    BENCHMARK("chunkgen");

    begin = CubePos{0, -lip_negative_y, 0};
    end = CubePos{Chunk::chunk_size, Chunk::chunk_size, Chunk::chunk_size} + CubePos{0, lip_positive_y, 0};

    ground_array = Array3D<bool>{begin.x, begin.y, begin.z, end.x, end.y, end.z};
    auto noise_heightmap_array = Array3D<float>{begin.x, 0, begin.z, end.x, 1, end.z};
    auto noise_3d_array = Array3D<float>{begin.x, begin.y, begin.z, end.x, end.y, end.z};
    biome_array = Array3D<biomes::Biome>{begin.x, 0, begin.z, end.x, 1, end.z};
    SmoothBiomeGrid biome_grid(wg, begin + chunk_pos * Chunk::chunk_size);

    for (i32 z = begin.z; z < end.z; z += 1) {
      for (i32 x = begin.x; x < end.x; x += 1) {
        auto blended_biome = biome_grid.get_biome(x, z);
        biome_array.set({x, 0, z}, blended_biome);
        float noise_heightmap = wg.get_heightmap_noise(chunk_pos * Chunk::chunk_size + LocalPos{x, 0, z}, blended_biome);
        noise_heightmap_array.set({x, 0, z}, noise_heightmap);
      }
    }

    for (i32 x = begin.x; x < end.x; x += 1) {
      if (!(x == begin.x || x == end.x - 1 || wrapi(x, 0, noise_step_size) == 0)) { continue; }
      for (i32 y = begin.y; y < end.y; y += 1) {
        if (!(y == begin.y || y == end.y - 1 || wrapi(y, 0, noise_step_size) == 0)) { continue; }
        for (i32 z = begin.z; z < end.z; z += 1) {
          if (!(z == begin.z || z == end.z - 1 || wrapi(z, 0, noise_step_size) == 0)) { continue; }

          CubePos cube_pos = chunk_pos * Chunk::chunk_size + LocalPos{x, y, z};

          float noise_3d = wg.get_3d_noise(chunk_pos * Chunk::chunk_size + LocalPos{x, y, z}, biome_array.at({x, 0, z}));
          noise_3d_array.set({x, y, z}, noise_3d);

          bool is_solid = world_gen.is_ground(cube_pos, biome_array.at({x, 0, z}), noise_heightmap_array.at({x, 0, z}), noise_3d);
          if (is_solid) { empty = false; }
          ground_array.set({x, y, z}, is_solid);
        }
      }
    }

    if (empty) { return; }
    // return;

    // Fix the skipped cubes in checkerboard generation
    for (i32 x = begin.x; x < end.x; x += 1) {
      for (i32 y = begin.y; y < end.y; y += 1) {
        for (i32 z = begin.z; z < end.z; z += 1) {
          i32 x_wrapped = wrapi(x, 0, noise_step_size);
          i32 y_wrapped = wrapi(y, 0, noise_step_size);
          i32 z_wrapped = wrapi(z, 0, noise_step_size);

          bool x_has_value = x == begin.x || x == end.x - 1 || x_wrapped == 0;
          bool y_has_value = y == begin.y || y == end.y - 1 || y_wrapped == 0;
          bool z_has_value = z == begin.z || z == end.z - 1 || z_wrapped == 0;
          if (x_has_value && y_has_value && z_has_value) { continue; }

          i32 x_low = std::max(x - x_wrapped, begin.x);
          i32 x_high = std::min(x + noise_step_size - x_wrapped, end.x - 1);
          i32 y_low = std::max(y - y_wrapped, begin.y);
          i32 y_high = std::min(y + noise_step_size - y_wrapped, end.y - 1);
          i32 z_low = std::max(z - z_wrapped, begin.z);
          i32 z_high = std::min(z + noise_step_size - z_wrapped, end.z - 1);

          float noise_3d_x_low_z_low = std::lerp(noise_3d_array.at({x_low, y_low, z_low}), noise_3d_array.at({x_low, y_high, z_low}), y_wrapped / (float)(noise_step_size - 1));
          float noise_3d_x_low_z_high = std::lerp(noise_3d_array.at({x_low, y_low, z_high}), noise_3d_array.at({x_low, y_high, z_high}), y_wrapped / (float)(noise_step_size - 1));
          float noise_3d_x_high_z_low = std::lerp(noise_3d_array.at({x_high, y_low, z_low}), noise_3d_array.at({x_high, y_high, z_low}), y_wrapped / (float)(noise_step_size - 1));
          float noise_3d_x_high_z_high = std::lerp(noise_3d_array.at({x_high, y_low, z_high}), noise_3d_array.at({x_high, y_high, z_high}), y_wrapped / (float)(noise_step_size - 1));

          float noise_3d_x_low = std::lerp(noise_3d_x_low_z_low, noise_3d_x_low_z_high, z_wrapped / (float)(noise_step_size - 1));
          float noise_3d_x_high = std::lerp(noise_3d_x_high_z_low, noise_3d_x_high_z_high, z_wrapped / (float)(noise_step_size - 1));

          float noise_3d_value = std::lerp(noise_3d_x_low, noise_3d_x_high, x_wrapped / (float)(noise_step_size - 1));

          float noise_heightmap_x_low = std::lerp(noise_heightmap_array.at({x_low, 0, z_low}), noise_heightmap_array.at({x_low, 0, z_high}), z_wrapped / (float)(noise_step_size - 1));
          float noise_heightmap_x_high = std::lerp(noise_heightmap_array.at({x_high, 0, z_low}), noise_heightmap_array.at({x_high, 0, z_high}), z_wrapped / (float)(noise_step_size - 1));

          float noise_heightmap_value = std::lerp(noise_heightmap_x_low, noise_heightmap_x_high, x_wrapped / (float)(noise_step_size - 1));

          CubePos cube_pos = chunk_pos * Chunk::chunk_size + LocalPos{x, y, z};
          ground_array.set({x, y, z}, wg.is_ground(cube_pos, biome_array.at({x, 0, z}), noise_heightmap_value, noise_3d_value));
        }
      }
    }
  }

  bool is_solid_unsafe(LocalPos local_pos) {
    return ground_array.at(local_pos);
  }

  std::optional<bool> is_solid(LocalPos local_pos) {
    if (!ground_array.has_index(local_pos)) { return std::nullopt; }
    return ground_array.at(local_pos);
  }

  bool is_empty() const { return empty; }

private:
  CubePos begin;
  CubePos end;
  const WorldGen& world_gen;
  bool empty = true;

public:
  Array3D<bool> ground_array;
  Array3D<biomes::Biome> biome_array;
};

WorldGen::WorldGen(World& w, i32 seed) : world(w) {
  constexpr float scale = 0.82;
  noise_heightmap.SetSeed(seed);
  noise_heightmap.SetFrequency(0.006064f / scale);
  noise_heightmap.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap.SetFractalOctaves(3);
  noise_heightmap.SetFractalGain(0.4f);
  noise_heightmap.SetFractalLacunarity(2.57f);

  noise_3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_3d.SetFrequency(0.00351f / scale);
  noise_3d.SetSeed(seed);
  noise_3d.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_3d.SetFractalOctaves(3);
  // noise_3d.SetFractalGain(0.6785f);
  // noise_3d.SetFractalLacunarity(2.481f);
  noise_3d.SetDomainWarpType(FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid);
  noise_3d.SetDomainWarpAmp(80.0f);

  noise_humidity.SetSeed(seed + 1);
  noise_humidity.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_humidity.SetFrequency(0.00224f / scale);
  noise_humidity.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_humidity.SetFractalOctaves(4);
  noise_humidity.SetFractalLacunarity(2.2f);
  noise_humidity.SetFractalGain(0.4f);

  noise_temperature.SetSeed(seed + 2);
  noise_temperature.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_temperature.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_temperature.SetFrequency(0.00224f / scale);
  noise_temperature.SetFractalOctaves(4);
  noise_temperature.SetFractalLacunarity(2.2f);
  noise_temperature.SetFractalGain(0.4f);
}

biomes::Biome WorldGen::get_blended_biome(WorldPos world_pos) const {
  float humidity = noise_humidity.GetNoise(world_pos.x, world_pos.z) * 0.5f + 0.5f;
  humidity = std::clamp(humidity, 0.0f, 1.0f);

  float temperature = noise_temperature.GetNoise(world_pos.x, world_pos.z) * 0.5f + 0.5f;
  temperature = std::clamp(temperature, 0.0f, 1.0f);

  return biomemap::get_biome(humidity, temperature);
}

float WorldGen::get_heightmap_noise(WorldPos pos, const biomes::Biome& blended_biome) const {
  return (noise_heightmap.GetNoise(pos.x, pos.z) + 1.0f) * 0.5f * blended_biome.noise_height_multiplier;
}

float WorldGen::get_3d_noise(WorldPos pos, const biomes::Biome& blended_biome) const {
  return noise_3d.GetNoise(pos.x, pos.y * 2.0f, pos.z) * blended_biome.noise_3d_multiplier;
}

bool WorldGen::is_ground(WorldPos pos, const biomes::Biome& blended_biome,
                         std::optional<float> noise_heightmap_value_opt, std::optional<float> noise_3d_value_opt) const {

  float noise_3d_value = 0.0f;
  if (blended_biome.noise_3d_multiplier > 0.01f) {
    if (noise_3d_value_opt.has_value()) {
      noise_3d_value = noise_3d_value_opt.value();
    } else {
      noise_3d_value = get_3d_noise(pos, blended_biome);
    }
  }

  float noise_heightmap_value = 0.0f;
  if (noise_heightmap_value_opt.has_value()) {
    noise_heightmap_value = noise_heightmap_value_opt.value();
  } else {
    noise_heightmap_value = get_heightmap_noise(pos, blended_biome);
  }

  noise_3d_value = 1.0f + noise_3d_value * 0.032f;
  float value = (blended_biome.base_height + noise_heightmap_value) * noise_3d_value;

  return value > pos.y;
}

bool WorldGen::is_ground(WorldPos pos, const biomes::Biome& blended_biome) const {
  return is_ground(pos, blended_biome, get_heightmap_noise(pos, blended_biome));
}

bool WorldGen::is_ground(WorldPos pos) const {
  auto blended_biome = get_blended_biome(pos);
  return is_ground(pos, blended_biome, get_heightmap_noise(pos, blended_biome));
}

void gen_tree_poplar(Chunk* chunk, LocalPos at) {
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
    chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::WOOD);
  }
}

void gen_tree_spruce(Chunk* chunk, LocalPos at) {
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
    chunk->set_cube_maybe_neigbour({at.x, at.y + i, at.z}, CubeId::WOOD);
  }
}

void gen_tree_pine(Chunk* chunk, LocalPos at) {
  i32 tree_height = StaticRandom::get().next<i32>(7, 10);

  auto gen_branch = [&](LocalPos center) {
    for (i32 ox = center.x - 1; ox <= center.x + 1; ox += 1) {
      for (i32 oy = center.y - 1; oy <= center.y + 1; oy += 1) {
        for (i32 oz = center.z - 1; oz <= center.z + 1; oz += 1) {
          bool x_edge = std::abs(ox - center.x) == 1;
          bool y_edge = std::abs(oy - center.y) == 1;
          bool z_edge = std::abs(oz - center.z) == 1;
          if ((i32)x_edge + (i32)y_edge + (i32)z_edge >= 3) { continue; }
          LocalPos local_pos_leaves = {at.x + ox, at.y + oy, at.z + oz};
          if (is_local_pos_valid(local_pos_leaves) && chunk->get_cube(local_pos_leaves) != CubeId::AIR) { continue; }
          chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::LEAVES);
        }
      }
    }
  };

  // Top crown
  for (i32 i = 0; i < 10; i += 1) {
    LocalPos center = {
        StaticRandom::get().next<i32>(-2, 2),
        StaticRandom::get().next<i32>(tree_height, tree_height),
        StaticRandom::get().next<i32>(-2, 2),
    };

    gen_branch(center);
  }

  // Lower crown
  i32 crown_detail_count = StaticRandom::get().next<i32>(1, 3);
  for (i32 i = 0; i < crown_detail_count; i += 1) {

    LocalPos center = {
        StaticRandom::get().next<i32>(-1, 1),
        StaticRandom::get().next<i32>(tree_height - 2, tree_height - 1),
        StaticRandom::get().next<i32>(-1, 1),
    };

    gen_branch(center);
  }

  for (i32 i = 0; i < crown_detail_count; i += 1) {

    LocalPos center = {
        StaticRandom::get().next<i32>(-2, 2),
        tree_height + 1,
        StaticRandom::get().next<i32>(-2, 2),
    };

    gen_branch(center);
  }

  // Branches
  i32 branch_count = StaticRandom::get().next<i32>(0, 2);
  for (i32 i = 0; i < branch_count; i += 1) {

    LocalPos center = {
        StaticRandom::get().rand_sign<i32>(),
        StaticRandom::get().next<i32>(3, tree_height - 2),
        StaticRandom::get().rand_sign<i32>(),
    };

    gen_branch(center);
  }

  for (i32 i = 0; i <= tree_height; i += 1) {
    LocalPos local_pos_leaves = {at.x, at.y + i, at.z};
    chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::WOOD);
  }
}

void WorldGen::generate_chunk_only_water(Chunk* chunk) const {
  for (size_t x_local = 0; x_local < Chunk::chunk_size; x_local += 1) {
    for (size_t z_local = 0; z_local < Chunk::chunk_size; z_local += 1) {
      for (size_t y_local = 0; y_local < Chunk::chunk_size; y_local += 1) {
        i64 y = chunk->position.y * Chunk::chunk_size + (int)y_local;
        if (y <= 0) {
          chunk->set_cube({x_local, y_local, z_local}, CubeId::WATER);
        }
      }
    }
  }
}

void WorldGen::generate_chunk(Chunk* chunk) const {
  auto chunk_solid_cubes_array = ChunkGenArray(*this, chunk->position);
  if (chunk_solid_cubes_array.is_empty()) {
    generate_chunk_only_water(chunk);
    return;
  }

  Array3D<bool> tree_map(Chunk::chunk_size, 1, Chunk::chunk_size);

  auto tree_map_get_or_false = [&tree_map](i32 at_x, i32 at_y) -> bool {
    if (!tree_map.has_index({at_x, 0, at_y})) { return false; }
    return tree_map.at({at_x, 0, at_y});
  };

  for (size_t i = 0; i < 8; i += 1) {
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

  for (size_t x_local = 0; x_local < Chunk::chunk_size; x_local += 1) {
    for (size_t z_local = 0; z_local < Chunk::chunk_size; z_local += 1) {
      // auto blended_biome = chunk_solid_cubes_array.biome_array.at({x_local, 0, z_local});
      auto blended_biome = get_blended_biome(local_pos_to_cube_pos(chunk->position, {x_local, 0, z_local}));

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

        bool just_over_ground = (true == chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, -1, 0}).value_or(false)) &&
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
          bool gen_tree = tree_map_get_or_false(x_local, z_local) && (blended_biome.get_ground_cube((i32)y, 0, 0.0) == CubeId::GRASS);

          // Don't spawn trees on steep terrain
          if (chunk_solid_cubes_array.is_solid(local_pos + LocalPos{-1, 1, 0}).value_or(false) ||
              chunk_solid_cubes_array.is_solid(local_pos + LocalPos{+1, 1, 0}).value_or(false) ||
              chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, -1}).value_or(false) ||
              chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, +1}).value_or(false)) {
            gen_tree = false;
          }

          if (gen_tree) { // Tree gen
            i32 tree_type = StaticRandom::get().next<i32>(0, 2);
            if (tree_type == 0) {
              gen_tree_poplar(chunk, {x_local, y_local, z_local});
            } else if (tree_type == 1) {
              gen_tree_spruce(chunk, {x_local, y_local, z_local});
            } else if (tree_type == 2) {
              gen_tree_pine(chunk, {x_local, y_local, z_local});
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
}
