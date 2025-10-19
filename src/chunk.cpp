#include "chunk.hpp"
#include <memory>
#include "chunk_mesh.hpp"
#include "common.hpp"
#include "cubes.hpp"

Chunk::Chunk(ChunkPos _chunk_position) : position(_chunk_position) {
  mesh = std::make_unique<ChunkMesh>();
  heightmap.fill(chunk_size);
  lightmap.resize(chunk_size * chunk_size * chunk_size);
}

Chunk::~Chunk() {
}

void Chunk::update_mesh_update_flags(LocalPos local_pos) {
  ensure(is_local_pos_valid(local_pos));
  flags.awaiting_mesh_update = true;

  if (local_pos.x == 0) {
    flags.update_mesh_of_adjacent_chunk.left = true;
  }

  if (local_pos.x == chunk_size - 1) {
    flags.update_mesh_of_adjacent_chunk.right = true;
  }

  if (local_pos.y == 0) {
    flags.update_mesh_of_adjacent_chunk.down = true;
  }

  if (local_pos.y == chunk_size - 1) {
    flags.update_mesh_of_adjacent_chunk.up = true;
  }

  if (local_pos.z == 0) {
    flags.update_mesh_of_adjacent_chunk.front = true;
  }

  if (local_pos.z == chunk_size - 1) {
    flags.update_mesh_of_adjacent_chunk.back = true;
  }
}

void Chunk::update() {
}

void Chunk::set_cube(LocalPos local_pos, CubeId cube_id) {
  ensure(is_local_pos_valid(local_pos));

  if (no_cubes && cube_id == CubeId::AIR) { return; }

  if (no_cubes) {
    no_cubes = false;
    cubes.resize(Chunk::chunk_cube_count);
  }

  cubes[local_pos_to_index(local_pos)] = cube_id;

  update_mesh_update_flags(local_pos);

  // Compute heightmap
  auto heightmap_at = get_heightmap(local_pos.x, local_pos.z);
  // Cube which was the highest cube in chunk was set to air. Compute new heighmap
  if (cube_id == CubeId::AIR && heightmap_at.has_value() && heightmap_at.value() == local_pos.y) {
    for (i32 new_y = local_pos.y - 1; new_y >= 0; new_y -= 1) {
      if (get_cube(LocalPos{local_pos.x, new_y, local_pos.z}) != CubeId::AIR) {
        set_heightmap(local_pos.x, local_pos.z, new_y);
        break;
      }
    }
    // No light occluders in this column
    if (get_heightmap(local_pos.x, local_pos.z) == local_pos.y) {
      clear_heightmap(local_pos.x, local_pos.z);
    }
  }

  if (cube_id != CubeId::AIR && local_pos.y >= get_heightmap(local_pos.x, local_pos.z).value_or(0)) {
    set_heightmap(local_pos.x, local_pos.z, local_pos.y);
  }
}

void Chunk::set_cube_maybe_neigbour(LocalPos local_pos, CubeId cube_id) {
  if (is_local_pos_valid(local_pos)) {
    set_cube(local_pos, cube_id);
  } else {
    auto chunk_pos = neigbour_chunk_pos(position, local_pos);
    local_pos = wrap_around_local_pos(local_pos);
    ensure(is_local_pos_valid(local_pos));
    CubePos cube_pos = local_pos_to_cube_pos(chunk_pos, local_pos);
    neigbour_chunks_cubes_to_set.push_back({cube_pos, cube_id});
  }
}

void Chunk::set_cube_index(u32 index, CubeId to) {
  LocalPos at = index_to_local_pos(index);
  set_cube(at, to);
}