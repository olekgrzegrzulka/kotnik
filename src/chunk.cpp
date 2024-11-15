#include "chunk.hpp"
#include <cassert>
#include <memory>
#include "chunk_mesh.hpp"
#include "common.hpp"
#include "cubes.hpp"

Chunk::Chunk(ChunkPos _chunk_position) : position(_chunk_position) {
  cubes.resize(CHUNK_CUBES);
  mesh = std::make_unique<ChunkMesh>();
}

Chunk::~Chunk() {
}

WorldPos Chunk::get_center_pos() const {
  return WorldPos{position * CHUNK_SIZE} + WorldPos{CHUNK_SIZE >> 1, CHUNK_SIZE >> 1, CHUNK_SIZE >> 1};
}

void Chunk::update_mesh_update_flags(LocalPos local_pos) {
  assert(is_local_pos_valid(local_pos));
  flags.awaiting_mesh_update = true;

  if (local_pos.x == 0) {
    flags.update_mesh_of_adjacent_chunk.left = true;
  }

  if (local_pos.x == CHUNK_SIZE - 1) {
    flags.update_mesh_of_adjacent_chunk.right = true;
  }

  if (local_pos.y == 0) {
    flags.update_mesh_of_adjacent_chunk.down = true;
  }

  if (local_pos.y == CHUNK_SIZE - 1) {
    flags.update_mesh_of_adjacent_chunk.up = true;
  }

  if (local_pos.z == 0) {
    flags.update_mesh_of_adjacent_chunk.front = true;
  }

  if (local_pos.z == CHUNK_SIZE - 1) {
    flags.update_mesh_of_adjacent_chunk.back = true;
  }
}

void Chunk::update() {
}

bool Chunk::is_solid(LocalPos local_pos) const {
  return get_cube(local_pos) != CubeId::AIR;
}

CubeId Chunk::get_cube(LocalPos local_pos) const {
  return cubes[local_pos_to_index(local_pos)];
}

void Chunk::set_cube(LocalPos local_pos, CubeId cube_id) {
  ensure(is_local_pos_valid(local_pos));

  cubes[local_pos_to_index(local_pos)] = cube_id;

  update_mesh_update_flags(local_pos);

  if (cube_id != CubeId::AIR) {
    no_cubes = false;
  }

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

  neigbour_chunks_cubes_to_set.push_back({cube_pos, cube_id});
}

void Chunk::set_cube_no_lock(LocalPos local_pos, CubeId cube_id) {
  assert(is_local_pos_valid(local_pos));

  cubes[local_pos_to_index(local_pos)] = cube_id;

  update_mesh_update_flags(local_pos);

  if (cube_id != CubeId::AIR) {
    no_cubes = false;
  }

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

  // FIXME this could be done more effeciently
  update_occlusion_map(local_pos);
  update_occlusion_map(local_pos + LocalPos{-1, 0, 0});
  update_occlusion_map(local_pos + LocalPos{1, 0, 0});
  update_occlusion_map(local_pos + LocalPos{0, -1, 0});
  update_occlusion_map(local_pos + LocalPos{0, 1, 0});
  update_occlusion_map(local_pos + LocalPos{0, 0, -1});
  update_occlusion_map(local_pos + LocalPos{0, 0, 1});
}

void Chunk::set_cube_index(u32 index, CubeId to) {
  LocalPos at = index_to_local_pos(index);
  set_cube(at, to);
}

void Chunk::set_cube_index_no_lock(u32 index, CubeId to) {
  LocalPos at = index_to_local_pos(index);
  set_cube_no_lock(at, to);
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

  const std::array<LocalPos, 6> neigbours = {
      local_pos + LocalPos{-1, 0, 0},
      local_pos + LocalPos{1, 0, 0},
      local_pos + LocalPos{0, -1, 0},
      local_pos + LocalPos{0, 1, 0},
      local_pos + LocalPos{0, 0, -1},
      local_pos + LocalPos{0, 0, 1},
  };

  static constexpr std::array<Dir, 6> neigbour_face_dirs{
      Dir::RIGHT,
      Dir::LEFT,
      Dir::TOP,
      Dir::BOTTOM,
      Dir::BACK,
      Dir::FRONT,
  };

  for (size_t i = 0; i < 6; i += 1) {
    const CubePos neigb_pos = neigbours[i];

    // One of cube's neigbours out of chunk bounds. Cannot determine if cube occluded.
    if (!is_local_pos_valid(neigb_pos)) {
      occlusion_map[index] = false;
      return;
    }

    const CubeId neigb_id = get_cube(neigb_pos);
    const Dir neigb_face_dir = neigbour_face_dirs[i];

    // One of cube's neigbours is air. Cube can't be occluded
    if (neigb_id == CubeId::AIR) {
      occlusion_map[index] = false;
      return;
    }

    const auto& neigb_occlude_adjacent_cube = cubes::get(neigb_id).draw_data.occlude_adjacent_cube;
    using enum cubes::Cube::CubeOccludeMode;
    if ((neigb_face_dir == Dir::LEFT && neigb_occlude_adjacent_cube.left == ALWAYS) ||
        (neigb_face_dir == Dir::RIGHT && neigb_occlude_adjacent_cube.right == ALWAYS) ||
        (neigb_face_dir == Dir::BOTTOM && neigb_occlude_adjacent_cube.bottom == ALWAYS) ||
        (neigb_face_dir == Dir::TOP && neigb_occlude_adjacent_cube.top == ALWAYS) ||
        (neigb_face_dir == Dir::FRONT && neigb_occlude_adjacent_cube.front == ALWAYS) ||
        (neigb_face_dir == Dir::BACK && neigb_occlude_adjacent_cube.back == ALWAYS)) {
      occlusion_map[index] = false;
      return;
    }
  }

  occlusion_map[index] = true;
}