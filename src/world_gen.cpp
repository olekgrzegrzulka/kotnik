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

    data = Array3D<bool>{begin.x, begin.y, begin.z, end.x, end.y, end.z};

    SmoothBiomeGrid biome_grid(wg, begin + chunk_pos * Chunk::chunk_size);

    for (i32 z = begin.z; z < end.z; z += 1) {
      for (i32 x = begin.x; x < end.x; x += 1) {
        auto blended_biome = biome_grid.get_biome(x, z);
        for (i32 y = begin.y; y < end.y; y += 1) {
          // Checkerboard
          if ((x + y + z) % 2 == 1) { continue; }

          WorldPos world_pos = chunk_pos * Chunk::chunk_size + LocalPos{x, y, z};

          bool is_solid = world_gen.is_ground(world_pos, blended_biome);
          if (is_solid) { empty = false; }
          data.set({x, y, z}, is_solid);
        }
      }
    }

    if (empty) { return; }

    // Fix the skipped cubes in checkerboard generation
    auto data_uncheckered = data;

    for (i32 z = begin.z; z < end.z; z += 1) {
      for (i32 y = begin.y; y < end.y; y += 1) {
        for (i32 x = begin.x; x < end.x; x += 1) {
          // Alternate checkerboard
          if ((x + y + z) % 2 == 0) { continue; }

          i32 neigbour_count_any = 0;
          i32 neigbour_count_solid = 0;

          static constexpr std::array<LocalPos, 6> offsets = {LocalPos{-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}};
          for (LocalPos o : offsets) {
            auto c = is_solid(LocalPos{x, y, z} + LocalPos{o.x, o.y, o.z});
            if (!c.has_value()) { continue; }
            neigbour_count_any += 1;
            neigbour_count_solid += (int)c.value();
          }

          float occlusion = (float)neigbour_count_solid / (float)neigbour_count_any;

          if (occlusion >= 0.5f) {
            data_uncheckered.set({x, y, z}, true);
          }
        }

        data = data_uncheckered;
      }
    }
  }

  bool is_solid_unsafe(LocalPos local_pos) {
    return data.at(local_pos);
  }

  std::optional<bool> is_solid(LocalPos local_pos) {
    if (!data.has_index(local_pos)) { return std::nullopt; }
    return data.at(local_pos);
  }

  bool is_empty() const { return empty; }

private:
  CubePos begin;
  CubePos end;
  const WorldGen& world_gen;
  bool empty = true;

  Array3D<bool> data;
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
  noise_3d.SetFractalGain(0.6785f);
  noise_3d.SetFractalLacunarity(2.481f);
  noise_3d.SetDomainWarpType(FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid);
  noise_3d.SetDomainWarpAmp(80.0f);

  noise_humidity.SetSeed(seed + 1);
  noise_humidity.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_humidity.SetFrequency(0.00244f / scale);
  noise_humidity.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_humidity.SetFractalOctaves(4);
  noise_humidity.SetFractalLacunarity(2.2f);
  noise_humidity.SetFractalGain(0.4f);

  noise_temperature.SetSeed(seed + 2);
  noise_temperature.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_temperature.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_temperature.SetFrequency(0.00244f / scale);
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

bool WorldGen::is_ground(WorldPos pos, const biomes::Biome& blended_biome) const {
  float value_height = (noise_heightmap.GetNoise(pos.x, pos.z) + 1.0f) * 0.5f * blended_biome.noise_height_multiplier;

  float value_3d = 0.0f;
  if (blended_biome.noise_3d_multiplier > 0.01f) {
    value_3d = noise_3d.GetNoise(pos.x, pos.y * 1.0f, pos.z) * blended_biome.noise_3d_multiplier;
  }

  value_3d = 1.0f + value_3d * 0.032f;
  float value = (blended_biome.base_height + value_height) * value_3d;

  return value > pos.y;
}

bool WorldGen::is_ground(WorldPos pos) const {
  auto blended_biome = get_blended_biome(pos);

  return is_ground(pos, blended_biome);
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
        chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::LEAVES);
      }
    }
  }

  for (i32 i = 0; i <= tree_height; i += 1) {
    chunk->set_cube_maybe_neigbour({at.x, at.y + i, at.z}, CubeId::WOOD);
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
        if (!spiky && radius > 1 && (std::abs(ox) == radius || std::abs(oz) == radius)) { continue; }

        chunk->set_cube_maybe_neigbour({at.x + ox, at.y + oy, at.z + oz}, CubeId::LEAVES);
      }
    }

    prev_radius = radius;
  }

  for (i32 i = 0; i <= tree_height; i += 1) {
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
          chunk->set_cube_maybe_neigbour({at.x + ox, at.y + oy, at.z + oz}, CubeId::LEAVES);
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
    chunk->set_cube_maybe_neigbour({at.x, at.y + i, at.z}, CubeId::WOOD);
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

  std::vector<bool> tree_map{};
  tree_map.resize(Chunk::chunk_size * Chunk::chunk_size, false);

  auto tree_map_get_or_false = [&tree_map](i32 at_x, i32 at_y) -> bool {
    if (at_x < 0 || at_y < 0 || at_x >= Chunk::chunk_size || at_y >= Chunk::chunk_size) {
      return false;
    }
    return tree_map[at_x + at_y * Chunk::chunk_size];
  };

  for (size_t i = 0; i < 8; i++) {
    // Starting from (1, 1) to prevent two trees sticking on chunk boundaries
    i32 ox = StaticRandom::get().next<i32>(1, Chunk::chunk_size - 1);
    i32 oy = StaticRandom::get().next<i32>(1, Chunk::chunk_size - 1);

    if (tree_map_get_or_false(ox - 1, oy + 1) || tree_map_get_or_false(ox + 0, oy + 1) || tree_map_get_or_false(ox + 1, oy + 1) ||
        tree_map_get_or_false(ox - 1, oy + 0) /*check 8 neigbours if there is a tree*/ || tree_map_get_or_false(ox + 1, oy + 0) ||
        tree_map_get_or_false(ox - 1, oy - 1) || tree_map_get_or_false(ox + 0, oy - 1) || tree_map_get_or_false(ox + 1, oy - 1)) {
      continue;
    }
    tree_map[ox + oy * Chunk::chunk_size] = true;
  }

  for (size_t x_local = 0; x_local < Chunk::chunk_size; x_local += 1) {
    for (size_t z_local = 0; z_local < Chunk::chunk_size; z_local += 1) {
      float x = chunk->position.x * Chunk::chunk_size + (int)x_local;
      float z = chunk->position.z * Chunk::chunk_size + (int)z_local;
      auto blended_biome = get_blended_biome({x, 0, z});

      for (size_t y_local = 0; y_local < Chunk::chunk_size; y_local += 1) {
        float y = chunk->position.y * Chunk::chunk_size + (int)y_local;
        LocalPos local_pos = {x_local, y_local, z_local};

        auto is_solid = chunk_solid_cubes_array.is_solid_unsafe({x_local, y_local, z_local});

        if (!is_solid) {
          if (y <= 0) {
            chunk->set_cube({x_local, y_local, z_local}, CubeId::WATER);
            continue;
          }

          if (chunk_solid_cubes_array.is_solid_unsafe({x_local, y_local - 1, z_local})) {
            float rng = StaticRandom::get().next<float>(0.0f, 1.0f);

            auto cube = blended_biome.get_foliage_cube((i32)y, rng);
            if (chunk->get_cube(local_pos) == CubeId::AIR) {
              chunk->set_cube(local_pos, cube);
            }

            bool gen_tree = tree_map_get_or_false(x_local, z_local) && (blended_biome.get_ground_cube((i32)y, 0, 0.0) == CubeId::GRASS);

            // Don't spawn trees on steep terrain
            if (chunk_solid_cubes_array.is_solid(local_pos + LocalPos{-1, 1, 0}).value_or(false) ||
                chunk_solid_cubes_array.is_solid(local_pos + LocalPos{+1, 1, 0}).value_or(false) ||
                chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, -1}).value_or(false) ||
                chunk_solid_cubes_array.is_solid(local_pos + LocalPos{0, 1, +1}).value_or(false)) {
              gen_tree = false;
            }

            if (gen_tree) {
              i32 tree_type = StaticRandom::get().next<i32>(0, 2);
              if (tree_type == 0) {
                gen_tree_poplar(chunk, {x_local, y_local, z_local});
              } else if (tree_type == 1) {
                gen_tree_spruce(chunk, {x_local, y_local, z_local});
              } else if (tree_type == 2) {
                gen_tree_pine(chunk, {x_local, y_local, z_local});
              }
            }
          }
          continue;
        }

        int depth = 0;
        for (; depth < 4; depth += 1) {
          if (!chunk_solid_cubes_array.is_solid_unsafe({x_local, y_local + depth + 1, z_local})) {
            break;
          }
        }

        auto cube = blended_biome.get_ground_cube((i32)y, depth, 0.0);
        chunk->set_cube({x_local, y_local, z_local}, cube);
      }
    }
  }
}
