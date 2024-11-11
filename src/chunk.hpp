#pragma once
#include <array>
#include <bitset>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>
#include <stdint.h>
#include "common.hpp"
#include "cubes.hpp"

#define CHUNK_SIZE (32)
#define CHUNK_CUBES (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

// Checks if a local position is in range of chunk's array
constexpr static bool is_local_pos_valid(LocalPos local_pos) {
  return local_pos.x >= 0 && local_pos.y >= 0 && local_pos.z >= 0 && local_pos.x < CHUNK_SIZE && local_pos.y < CHUNK_SIZE && local_pos.z < CHUNK_SIZE;
}

// Returns local position  of cube identified by given index of chunk's array
constexpr static LocalPos index_to_local_pos(size_t index) {
  ChunkPos pos;
  pos.x = index % CHUNK_SIZE;
  pos.y = (index / CHUNK_SIZE) % CHUNK_SIZE;
  pos.z = index / (CHUNK_SIZE * CHUNK_SIZE);
  return pos;
}

// Returns chunk's array index to the cube located at the given local position
constexpr static size_t local_pos_to_index(LocalPos pos) {
  // assert(is_local_pos_valid(pos));
  size_t index = pos.x + pos.y * CHUNK_SIZE + pos.z * CHUNK_SIZE * CHUNK_SIZE;
  return index;
}

// Returns cube position of given a chunk position and a (not necessarily in chunk bounds) local position
constexpr static CubePos local_pos_to_cube_pos(ChunkPos chunk_pos, LocalPos local_pos) {
  return (chunk_pos * (i32)CHUNK_SIZE) + local_pos;
}

// Returns chunk position that contains the given world position
constexpr static ChunkPos world_pos_to_chunk_pos(CubePos cube_pos) {
  ChunkPos chunk_pos;
  chunk_pos.x = (cube_pos.x >= 0) ? (cube_pos.x / (i32)CHUNK_SIZE) : ((cube_pos.x - CHUNK_SIZE + 1) / (i32)CHUNK_SIZE);
  chunk_pos.y = (cube_pos.y >= 0) ? (cube_pos.y / (i32)CHUNK_SIZE) : ((cube_pos.y - CHUNK_SIZE + 1) / (i32)CHUNK_SIZE);
  chunk_pos.z = (cube_pos.z >= 0) ? (cube_pos.z / (i32)CHUNK_SIZE) : ((cube_pos.z - CHUNK_SIZE + 1) / (i32)CHUNK_SIZE);

  return chunk_pos;
}

// Returns a pair of chunk and local positions of a global cube position
constexpr static std::pair<ChunkPos, LocalPos> cube_to_local(CubePos cube_pos) {
  ChunkPos chunk_pos;
  chunk_pos.x = (cube_pos.x >= 0) ? (cube_pos.x / (i32)CHUNK_SIZE) : ((cube_pos.x - CHUNK_SIZE + 1) / (i32)CHUNK_SIZE);
  chunk_pos.y = (cube_pos.y >= 0) ? (cube_pos.y / (i32)CHUNK_SIZE) : ((cube_pos.y - CHUNK_SIZE + 1) / (i32)CHUNK_SIZE);
  chunk_pos.z = (cube_pos.z >= 0) ? (cube_pos.z / (i32)CHUNK_SIZE) : ((cube_pos.z - CHUNK_SIZE + 1) / (i32)CHUNK_SIZE);

  LocalPos local_pos = cube_pos - ((i32)CHUNK_SIZE * chunk_pos);
  return {chunk_pos, local_pos};
}

// Returns a ChunkPosition at an offset from the given chunk position
constexpr static ChunkPos neigbour_chunk_pos(ChunkPos chunk_pos, LocalPos offset) {
  return {
      chunk_pos.x + offset.x / CHUNK_SIZE - (i32)(offset.x < 0),
      chunk_pos.y + offset.y / CHUNK_SIZE - (i32)(offset.y < 0),
      chunk_pos.z + offset.z / CHUNK_SIZE - (i32)(offset.z < 0),
  };
}

constexpr static LocalPos wrap_around_local_pos(LocalPos local_pos) {
  local_pos.x = (local_pos.x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
  local_pos.y = (local_pos.y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
  local_pos.z = (local_pos.z % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
  return local_pos;
}

class ChunkRenderer;

class Chunk {
public:
  ChunkPos position{};

  struct {
    bool awaiting_mesh_update = false;

    struct {
      bool up = false;
      bool down = false;
      bool left = false;
      bool right = false;
      bool front = false;
      bool back = false;
    } update_mesh_of_adjacent_chunk;

    // Chunk is too far from view, and may be unloaded an any time
    bool marked_for_unload = false;
  } flags;

  WorldPos get_center_pos() const;

  void update_mesh_update_flags(LocalPos local_pos);

  std::vector<std::pair<CubePos, CubeId>> neigbour_chunks_cubes_to_set;

  ChunkRenderer* renderer{};

  // This flag is set when a background thread finishes generating draw_data, allowing binding VAO on next World::update()
  // std::atomic<bool> upload_vao = false;
  bool upload_vao = false;
  // std::atomic<bool> test;

  struct {
    // bool locked = false;
    std::array<LightLevel, CHUNK_CUBES> data{};
  } lightmap;

private:
  std::array<CubeId, CHUNK_CUBES> cubes{};

  // A true value means that the cube at index is occluded. All cubes with neigbours outside chunk bounds always have value of false
  std::bitset<CHUNK_CUBES> occlusion_map;

  // The highest cube in chunk. Empty if column has no cubes
  std::array<std::optional<uint16_t>, CHUNK_SIZE * CHUNK_SIZE> heightmap{};

public:
  Chunk(ChunkPos _chunk_position);

  ~Chunk();

  bool is_cube_occluded(LocalPos local_pos) const;

  void update();

  bool is_solid(LocalPos at) const;

  CubeId get_cube(LocalPos at) const;

  void set_cube(LocalPos local_pos, CubeId cube_id);
  void set_cube_no_lock(LocalPos local_pos, CubeId cube_id);
  void set_cube_index(u32 index, CubeId cube_id);
  void set_cube_index_no_lock(u32 index, CubeId cube_id);

  // Allows changing cubes of different chunks by storing them, for World to set them later.
  void set_cube_neigbour(ChunkPos chunk_pos, LocalPos local_pos, CubeId cube_id);

  std::optional<uint16_t> get_heightmap(uint16_t x, uint16_t z) const;

  std::array<CubeId, CHUNK_CUBES> get_cubes() const {
    return cubes;
  }

private:
  void update_occlusion_map(LocalPos local_pos);
};