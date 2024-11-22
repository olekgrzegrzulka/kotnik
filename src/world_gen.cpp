#include "world_gen.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <optional>
#include <unordered_map>
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

    static constexpr size_t biome_sample_step_size = CHUNK_SIZE >> biome_samples_subdivisions;
    static constexpr size_t biome_sample_grid_extents = (CHUNK_SIZE / biome_sample_step_size) + 1;
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
    begin = CubePos{0, -lip_negative_y, 0};
    end = CubePos{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE} + CubePos{0, lip_positive_y, 0};

    data = Array3D<bool>{begin.x, begin.y, begin.z, end.x, end.y, end.z};

    SmoothBiomeGrid biome_grid(wg, begin + chunk_pos * CHUNK_SIZE);

    std::unordered_map<glm::vec<2, i32>, biomes::Biome, Vec2Hasher> blended_biome_cache;

    for (i32 z = begin.z; z < end.z; z += 1) {
      for (i32 y = begin.y; y < end.y; y += 1) {
        for (i32 x = begin.x; x < end.x; x += 1) {
          // Checkerboard
          if ((x + y + z) % 2 == 1) { continue; }

          if (!blended_biome_cache.contains({x, z})) {
            blended_biome_cache[{x, z}] = biome_grid.get_biome(x, z);
          }
          auto blended_biome = blended_biome_cache[{x, z}];

          WorldPos world_pos = chunk_pos * CHUNK_SIZE + LocalPos{x, y, z};

          bool is_solid = world_gen.is_ground(world_pos, blended_biome);
          if (is_solid) { empty = false; }
          data.set({x, y, z}, is_solid);
        }
      }
    }

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
  bool empty = false;

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

  float value_3d = noise_3d.GetNoise(pos.x, pos.y * 1.0f, pos.z) * blended_biome.noise_3d_multiplier;
  value_3d = 1.0f + value_3d * 0.032f;
  float value = (blended_biome.base_height + value_height) * value_3d;

  return value > pos.y;
}

bool WorldGen::is_ground(WorldPos pos) const {
  auto blended_biome = get_blended_biome(pos);

  return is_ground(pos, blended_biome);
}

void WorldGen::generate_chunk(Chunk* chunk) const {
  auto chunk_solid_cubes_array = ChunkGenArray(*this, chunk->position);
  if (chunk_solid_cubes_array.is_empty()) { return; }

  std::vector<bool> tree_map{};
  tree_map.resize(CHUNK_SIZE * CHUNK_SIZE, false);

  auto tree_map_get_or_false = [&tree_map](i32 at_x, i32 at_y) -> bool {
    if (at_x < 0 || at_y < 0 || at_x >= CHUNK_SIZE || at_y >= CHUNK_SIZE) {
      return false;
    }
    return tree_map[at_x + at_y * CHUNK_SIZE];
  };

  for (size_t i = 0; i < 10; i++) {
    // Starting from (1, 1) to prevent two trees sticking on chunk boundaries
    i32 ox = StaticRandom::get().next<i32>(1, CHUNK_SIZE - 1);
    i32 oy = StaticRandom::get().next<i32>(1, CHUNK_SIZE - 1);

    if (tree_map_get_or_false(ox - 1, oy + 1) || tree_map_get_or_false(ox + 0, oy + 1) || tree_map_get_or_false(ox + 1, oy + 1) ||
        tree_map_get_or_false(ox - 1, oy + 0) /*check 9 neigbours if there is a tree*/ || tree_map_get_or_false(ox + 1, oy + 0) ||
        tree_map_get_or_false(ox - 1, oy - 1) || tree_map_get_or_false(ox + 0, oy - 1) || tree_map_get_or_false(ox + 1, oy - 1)) {
      continue;
    }
    tree_map[ox + oy * CHUNK_SIZE] = true;
  }

  for (size_t x_local = 0; x_local < CHUNK_SIZE; x_local += 1) {
    for (size_t z_local = 0; z_local < CHUNK_SIZE; z_local += 1) {
      float x = chunk->position.x * CHUNK_SIZE + (int)x_local;
      float z = chunk->position.z * CHUNK_SIZE + (int)z_local;
      auto blended_biome = get_blended_biome({x, 0, z});

      for (size_t y_local = 0; y_local < CHUNK_SIZE; y_local += 1) {
        float y = chunk->position.y * CHUNK_SIZE + (int)y_local;
        LocalPos local_pos = {x_local, y_local, z_local};

        auto is_solid = chunk_solid_cubes_array.is_solid_unsafe({x_local, y_local, z_local});

        if (!is_solid) {
          if (y <= 0) {
            chunk->set_cube_no_lock({x_local, y_local, z_local}, CubeId::WATER);
            continue;
          }
          float rng = StaticRandom::get().next<float>(0.0f, 1.0f);
          if (chunk_solid_cubes_array.is_solid_unsafe({x_local, y_local - 1, z_local})) {
            auto cube = blended_biome.get_foliage_cube((i32)y, rng);
            chunk->set_cube_no_lock({x_local, y_local, z_local}, cube);

            bool is_tree = tree_map_get_or_false(x_local, z_local) && (blended_biome.get_ground_cube((i32)y, 0, 0.0) == CubeId::GRASS);

            if (is_tree) {
              i32 tree_height = StaticRandom::get().next<i32>(4, 7);

              for (i32 ox = -2; ox <= 2; ox += 1) {
                for (i32 oy = tree_height - 2; oy <= tree_height + 1; oy += 1) {
                  for (i32 oz = -2; oz <= 2; oz += 1) {
                    bool ox_edge = ox == -2 || ox == 2;
                    bool oy_edge = oy == tree_height - 2 || oy == tree_height + 1;
                    bool oz_edge = oz == -2 || oz == 2;
                    LocalPos local_pos_leaves = {x_local + ox, y_local + oy, z_local + oz};
                    if ((int)ox_edge + (int)oy_edge + (int)oz_edge >= 2) { continue; }
                    if (is_local_pos_valid(local_pos_leaves) && chunk->get_cube(local_pos_leaves) != CubeId::AIR) {
                      continue;
                    }
                    chunk->set_cube_maybe_neigbour(local_pos_leaves, CubeId::LEAVES);
                  }
                }
              }

              for (i32 i = 0; i <= tree_height; i += 1) {
                chunk->set_cube_maybe_neigbour({x_local, y_local + i, z_local}, CubeId::WOOD);
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
        chunk->set_cube_no_lock({x_local, y_local, z_local}, cube);
      }
    }
  }
}
