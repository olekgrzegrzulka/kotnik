#include "chunk_mesh.hpp"
#include <memory>
#include <glm/gtx/norm.hpp>
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "glad/glad.h"
#include "world.hpp"
#include "world_renderer.hpp"

ChunkMeshData::ChunkMeshData(ChunkPos chunk_pos_, World& world) {
  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* chunk = world.get_chunk(chunk_pos_ + ChunkPos{x, y, z});
        if (!chunk) {
          valid = false;
          return;
        }
      }
    }
  }

  data.resize(27);
  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* chunk = world.get_chunk(chunk_pos_ + ChunkPos{x, y, z});
        data[neigbour_chunk_offset_to_data_index({x, y, z})] = chunk->get_cubes();
      }
    }
  }
  valid = true;
  chunk_pos = chunk_pos_;
}

CubeId ChunkMeshData::get_cube_id(LocalPos at) {
  auto chunk_pos_ = neigbour_chunk_pos({0, 0, 0}, at);
  return data[neigbour_chunk_offset_to_data_index(chunk_pos_)][local_pos_to_index(wrap_around_local_pos(at))];
}

ChunkMesh::ChunkMesh() {
}

ChunkMesh::ChunkMesh(std::unique_ptr<ChunkMeshData> chunk_mesh_data) {
  const ChunkPos chunk_pos = chunk_mesh_data->get_chunk_pos();

  for (size_t i = 0; i < (size_t)CHUNK_CUBES; i += 1) {
    LocalPos l = index_to_local_pos(i);
    if (chunk_mesh_data->get_cube_id(l) == CubeId::AIR) { continue; }

    auto cube = cubes::get(chunk_mesh_data->get_cube_id(l));

    NeigbourCubeIds neigbour_cube_ids{};

    neigbour_cube_ids.center = chunk_mesh_data->get_cube_id(LocalPos{l.x, l.y, l.z});

    // Straight neigbour_cube_ids
    neigbour_cube_ids.left = chunk_mesh_data->get_cube_id({l.x - 1, l.y + 0, l.z + 0});
    neigbour_cube_ids.right = chunk_mesh_data->get_cube_id({l.x + 1, l.y + 0, l.z + 0});
    neigbour_cube_ids.bottom = chunk_mesh_data->get_cube_id({l.x + 0, l.y - 1, l.z + 0});
    neigbour_cube_ids.top = chunk_mesh_data->get_cube_id({l.x + 0, l.y + 1, l.z + 0});
    neigbour_cube_ids.front = chunk_mesh_data->get_cube_id({l.x + 0, l.y + 0, l.z - 1});
    neigbour_cube_ids.back = chunk_mesh_data->get_cube_id({l.x + 0, l.y + 0, l.z + 1});

    // Edge neighbours
    neigbour_cube_ids.left_bottom = chunk_mesh_data->get_cube_id({l.x - 1, l.y - 1, l.z + 0});
    neigbour_cube_ids.right_bottom = chunk_mesh_data->get_cube_id({l.x + 1, l.y - 1, l.z + 0});
    neigbour_cube_ids.front_bottom = chunk_mesh_data->get_cube_id({l.x + 0, l.y - 1, l.z - 1});
    neigbour_cube_ids.back_bottom = chunk_mesh_data->get_cube_id({l.x + 0, l.y - 1, l.z + 1});

    neigbour_cube_ids.left_top = chunk_mesh_data->get_cube_id({l.x - 1, l.y + 1, l.z + 0});
    neigbour_cube_ids.right_top = chunk_mesh_data->get_cube_id({l.x + 1, l.y + 1, l.z + 0});
    neigbour_cube_ids.front_top = chunk_mesh_data->get_cube_id({l.x + 0, l.y + 1, l.z - 1});
    neigbour_cube_ids.back_top = chunk_mesh_data->get_cube_id({l.x + 0, l.y + 1, l.z + 1});

    neigbour_cube_ids.left_front = chunk_mesh_data->get_cube_id({l.x - 1, l.y + 0, l.z - 1});
    neigbour_cube_ids.right_front = chunk_mesh_data->get_cube_id({l.x + 1, l.y + 0, l.z - 1});
    neigbour_cube_ids.left_back = chunk_mesh_data->get_cube_id({l.x - 1, l.y + 0, l.z + 1});
    neigbour_cube_ids.right_back = chunk_mesh_data->get_cube_id({l.x + 1, l.y + 0, l.z + 1});

    // Corner neighbours
    neigbour_cube_ids.left_bottom_front = chunk_mesh_data->get_cube_id({l.x - 1, l.y - 1, l.z - 1});
    neigbour_cube_ids.left_bottom_back = chunk_mesh_data->get_cube_id({l.x - 1, l.y - 1, l.z + 1});
    neigbour_cube_ids.left_top_front = chunk_mesh_data->get_cube_id({l.x - 1, l.y + 1, l.z - 1});
    neigbour_cube_ids.left_top_back = chunk_mesh_data->get_cube_id({l.x - 1, l.y + 1, l.z + 1});
    neigbour_cube_ids.right_bottom_front = chunk_mesh_data->get_cube_id({l.x + 1, l.y - 1, l.z - 1});
    neigbour_cube_ids.right_bottom_back = chunk_mesh_data->get_cube_id({l.x + 1, l.y - 1, l.z + 1});
    neigbour_cube_ids.right_top_front = chunk_mesh_data->get_cube_id({l.x + 1, l.y + 1, l.z - 1});
    neigbour_cube_ids.right_top_back = chunk_mesh_data->get_cube_id({l.x + 1, l.y + 1, l.z + 1});

    if (cube.draw_data.is_translucent) {
      cube.get_vertices(chunk_pos * CHUNK_SIZE + l, neigbour_cube_ids, std::nullopt, vertices_translucent);
    } else {
      cube.get_vertices(chunk_pos * CHUNK_SIZE + l, neigbour_cube_ids, std::nullopt, vertices);
    }
  }
}

void ChunkMesh::inherit_buffers_from_previous_mesh(ChunkMesh* prev_mesh) {
  ensure(prev_mesh);
  ensure(!ready);
  vbo = prev_mesh->vbo;
  vao = prev_mesh->vao;
  vbo_translucent = prev_mesh->vbo_translucent;
  vao_translucent = prev_mesh->vao_translucent;
}

void ChunkMesh::initialize() {
  ensure((vbo == 0) == (vao == 0));
  if (vbo == 0 && vao == 0) {
    // NORMAL VERTICES

    // VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(cubes::CompactVertex), vertices.data(), GL_STATIC_DRAW);

    // VAO
    glGenVertexArrays(1, (GLuint*)&vao);
    glBindVertexArray(vao);

    // Bind vertex position
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pos));

    // Bind packed normals information
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pack));

    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // TRANSLUCENT VERTICES

    // Create and bind VAO
    glGenVertexArrays(1, (GLuint*)&vao_translucent);

    glBindVertexArray(vao_translucent);

    // Create vertex VBO
    glGenBuffers(1, &vbo_translucent);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_translucent);
    glBufferData(GL_ARRAY_BUFFER, vertices_translucent.size() * sizeof(cubes::CompactVertex), vertices_translucent.data(), GL_STATIC_DRAW);

    // Bind vertex position
    glBindBuffer(GL_ARRAY_BUFFER, vbo_translucent);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pos));

    // Bind packed normals information
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(cubes::CompactVertex), (void*)offsetof(cubes::CompactVertex, pack));

    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  } else {
    // Normal
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(cubes::CompactVertex), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Translucent
    glBindVertexArray(vao_translucent);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_translucent);
    glBufferData(GL_ARRAY_BUFFER, vertices_translucent.size() * sizeof(cubes::CompactVertex), vertices_translucent.data(), GL_DYNAMIC_DRAW);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
}

void ChunkMesh::draw() {
  if (!ready) {
    initialize();
    ready = true;
  }
  if (vao == 0) { return; }
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  glBindVertexArray(0);
}

void ChunkMesh::draw_translucent() {
  if (!ready) {
    initialize();
    ready = true;
  }
  if (vao_translucent == 0) { return; }
  glBindVertexArray(vao_translucent);
  glDrawArrays(GL_TRIANGLES, 0, vertices_translucent.size());
  glBindVertexArray(0);
}
