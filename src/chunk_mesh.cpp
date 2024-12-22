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

  chunks_data.resize(3 * 3 * 3);

  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* chunk = world.get_chunk(chunk_pos_ + ChunkPos{x, y, z});
        size_t chunk_i = (x + 1) + (y + 1) * 3 + (z + 1) * 9;
        if (chunk->has_no_cubes()) {
          continue;
        }
        chunks_data[chunk_i] = chunk->get_cubes();
      }
    }
  }
  valid = true;
  chunk_pos = chunk_pos_;
}

CubeId ChunkMeshData::get_cube_id(LocalPos at) {
  auto chunk_pos_ = neigbour_chunk_pos({0, 0, 0}, at);
  i32 i = (chunk_pos_.x + 1) + (chunk_pos_.y + 1) * 3 + (chunk_pos_.z + 1) * 9;
  ensure(i >= 0 && i < 27);
  if (chunks_data[i].size() == 0) { return CubeId::AIR; }
  return chunks_data[i][local_pos_to_index(wrap_around_local_pos(at))];
}

ChunkMesh::ChunkMesh() {
}

ChunkMesh::ChunkMesh(std::unique_ptr<ChunkMeshData> data) {
  BENCHMARK("chunk meshing");
  const ChunkPos chunk_pos = data->get_chunk_pos();

  for (size_t i = 0; i < (size_t)Chunk::chunk_cube_count; i += 1) {
    LocalPos l = index_to_local_pos(i);
    if (data->get_cube_id(l) == CubeId::AIR) { continue; }

    auto& cube = cubes_get(data->get_cube_id(l));

    NeigbourCubeIds neigbour_cube_ids{};

    neigbour_cube_ids.center = data->get_cube_id(LocalPos{l.x, l.y, l.z});

    // Straight neigbour_cube_ids
    neigbour_cube_ids.left = data->get_cube_id(l + LocalPos{-1, +0, +0});
    neigbour_cube_ids.right = data->get_cube_id(l + LocalPos{+1, +0, +0});
    neigbour_cube_ids.bottom = data->get_cube_id(l + LocalPos{+0, -1, +0});
    neigbour_cube_ids.top = data->get_cube_id(l + LocalPos{+0, +1, +0});
    neigbour_cube_ids.front = data->get_cube_id(l + LocalPos{+0, +0, -1});
    neigbour_cube_ids.back = data->get_cube_id(l + LocalPos{+0, +0, +1});

    // Edge neighbours
    neigbour_cube_ids.left_bottom = data->get_cube_id(l + LocalPos{-1, -1, +0});
    neigbour_cube_ids.right_bottom = data->get_cube_id(l + LocalPos{+1, -1, +0});
    neigbour_cube_ids.front_bottom = data->get_cube_id(l + LocalPos{+0, -1, -1});
    neigbour_cube_ids.back_bottom = data->get_cube_id(l + LocalPos{+0, -1, +1});

    neigbour_cube_ids.left_top = data->get_cube_id(l + LocalPos{-1, +1, +0});
    neigbour_cube_ids.right_top = data->get_cube_id(l + LocalPos{+1, +1, +0});
    neigbour_cube_ids.front_top = data->get_cube_id(l + LocalPos{+0, +1, -1});
    neigbour_cube_ids.back_top = data->get_cube_id(l + LocalPos{+0, +1, +1});

    neigbour_cube_ids.left_front = data->get_cube_id(l + LocalPos{-1, +0, -1});
    neigbour_cube_ids.right_front = data->get_cube_id(l + LocalPos{+1, +0, -1});
    neigbour_cube_ids.left_back = data->get_cube_id(l + LocalPos{-1, +0, +1});
    neigbour_cube_ids.right_back = data->get_cube_id(l + LocalPos{+1, +0, +1});

    // Corner neighbours
    neigbour_cube_ids.left_bottom_front = data->get_cube_id(l + LocalPos{-1, -1, -1});
    neigbour_cube_ids.left_bottom_back = data->get_cube_id(l + LocalPos{-1, -1, +1});
    neigbour_cube_ids.left_top_front = data->get_cube_id(l + LocalPos{-1, +1, -1});
    neigbour_cube_ids.left_top_back = data->get_cube_id(l + LocalPos{-1, +1, +1});
    neigbour_cube_ids.right_bottom_front = data->get_cube_id(l + LocalPos{+1, -1, -1});
    neigbour_cube_ids.right_bottom_back = data->get_cube_id(l + LocalPos{+1, -1, +1});
    neigbour_cube_ids.right_top_front = data->get_cube_id(l + LocalPos{+1, +1, -1});
    neigbour_cube_ids.right_top_back = data->get_cube_id(l + LocalPos{+1, +1, +1});

    if (cube.draw_data.is_translucent) {
      cube.get_vertices(chunk_pos * Chunk::chunk_size + l, neigbour_cube_ids, std::nullopt, vertices_translucent);
    } else {
      cube.get_vertices(chunk_pos * Chunk::chunk_size + l, neigbour_cube_ids, std::nullopt, vertices);
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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CompactVertex), vertices.data(), GL_STATIC_DRAW);

    // VAO
    glGenVertexArrays(1, (GLuint*)&vao);
    glBindVertexArray(vao);

    // Bind vertex position
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CompactVertex), (void*)offsetof(CompactVertex, pos));

    // Bind packed normals information
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(CompactVertex), (void*)offsetof(CompactVertex, pack));

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
    glBufferData(GL_ARRAY_BUFFER, vertices_translucent.size() * sizeof(CompactVertex), vertices_translucent.data(), GL_STATIC_DRAW);

    // Bind vertex position
    glBindBuffer(GL_ARRAY_BUFFER, vbo_translucent);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CompactVertex), (void*)offsetof(CompactVertex, pos));

    // Bind packed normals information
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(CompactVertex), (void*)offsetof(CompactVertex, pack));

    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  } else {
    // Normal
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CompactVertex), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Translucent
    glBindVertexArray(vao_translucent);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_translucent);
    glBufferData(GL_ARRAY_BUFFER, vertices_translucent.size() * sizeof(CompactVertex), vertices_translucent.data(), GL_DYNAMIC_DRAW);

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
