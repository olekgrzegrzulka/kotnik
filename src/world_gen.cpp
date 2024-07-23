#include "world_gen.hpp"
#include <algorithm>
#include <cstdlib>
#include "biome.hpp"
#include "biome_map.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "world.hpp"

using Biomes::BlendedBiome;

struct ChunkGenArray {
public:
  static constexpr u32 lip_negative_x = 0;
  static constexpr u32 lip_positive_x = 0;

  static constexpr u32 lip_negative_y = 1;
  static constexpr u32 lip_positive_y = 4;

  static constexpr u32 lip_negative_z = 0;
  static constexpr u32 lip_positive_z = 0;

  struct CubeData {
    bool is_solid;
    float rng;
  };

  ChunkGenArray(const WorldGen& wg, ChunkPos chunk_pos) : world_gen(wg) {
    begin = chunk_pos * CHUNK_SIZE - CubePos{lip_negative_x, lip_negative_y, lip_negative_z};
    end = chunk_pos * CHUNK_SIZE + CubePos{CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1} + CubePos{lip_positive_x, lip_positive_y, lip_positive_z};

    static constexpr size_t array_size = ((CHUNK_SIZE + lip_negative_x + lip_positive_x) *
                                          (CHUNK_SIZE + lip_negative_y + lip_positive_y) *
                                          (CHUNK_SIZE + lip_negative_z + lip_positive_z));

    data.resize(array_size);

    for (i32 x_local = -lip_negative_x; x_local < (i32)(CHUNK_SIZE + lip_positive_x); x_local += 1) {
      for (i32 z_local = -lip_negative_z; z_local < (i32)(CHUNK_SIZE + lip_positive_z); z_local += 1) {
        LocalPos local_pos = {x_local, 0, z_local};
        WorldPos world_pos = begin + local_pos;

        auto blended_biome = world_gen.get_blended_biome(world_pos);

        for (i32 y_local = -lip_negative_y; y_local < (i32)(CHUNK_SIZE + lip_positive_y); y_local += 1) {
          local_pos.y = y_local;
          world_pos.y = begin.y + local_pos.y;
          size_t i = get_index(local_pos);
          assert(local_pos == index_to_local_pos(i));
          data[i] = CubeData{
              .is_solid = world_gen.is_ground(world_pos, blended_biome),
              .rng = world_gen.get_cube_rng(world_pos),
          };
        }
      }
    }
  }

  CubeData get_cube_info(LocalPos local_pos) {
    size_t index = get_index(local_pos);
    assert(index < data.size());
    return data.at(index);
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

    local_pos.y = (index / ((CHUNK_SIZE + lip_negative_x + lip_positive_x))) % (CHUNK_SIZE + lip_negative_y + lip_positive_y);
    local_pos.y -= lip_negative_y;

    local_pos.x = (index) % (CHUNK_SIZE + lip_negative_x + lip_positive_x);
    local_pos.x -= lip_negative_x;

    return local_pos;
  }

private:
  CubePos begin;
  CubePos end;
  const WorldGen& world_gen;

  std::vector<CubeData> data;
};

WorldGen::WorldGen(World& w, i32 seed) : world(w) {
  noise_heightmap.SetSeed(seed);
  noise_heightmap.SetFrequency(0.006064f);
  noise_heightmap.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap.SetFractalOctaves(3);
  noise_heightmap.SetFractalGain(0.4f);
  noise_heightmap.SetFractalLacunarity(2.57f);

  noise_3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_3d.SetFrequency(0.00311f);
  noise_3d.SetSeed(seed);
  noise_3d.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_3d.SetFractalOctaves(3);
  noise_3d.SetFractalGain(0.6785f);
  noise_3d.SetFractalLacunarity(2.481f);
  noise_3d.SetDomainWarpType(FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid);
  noise_3d.SetDomainWarpAmp(80.0f);

  noise_humidity.SetSeed(seed + 1);
  noise_humidity.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_humidity.SetFrequency(0.00217f);
  noise_humidity.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_humidity.SetFractalOctaves(3);
  noise_humidity.SetFractalLacunarity(2.65f);
  noise_humidity.SetFractalGain(0.418f);

  noise_temperature.SetSeed(seed + 2);
  noise_temperature.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_temperature.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_temperature.SetFrequency(0.00217f);
  noise_temperature.SetFractalOctaves(3);
  noise_temperature.SetFractalLacunarity(3.15f);
  noise_temperature.SetFractalGain(0.418f);

  noise_rng.SetSeed(seed + 3);
  noise_rng.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_rng.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_rng.SetFrequency(0.54117f);
  noise_rng.SetFractalOctaves(3);
}

BlendedBiome WorldGen::get_blended_biome(WorldPos world_pos) const {
  float humidity = noise_humidity.GetNoise(world_pos.x, world_pos.z) * 0.5f + 0.5f;
  humidity = std::clamp(humidity, 0.0f, 1.0f);

  float temperature = noise_temperature.GetNoise(world_pos.x, world_pos.z) * 0.5f + 0.5f;
  temperature = std::clamp(temperature, 0.0f, 1.0f);

  auto blended_biome = biome_map.get_biome(humidity, temperature);
  return blended_biome;
}

bool WorldGen::is_ground(WorldPos pos, const BlendedBiome& blended_biome) const {

  float value_height = (noise_heightmap.GetNoise(pos.x, pos.z) + 1.0f) * 0.5f * blended_biome.get_noise_height_multiplier();

  float value_3d = (noise_3d.GetNoise(pos.x, pos.y * 2.5f, pos.z) + 1.0f) * 0.5f * blended_biome.get_noise_3d_multiplier();
  value_3d = 1.0f + value_3d * 0.032f;
  float value = (blended_biome.get_base_height() + value_height) * value_3d;

  return value > pos.y;
}

float WorldGen::get_cube_rng(WorldPos pos) const {
  return noise_rng.GetNoise(pos.x, pos.y, pos.z) * 0.5f + 0.5f;
}

bool WorldGen::is_ground(WorldPos pos) const {
  auto blended_biome = get_blended_biome(pos);

  return is_ground(pos, blended_biome);
}

void WorldGen::generate_chunk(Chunk* chunk) const {
  auto chunk_solid_cubes_array = ChunkGenArray(*this, chunk->position);

  for (size_t x_local = 0; x_local < CHUNK_SIZE; x_local += 1) {
    for (size_t z_local = 0; z_local < CHUNK_SIZE; z_local += 1) {
      float x = chunk->position.x * CHUNK_SIZE + (int)x_local;
      float z = chunk->position.z * CHUNK_SIZE + (int)z_local;
      auto blended_biome = get_blended_biome({x, 0, z});

      for (size_t y_local = 0; y_local < CHUNK_SIZE; y_local += 1) {
        float y = chunk->position.y * CHUNK_SIZE + (int)y_local;

        auto cube_info = chunk_solid_cubes_array.get_cube_info({x_local, y_local, z_local});
        bool solid = cube_info.is_solid;

        if (!solid) {
          if (y <= 0) {
            chunk->set_cube_no_lock({x_local, y_local, z_local}, CubeId::WATER);
            continue;
          }
          if (chunk_solid_cubes_array.get_cube_info({x_local, y_local - 1, z_local}).is_solid) {
            auto cube = blended_biome.get_foliage_cube((i32)y, cube_info.rng);
            chunk->set_cube_no_lock({x_local, y_local, z_local}, cube);
          } else {
            auto cube = blended_biome.get_air_cube((i32)y, cube_info.rng);
            chunk->set_cube_no_lock({x_local, y_local, z_local}, cube);
          }
          continue;
        }

        int depth = 0;
        for (; depth < 4; depth += 1) {
          if (!chunk_solid_cubes_array.get_cube_info({x_local, y_local + depth + 1, z_local}).is_solid) {
            break;
          }
        }

        auto cube = blended_biome.get_ground_cube((i32)y, depth, cube_info.rng);
        chunk->set_cube_no_lock({x_local, y_local, z_local}, cube);
      }
    }
  }

  print(chunk->position);
}
