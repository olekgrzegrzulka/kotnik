#include "chunk_renderer.hpp"
#include <glm/gtx/norm.hpp>
#include "chunk.hpp"
#include "cubes.hpp"
#include "glad/glad.h"
#include "world.hpp"
#include "world_renderer.hpp"

ChunkRenderer::ChunkRenderer(Chunk& _chunk) : chunk(_chunk) {
}

void ChunkRenderer::rebuild_mesh(std::vector<Chunk*> chunk_list) {
  if (is_running) { return; }
  is_running = true;

  assert(chunk.flags.ready);

  std::vector<const Chunk*> chunks = {};

  vertices[building_index].clear();
  vertices[building_index].reserve(512);

  vertices_translucent[building_index].clear();
  vertices_translucent[building_index].reserve(512);

  for (size_t x = 0; x < CHUNK_SIZE; x += 1) {
    for (size_t y = 0; y < CHUNK_SIZE; y += 1) {
      for (size_t z = 0; z < CHUNK_SIZE; z += 1) {
        LocalPos local_pos{x, y, z};
        if (chunk.is_cube_occluded(local_pos)) { continue; }
        if (!chunk.is_solid(local_pos)) { continue; }
        if (chunk.get_cube(local_pos) == CubeId::AIR) { continue; }
        CubePos cube_pos = local_pos + chunk.position * CubePos{CHUNK_SIZE};

        auto cube = cubes::get(chunk.get_cube(local_pos));
        NeigbourCubeIds neigbour_cube_ids = chunk.world.get_neigbour_ids(cube_pos, WorldRenderer::ambient_occlusion_enabled, WorldRenderer::ambient_occlusion_enabled);

        if (cube.draw_data.is_translucent) {
          cube.get_vertices(chunk.position * CHUNK_SIZE + local_pos, neigbour_cube_ids, vertices_translucent[building_index]);
        } else {
          cube.get_vertices(chunk.position * CHUNK_SIZE + local_pos, neigbour_cube_ids, vertices[building_index]);
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
