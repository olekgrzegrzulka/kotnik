#pragma once
#include <array>
#include <bitset>
#include <optional>
#include <utility>
#include <vector>
#include <stdint.h>
#include "common.hpp"
#include "cubes.hpp"

constexpr static bool is_local_pos_valid(LocalPos local_pos);
constexpr static LocalPos index_to_local_pos(size_t index);
constexpr static size_t local_pos_to_index(LocalPos pos);
constexpr static CubePos local_pos_to_cube_pos(ChunkPos chunk_pos, LocalPos local_pos);
constexpr static ChunkPos world_pos_to_chunk_pos(CubePos cube_pos);
constexpr static std::pair<ChunkPos, LocalPos> cube_to_local(CubePos cube_pos);
constexpr static ChunkPos neigbour_chunk_pos(ChunkPos chunk_pos, LocalPos offset);
constexpr static LocalPos wrap_around_local_pos(LocalPos local_pos);

class ChunkMesh;

class Chunk {
public:
  static constexpr i32 chunk_size = 32;
  static constexpr i32 chunk_cube_count = chunk_size * chunk_size * chunk_size;

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

  void update_mesh_update_flags(LocalPos local_pos);

  std::vector<std::pair<CubePos, CubeId>> neigbour_chunks_cubes_to_set;

  std::unique_ptr<ChunkMesh> mesh;

private:
  std::vector<CubeId> cubes{};
  bool no_cubes = true;

  // A true value means that the cube at index is occluded. All cubes with neigbours outside chunk bounds always have value of false
  std::bitset<chunk_cube_count> occlusion_map;

  // The highest cube in chunk. Empty if column has no cubes
  std::array<std::optional<uint16_t>, chunk_size * chunk_size> heightmap{};

public:
  Chunk(ChunkPos);
  ~Chunk();

  void update();

  // Setters
  void set_cube(LocalPos, CubeId);
  void set_cube_maybe_neigbour(LocalPos, CubeId);
  void set_cube_index(u32 index, CubeId);

  // Getters
  bool is_cube_occluded(LocalPos local_pos) const {
    ensure(is_local_pos_valid(local_pos));
    return occlusion_map[local_pos_to_index(local_pos)];
  }

  bool is_solid(LocalPos local_pos) const {
    return get_cube(local_pos) != CubeId::AIR;
  }

  CubeId get_cube(LocalPos local_pos) const {
    return cubes[local_pos_to_index(local_pos)];
  }

  bool has_no_cubes() const { return no_cubes; }

  std::optional<uint16_t> get_heightmap(uint16_t x, uint16_t z) const {
    return heightmap[x + z * chunk_size];
  }

  std::vector<CubeId> get_cubes() const {
    return cubes;
  }

  decltype(occlusion_map) get_occlusion_map() const {
    return occlusion_map;
  }

private:
  void update_occlusion_map(LocalPos local_pos);
};

// Checks if a local position is in range of chunk's array
constexpr static bool is_local_pos_valid(LocalPos local_pos) {
  return local_pos.x >= 0 && local_pos.y >= 0 && local_pos.z >= 0 && local_pos.x < Chunk::chunk_size && local_pos.y < Chunk::chunk_size && local_pos.z < Chunk::chunk_size;
}

// Returns local position  of cube identified by given index of chunk's array
constexpr static LocalPos index_to_local_pos(size_t index) {
  ChunkPos pos;
  pos.x = index % Chunk::chunk_size;
  pos.y = (index / Chunk::chunk_size) % Chunk::chunk_size;
  pos.z = index / (Chunk::chunk_size * Chunk::chunk_size);
  return pos;
}

// Returns chunk's array index to the cube located at the given local position
constexpr static size_t local_pos_to_index(LocalPos pos) {
  size_t index = pos.x + pos.y * Chunk::chunk_size + pos.z * Chunk::chunk_size * Chunk::chunk_size;
  return index;
}

// Returns cube position of given a chunk position and a (not necessarily in chunk bounds) local position
constexpr static CubePos local_pos_to_cube_pos(ChunkPos chunk_pos, LocalPos local_pos) {
  return (chunk_pos * (i32)Chunk::chunk_size) + local_pos;
}

// Returns chunk position that contains the given world position
constexpr static ChunkPos world_pos_to_chunk_pos(CubePos cube_pos) {
  ChunkPos chunk_pos;
  chunk_pos.x = (cube_pos.x >= 0) ? (cube_pos.x / (i32)Chunk::chunk_size) : ((cube_pos.x - Chunk::chunk_size + 1) / (i32)Chunk::chunk_size);
  chunk_pos.y = (cube_pos.y >= 0) ? (cube_pos.y / (i32)Chunk::chunk_size) : ((cube_pos.y - Chunk::chunk_size + 1) / (i32)Chunk::chunk_size);
  chunk_pos.z = (cube_pos.z >= 0) ? (cube_pos.z / (i32)Chunk::chunk_size) : ((cube_pos.z - Chunk::chunk_size + 1) / (i32)Chunk::chunk_size);

  return chunk_pos;
}

// Returns a pair of chunk and local positions of a global cube position
constexpr static std::pair<ChunkPos, LocalPos> cube_to_local(CubePos cube_pos) {
  ChunkPos chunk_pos;
  chunk_pos.x = (cube_pos.x >= 0) ? (cube_pos.x / (i32)Chunk::chunk_size) : ((cube_pos.x - Chunk::chunk_size + 1) / (i32)Chunk::chunk_size);
  chunk_pos.y = (cube_pos.y >= 0) ? (cube_pos.y / (i32)Chunk::chunk_size) : ((cube_pos.y - Chunk::chunk_size + 1) / (i32)Chunk::chunk_size);
  chunk_pos.z = (cube_pos.z >= 0) ? (cube_pos.z / (i32)Chunk::chunk_size) : ((cube_pos.z - Chunk::chunk_size + 1) / (i32)Chunk::chunk_size);

  LocalPos local_pos = cube_pos - ((i32)Chunk::chunk_size * chunk_pos);
  return {chunk_pos, local_pos};
}

// Returns a ChunkPosition at an offset from the given chunk position
constexpr static ChunkPos neigbour_chunk_pos(ChunkPos chunk_pos, LocalPos offset) {
  return {
      chunk_pos.x + offset.x / Chunk::chunk_size - (i32)(offset.x < 0),
      chunk_pos.y + offset.y / Chunk::chunk_size - (i32)(offset.y < 0),
      chunk_pos.z + offset.z / Chunk::chunk_size - (i32)(offset.z < 0),
  };
}

constexpr static LocalPos wrap_around_local_pos(LocalPos local_pos) {
  local_pos.x = (local_pos.x % Chunk::chunk_size + Chunk::chunk_size) % Chunk::chunk_size;
  local_pos.y = (local_pos.y % Chunk::chunk_size + Chunk::chunk_size) % Chunk::chunk_size;
  local_pos.z = (local_pos.z % Chunk::chunk_size + Chunk::chunk_size) % Chunk::chunk_size;
  return local_pos;
}