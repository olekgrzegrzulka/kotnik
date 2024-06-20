#pragma once

#include "world_gen.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include "biome.hpp"
#include "biome_map.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "world.hpp"

using Biomes::BlendedBiome;

struct ChunkSolidCubesArray {
public:
  static constexpr uint32_t lip_negative_x = 0;
  static constexpr uint32_t lip_positive_x = 0;

  static constexpr uint32_t lip_negative_y = 1;
  static constexpr uint32_t lip_positive_y = 4;

  static constexpr uint32_t lip_negative_z = 0;
  static constexpr uint32_t lip_positive_z = 0;

  ChunkSolidCubesArray(const WorldGen& wg, ChunkPos chunk_pos) : world_gen(wg) {
    begin = chunk_pos * CHUNK_SIZE - CubePos{lip_negative_x, lip_negative_y, lip_negative_z};
    end = chunk_pos * CHUNK_SIZE + CubePos{CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1} + CubePos{lip_positive_x, lip_positive_y, lip_positive_z};

    static constexpr size_t array_size = ((CHUNK_SIZE + lip_negative_x + lip_positive_x) *
                                          (CHUNK_SIZE + lip_negative_y + lip_positive_y) *
                                          (CHUNK_SIZE + lip_negative_z + lip_positive_z));

    cubes.resize(array_size);

    for (int32_t x_local = -lip_negative_x; x_local < (int32_t)(CHUNK_SIZE + lip_positive_x); x_local += 1) {
      for (int32_t z_local = -lip_negative_z; z_local < (int32_t)(CHUNK_SIZE + lip_positive_z); z_local += 1) {
        LocalPos local_pos = {x_local, 0, z_local};
        WorldPos world_pos = begin + local_pos;

        auto blended_biome = world_gen.get_blended_biome(world_pos);

        for (int32_t y_local = -lip_negative_y; y_local < (int32_t)(CHUNK_SIZE + lip_positive_y); y_local += 1) {
          local_pos.y = y_local;
          world_pos.y = begin.y + local_pos.y;
          size_t i = get_index(local_pos);
          cubes[i] = world_gen.is_ground(world_pos, blended_biome);
        }
      }
    }
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
  std::vector<uint8_t> cubes;
};

WorldGen::WorldGen(World& w) : world(w) {
  constexpr int seed = 888;

  noise_heightmap.SetSeed(seed);
  noise_heightmap.SetFrequency(0.008564f);
  noise_heightmap.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_heightmap.SetFractalOctaves(3);
  noise_heightmap.SetFractalGain(0.4f);
  noise_heightmap.SetFractalLacunarity(2.57f);

  noise_3d.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
  noise_3d.SetFrequency(0.005f);
  noise_3d.SetSeed(seed);
  noise_3d.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
  noise_3d.SetFractalOctaves(3);
  noise_3d.SetFractalGain(0.6785f);
  noise_3d.SetFractalLacunarity(2.481f);
  noise_3d.SetDomainWarpType(FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid);
  noise_3d.SetDomainWarpAmp(80.0f);

  noise_humidity.SetSeed(seed + 1);
  noise_humidity.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Value);
  noise_humidity.SetFrequency(0.0077f);
  noise_humidity.SetFractalOctaves(3);

  noise_temperature.SetSeed(seed + 2);
  noise_temperature.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Value);
  noise_temperature.SetFrequency(0.0077f);
  noise_temperature.SetFractalOctaves(3);
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

  float value_3d = (noise_3d.GetNoise(pos.x, pos.y * 2.0f, pos.z) + 1.0f) * 0.5f * blended_biome.get_noise_3d_multiplier();
  value_3d = 1.0f + value_3d * 0.032f;
  float value = (blended_biome.get_base_height() + value_height) * value_3d;

  return value > pos.y;
}

bool WorldGen::is_ground(WorldPos pos) const {
  auto blended_biome = get_blended_biome(pos);

  return is_ground(pos, blended_biome);
}

void WorldGen::generate_chunk(Chunk* chunk) const {
  auto chunk_solid_cubes_array = ChunkSolidCubesArray(*this, chunk->position);

  for (size_t x_local = 0; x_local < CHUNK_SIZE; x_local += 1) {
    for (size_t z_local = 0; z_local < CHUNK_SIZE; z_local += 1) {
      float x = chunk->position.x * CHUNK_SIZE + (int)x_local;
      float z = chunk->position.z * CHUNK_SIZE + (int)z_local;
      auto blended_biome = get_blended_biome({x, 0, z});

      for (size_t y_local = 0; y_local < CHUNK_SIZE; y_local += 1) {
        // float y = chunk->position.y * CHUNK_SIZE + (int)y_local;

        bool solid = chunk_solid_cubes_array.is_solid({x_local, y_local, z_local});
        if (!solid) { continue; }
        int depth = 0;
        for (; depth < 4; depth += 1) {
          if (!chunk_solid_cubes_array.is_solid({x_local, y_local + depth + 1, z_local})) {
            break;
          }
        }

        auto cube = blended_biome.get_ground_cube(depth);
        chunk->set_cube_no_lock({x_local, y_local, z_local}, cube);
      }
    }
  }

  print(chunk->position);
}
