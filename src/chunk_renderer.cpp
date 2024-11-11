#include "chunk_renderer.hpp"
#include <glm/gtx/norm.hpp>
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "glad/glad.h"
#include "world.hpp"
#include "world_renderer.hpp"

ChunkMeshData::ChunkMeshData(ChunkPos chunk_pos, World& world) {
  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        data[neigbour_chunk_offset_to_data_index({x, y, z})] = world.get_chunk(chunk_pos + ChunkPos{x, y, z})->get_cubes();
      }
    }
  }
}

CubeId ChunkMeshData::get_cube_id(LocalPos at) {
  auto chunk_pos = neigbour_chunk_pos({0, 0, 0}, at);
  return data[neigbour_chunk_offset_to_data_index(chunk_pos)][local_pos_to_index(wrap_around_local_pos(at))];
}

ChunkRenderer::ChunkRenderer(Chunk& _chunk) : chunk(_chunk) {
}

void ChunkRenderer::rebuild_mesh(ChunkMeshData chunk_mesh_data) {
  if (is_running) { return; }
  is_running = true;

  std::vector<const Chunk*> chunks = {};

  vertices[building_index].clear();
  vertices[building_index].reserve(512);

  vertices_translucent[building_index].clear();
  vertices_translucent[building_index].reserve(512);

  for (size_t x = 0; x < CHUNK_SIZE; x += 1) {
    for (size_t y = 0; y < CHUNK_SIZE; y += 1) {
      for (size_t z = 0; z < CHUNK_SIZE; z += 1) {
        LocalPos local_pos{x, y, z};
        if (chunk_mesh_data.get_cube_id(local_pos) == CubeId::AIR) { continue; }

        auto cube = cubes::get(chunk_mesh_data.get_cube_id(local_pos));

        NeigbourCubeIds neigbour_cube_ids{};

        neigbour_cube_ids.center = chunk_mesh_data.get_cube_id(LocalPos{x, y, z});

        // Straight neigbour_cube_ids
        neigbour_cube_ids.left = chunk_mesh_data.get_cube_id({x - 1, y + 0, z + 0});
        neigbour_cube_ids.right = chunk_mesh_data.get_cube_id({x + 1, y + 0, z + 0});
        neigbour_cube_ids.bottom = chunk_mesh_data.get_cube_id({x + 0, y - 1, z + 0});
        neigbour_cube_ids.top = chunk_mesh_data.get_cube_id({x + 0, y + 1, z + 0});
        neigbour_cube_ids.front = chunk_mesh_data.get_cube_id({x + 0, y + 0, z - 1});
        neigbour_cube_ids.back = chunk_mesh_data.get_cube_id({x + 0, y + 0, z + 1});

        // Edge neighbours
        neigbour_cube_ids.left_bottom = chunk_mesh_data.get_cube_id({x - 1, y - 1, z + 0});
        neigbour_cube_ids.right_bottom = chunk_mesh_data.get_cube_id({x + 1, y - 1, z + 0});
        neigbour_cube_ids.front_bottom = chunk_mesh_data.get_cube_id({x + 0, y - 1, z - 1});
        neigbour_cube_ids.back_bottom = chunk_mesh_data.get_cube_id({x + 0, y - 1, z + 1});

        neigbour_cube_ids.left_top = chunk_mesh_data.get_cube_id({x - 1, y + 1, z + 0});
        neigbour_cube_ids.right_top = chunk_mesh_data.get_cube_id({x + 1, y + 1, z + 0});
        neigbour_cube_ids.front_top = chunk_mesh_data.get_cube_id({x + 0, y + 1, z - 1});
        neigbour_cube_ids.back_top = chunk_mesh_data.get_cube_id({x + 0, y + 1, z + 1});

        neigbour_cube_ids.left_front = chunk_mesh_data.get_cube_id({x - 1, y + 0, z - 1});
        neigbour_cube_ids.right_front = chunk_mesh_data.get_cube_id({x + 1, y + 0, z - 1});
        neigbour_cube_ids.left_back = chunk_mesh_data.get_cube_id({x - 1, y + 0, z + 1});
        neigbour_cube_ids.right_back = chunk_mesh_data.get_cube_id({x + 1, y + 0, z + 1});

        // Corner neighbours
        neigbour_cube_ids.left_bottom_front = chunk_mesh_data.get_cube_id({x - 1, y - 1, z - 1});
        neigbour_cube_ids.left_bottom_back = chunk_mesh_data.get_cube_id({x - 1, y - 1, z + 1});
        neigbour_cube_ids.left_top_front = chunk_mesh_data.get_cube_id({x - 1, y + 1, z - 1});
        neigbour_cube_ids.left_top_back = chunk_mesh_data.get_cube_id({x - 1, y + 1, z + 1});
        neigbour_cube_ids.right_bottom_front = chunk_mesh_data.get_cube_id({x + 1, y - 1, z - 1});
        neigbour_cube_ids.right_bottom_back = chunk_mesh_data.get_cube_id({x + 1, y - 1, z + 1});
        neigbour_cube_ids.right_top_front = chunk_mesh_data.get_cube_id({x + 1, y + 1, z - 1});
        neigbour_cube_ids.right_top_back = chunk_mesh_data.get_cube_id({x + 1, y + 1, z + 1});

        i32 rng = (chunk.position + local_pos).x * 11 - (chunk.position + local_pos).y * 2 + (chunk.position + local_pos).z * 3;
        if (cube.draw_data.is_translucent) {
          cube.get_vertices(chunk.position * CHUNK_SIZE + local_pos, neigbour_cube_ids, rng, vertices_translucent[building_index]);
        } else {
          cube.get_vertices(chunk.position * CHUNK_SIZE + local_pos, neigbour_cube_ids, rng, vertices[building_index]);
        }
      }
    }
  }

  is_running = false;
  can_swap_buffers = true;
}

void ChunkRenderer::swap_buffers() {
  can_swap_buffers = false;
  std::swap(building_index, drawing_index);

  //
  // NORMAL VERTICES
  //

  // Create and bind VAO
  glGenVertexArrays(1, (GLuint*)&vaos[drawing_index]);

  glBindVertexArray(vaos[drawing_index]);

  // Create vertex VBO
  glGenBuffers(1, &vbos[drawing_index]);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[drawing_index]);
  glBufferData(GL_ARRAY_BUFFER, vertices[drawing_index].size() * sizeof(cubes::CompactVertex), vertices[drawing_index].data(), GL_STATIC_DRAW);

  // Bind vertex position
  glBindBuffer(GL_ARRAY_BUFFER, vbos[drawing_index]);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pos));

  // Bind packed normals information
  glEnableVertexAttribArray(1);
  glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pack));

  // Unbind buffers
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //
  // TRANSLUCENT VERTICES
  //

  // Create and bind VAO
  glGenVertexArrays(1, (GLuint*)&vaos_translucent[drawing_index]);

  glBindVertexArray(vaos_translucent[drawing_index]);

  // Create vertex VBO
  glGenBuffers(1, &vbos_translucent[drawing_index]);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_translucent[drawing_index]);
  glBufferData(GL_ARRAY_BUFFER, vertices_translucent[drawing_index].size() * sizeof(cubes::CompactVertex), vertices_translucent[drawing_index].data(), GL_STATIC_DRAW);

  // Bind vertex position
  glBindBuffer(GL_ARRAY_BUFFER, vbos_translucent[drawing_index]);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pos));

  // Bind packed normals information
  glEnableVertexAttribArray(1);
  glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pack));

  // Unbind buffers
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //
  // CLEAN-UP OLD VERTICES
  //

  vertices[building_index].clear();
  vertices_translucent[building_index].clear();
}

void ChunkRenderer::draw() const {
  glBindVertexArray(vaos[drawing_index]);

  glDrawArrays(GL_TRIANGLES, 0, vertices[drawing_index].size());

  glBindVertexArray(0);
}

void ChunkRenderer::draw_translucent() const {
  glBindVertexArray(vaos_translucent[drawing_index]);

  glDrawArrays(GL_TRIANGLES, 0, vertices_translucent[drawing_index].size());

  glBindVertexArray(0);
}
