#include "chunk.hpp"
#include <execution>
#include "chunk_renderer.hpp"
#include "fast_noise_lite.h"
#include "world.hpp"

Chunk::Chunk(World& _world, ChunkPos _chunk_position) : world(_world), position(_chunk_position) {
}

Chunk::~Chunk() {
  if (renderer) {
    delete renderer;
  }
}

void Chunk::update() {
  // Event queue
  if (!(flags.is_write_locked())) {
    for (auto& event : event_queue) {
      if (event.type == ChunkEventType::SET_CUBE) {
        set_cube(event.local_pos, event.cube_id);
      }
    }
    event_queue.clear();
  }
}

bool Chunk::is_solid(LocalPos local_pos) const {
  return get_cube(local_pos) != CubeId::AIR;
}

CubeId Chunk::get_cube(LocalPos local_pos) const {
  return cubes[local_pos_to_index(local_pos)];
}

LightLevel Chunk::get_lightmap(LocalPos local_pos) const {
  assert(is_local_pos_valid(local_pos));
  return lightmap[local_pos_to_index(local_pos)];
}

void Chunk::set_cube(LocalPos local_pos, CubeId cube_id) {
  assert(is_local_pos_valid(local_pos));
  if (flags.is_write_locked()) {
    event_queue.emplace_back(ChunkEvent{.type = ChunkEventType::SET_CUBE, .local_pos = local_pos, .cube_id = cube_id});
    return;
  }
  cubes[local_pos_to_index(local_pos)] = cube_id;

  flags.update_geometry = true;

  // Compute heightmap
  const auto heightmap_at = heightmap[local_pos.x + local_pos.z * CHUNK_SIZE];
  // Cube which was the highest cube in chunk was set to air. Compute new heighmap
  if (cube_id == CubeId::AIR && heightmap_at.has_value() && heightmap_at.value() == local_pos.y) {
    for (int16_t new_y = local_pos.y - 1; new_y >= 0; new_y -= 1) {
      if (is_solid(LocalPos{local_pos.x, new_y, local_pos.z})) {
        heightmap[local_pos.x + local_pos.z * CHUNK_SIZE] = new_y;
        break;
      }
    }
    // If heightmap wasn't updated, then there are'nt any cubes in this column
    if (heightmap[local_pos.x + local_pos.z * CHUNK_SIZE].value() == local_pos.y) {
      heightmap[local_pos.x + local_pos.z * CHUNK_SIZE] = {};
    }
  }

  if (cube_id != CubeId::AIR && local_pos.y >= heightmap[local_pos.x + local_pos.z * CHUNK_SIZE].value_or(0)) {
    heightmap[local_pos.x + local_pos.z * CHUNK_SIZE] = local_pos.y;
  }

  update_occlusion_map(local_pos);
  update_occlusion_map(local_pos + LocalPos{-1, 0, 0});
  update_occlusion_map(local_pos + LocalPos{1, 0, 0});
  update_occlusion_map(local_pos + LocalPos{0, -1, 0});
  update_occlusion_map(local_pos + LocalPos{0, 1, 0});
  update_occlusion_map(local_pos + LocalPos{0, 0, -1});
  update_occlusion_map(local_pos + LocalPos{0, 0, 1});
}

void Chunk::set_cube_neigbour(ChunkPos chunk_pos, LocalPos local_pos, CubeId cube_id) {
  assert(is_local_pos_valid(local_pos));

  CubePos cube_pos = local_pos_to_cube_pos(chunk_pos, local_pos);

  neigbour_chunks_cubes_to_set.insert({cube_pos, cube_id});
}

void Chunk::set_cube_no_lock(LocalPos local_pos, CubeId cube_id) {
  assert(is_local_pos_valid(local_pos));

  cubes[local_pos_to_index(local_pos)] = cube_id;

  flags.update_geometry = true;

  // Compute heightmap
  const auto heightmap_at = heightmap[local_pos.x + local_pos.z * CHUNK_SIZE];
  // Cube which was the highest cube in chunk was set to air. Compute new heighmap
  if (cube_id == CubeId::AIR && heightmap_at.has_value() && heightmap_at.value() == local_pos.y) {
    for (int16_t new_y = local_pos.y - 1; new_y >= 0; new_y -= 1) {
      if (is_solid(LocalPos{local_pos.x, new_y, local_pos.z})) {
        heightmap[local_pos.x + local_pos.z * CHUNK_SIZE] = new_y;
        break;
      }
    }
    // If heightmap wasn't updated, then there are'nt any cubes in this column
    if (heightmap[local_pos.x + local_pos.z * CHUNK_SIZE].value() == local_pos.y) {
      heightmap[local_pos.x + local_pos.z * CHUNK_SIZE] = {};
    }
  }

  if (cube_id != CubeId::AIR && local_pos.y >= heightmap[local_pos.x + local_pos.z * CHUNK_SIZE].value_or(0)) {
    heightmap[local_pos.x + local_pos.z * CHUNK_SIZE] = local_pos.y;
  }

  update_occlusion_map(local_pos);
  update_occlusion_map(local_pos + LocalPos{-1, 0, 0});
  update_occlusion_map(local_pos + LocalPos{1, 0, 0});
  update_occlusion_map(local_pos + LocalPos{0, -1, 0});
  update_occlusion_map(local_pos + LocalPos{0, 1, 0});
  update_occlusion_map(local_pos + LocalPos{0, 0, -1});
  update_occlusion_map(local_pos + LocalPos{0, 0, 1});
}

void Chunk::set_cube_index(uint32_t index, CubeId to) {
  LocalPos at = index_to_local_pos(index);
  set_cube(at, to);
}

void Chunk::set_cube_index_no_lock(uint32_t index, CubeId to) {
  LocalPos at = index_to_local_pos(index);
  set_cube_no_lock(at, to);
}

void Chunk::set_lightmap(LocalPos local_pos, LightLevel light_level) {
  lightmap[local_pos_to_index(local_pos)] = light_level;
}

std::optional<uint16_t> Chunk::get_heightmap(uint16_t x, uint16_t z) const {
  return heightmap[x + z * CHUNK_SIZE];
}

bool Chunk::is_cube_occluded(LocalPos local_pos) const {
  assert(is_local_pos_valid(local_pos));
  return occlusion_map[local_pos_to_index(local_pos)];
}

void Chunk::update_occlusion_map(LocalPos local_pos) {
  if (!is_local_pos_valid(local_pos)) {
    return;
  }
  size_t index = local_pos_to_index(local_pos);

  std::vector<LocalPos> neigbours = {
      local_pos + LocalPos{-1, 0, 0},
      local_pos + LocalPos{1, 0, 0},
      local_pos + LocalPos{0, -1, 0},
      local_pos + LocalPos{0, 1, 0},
      local_pos + LocalPos{0, 0, -1},
      local_pos + LocalPos{0, 0, 1},
  };

  std::vector<Dir> neigbour_face_dirs{
      Dir::RIGHT,
      Dir::LEFT,
      Dir::TOP,
      Dir::BOTTOM,
      Dir::BACK,
      Dir::FRONT,
  };

  for (size_t i = 0; i < 6; i += 1) {
    const CubePos neigb_pos = neigbours[i];
    const Dir neigb_face_dir = neigbour_face_dirs[i];

    // One of cube's neigbours out of chunk bounds. Cannot determine if cube occluded.
    if (!is_local_pos_valid(neigb_pos)) {
      occlusion_map[index] = false;
      return;
    }

    const CubeId neigb_id = get_cube(neigb_pos);

    // One of cube's neigbours is air. Cube can't be occluded
    if (neigb_id == CubeId::AIR) {
      occlusion_map[index] = false;
      return;
    }

    // If neigbour face which faces this cube is not solid this cube can't be occluded.
    if (cube_properties[(size_t)neigb_id].render_data.face_solidity.at(neigb_face_dir) == false) {
      occlusion_map[index] = false;
      return;
    }
  }

  occlusion_map[index] = true;
}