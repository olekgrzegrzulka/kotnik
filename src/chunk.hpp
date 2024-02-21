#pragma once
#include <array>
#include <atomic>
#include <bitset>
#include <optional>
#include <utility> // for std::pair
#include <vector>
#include <stdint.h>
#include "common.hpp"
#include "cubes.hpp"

#define CHUNK_SIZE (32)
#define CHUNK_CUBES (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class World;

// Checks if a local position is in range of chunk's array
constexpr bool is_local_pos_valid(LocalPos local_pos) {
  return local_pos.x >= 0 && local_pos.y >= 0 && local_pos.z >= 0 && local_pos.x < CHUNK_SIZE && local_pos.y < CHUNK_SIZE && local_pos.z < CHUNK_SIZE;
}

// Returns local position  of cube identified by given index of chunk's array
static LocalPos index_to_local_pos(size_t index) {
  ChunkPos pos;
  pos.x = index % CHUNK_SIZE;
  pos.y = (index / CHUNK_SIZE) % CHUNK_SIZE;
  pos.z = index / (CHUNK_SIZE * CHUNK_SIZE);
  return pos;
}

// Returns chunk's array index to the cube located at the given local position
static size_t local_pos_to_index(LocalPos pos) {
  // assert(is_local_pos_valid(pos));
  size_t index = pos.x + pos.y * CHUNK_SIZE + pos.z * CHUNK_SIZE * CHUNK_SIZE;
  return index;
}

// Returns cube position of given a chunk position and a (not necessarily in chunk bounds) local position
static inline constexpr CubePos local_pos_to_cube_pos(ChunkPos chunk_pos, LocalPos local_pos) {
  return (chunk_pos * (int32_t)CHUNK_SIZE) + local_pos;
}

// Returns chunk position that contains the given world position
constexpr ChunkPos world_pos_to_chunk_pos(CubePos cube_pos) {
  ChunkPos chunk_pos;
  chunk_pos.x = (cube_pos.x >= 0) ? (cube_pos.x / (int32_t)CHUNK_SIZE) : ((cube_pos.x - CHUNK_SIZE + 1) / (int32_t)CHUNK_SIZE);
  chunk_pos.y = (cube_pos.y >= 0) ? (cube_pos.y / (int32_t)CHUNK_SIZE) : ((cube_pos.y - CHUNK_SIZE + 1) / (int32_t)CHUNK_SIZE);
  chunk_pos.z = (cube_pos.z >= 0) ? (cube_pos.z / (int32_t)CHUNK_SIZE) : ((cube_pos.z - CHUNK_SIZE + 1) / (int32_t)CHUNK_SIZE);

  return chunk_pos;
}

// Returns a pair of chunk and local positions of a global cube position
constexpr std::pair<ChunkPos, LocalPos> cube_to_local(CubePos cube_pos) {
  ChunkPos chunk_pos;
  chunk_pos.x = (cube_pos.x >= 0) ? (cube_pos.x / (int32_t)CHUNK_SIZE) : ((cube_pos.x - CHUNK_SIZE + 1) / (int32_t)CHUNK_SIZE);
  chunk_pos.y = (cube_pos.y >= 0) ? (cube_pos.y / (int32_t)CHUNK_SIZE) : ((cube_pos.y - CHUNK_SIZE + 1) / (int32_t)CHUNK_SIZE);
  chunk_pos.z = (cube_pos.z >= 0) ? (cube_pos.z / (int32_t)CHUNK_SIZE) : ((cube_pos.z - CHUNK_SIZE + 1) / (int32_t)CHUNK_SIZE);

  LocalPos local_pos = cube_pos - ((int32_t)CHUNK_SIZE * chunk_pos);
  return {chunk_pos, local_pos};
}

enum class ChunkEventType {
  SET_CUBE,
};

struct ChunkEvent {
  ChunkEventType type;
  LocalPos local_pos;
  CubeId cube_id;
};

class ChunkRenderer;

class Chunk {
public:
  World& world;

  ChunkPos position{};
  // Every time the cubes array is modified, cube position is added to this array.
  // The managment of this array is handled in World::update()
  struct {
    // When chunk is ready, it can be accessed from the World::get_chunk() method. Used when chunk is being loaded in a background thread
    bool ready = false;

    bool update_geometry = false;

    // When > 0 prevents chunk data from being modified, instead all chunk modifications are sent to event_queue
    size_t threads_reading = 0;

    // Prevents chunk data from being modified, instead all chunk modifications are sent to event_queue
    bool thread_writing = false;

    bool is_read_locked() const {
      return thread_writing;
    }

    bool is_write_locked() const {
      return thread_writing || threads_reading > 0;
    }

  } flags;

  std::unordered_map<CubePos, CubeId, Vec3Hasher> neigbour_chunks_cubes_to_set;

  ChunkRenderer* renderer{};

  // This flag is set when a background thread finishes generating draw_data, allowing binding VAO on next World::update()
  // std::atomic<bool> upload_vao = false;
  bool upload_vao = false;
  // std::atomic<bool> test;

  std::vector<ChunkEvent> event_queue;

private:
  std::array<CubeId, CHUNK_CUBES> cubes{};

  std::array<LightLevel, CHUNK_CUBES> lightmap{};

  // A true value means that the cube at index is occluded. All cubes with neigbours outside chunk bounds always have value of false
  std::bitset<CHUNK_CUBES> occlusion_map;

  // The highest cube in chunk. Empty if column has no cubes
  std::array<std::optional<uint16_t>, CHUNK_SIZE * CHUNK_SIZE> heightmap{};

public:
  Chunk(World& _world, ChunkPos _chunk_position);

  ~Chunk();

  bool is_cube_occluded(LocalPos local_pos) const;

  void update();

  bool is_solid(LocalPos at) const;

  void clear_lightmap() {
    lightmap.fill({0, 0, 0});
  }

  CubeId get_cube(LocalPos at) const;
  LightLevel get_lightmap(LocalPos at) const;

  void set_cube(LocalPos local_pos, CubeId cube_id);
  void set_cube_no_lock(LocalPos local_pos, CubeId cube_id);
  void set_cube_index(uint32_t index, CubeId cube_id);
  void set_cube_index_no_lock(uint32_t index, CubeId cube_id);

  // Allows changing cubes of different chunks by storing them, for World to set them later.
  void set_cube_neigbour(ChunkPos chunk_pos, LocalPos local_pos, CubeId cube_id);

  void set_lightmap(LocalPos local_pos, LightLevel cube_id);

  std::optional<uint16_t> get_heightmap(uint16_t x, uint16_t z) const;

private:
  void update_occlusion_map(LocalPos local_pos);
};