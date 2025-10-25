#include "chunk_mesh.hpp"
#include <memory>
#include <glm/gtx/norm.hpp>
#include "biome.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "debug.hpp"
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

  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* chunk = world.get_chunk(chunk_pos_ + ChunkPos{x, y, z});
        size_t chunk_i = (x + 1) + (y + 1) * 3 + (z + 1) * 9;
        if (chunk->has_no_cubes()) {
          continue;
        }
        cubes[chunk_i] = chunk->get_cubes_copy();
        lightmaps[chunk_i] = chunk->get_lightmap_copy();
      }
    }
  }
  valid = true;
  chunk_pos = chunk_pos_;

  i32 half = Chunk::chunk_size / 2;
  auto& wg = world.get_world_gen();
  CubePos middle = chunk_pos * Chunk::chunk_size + CubePos{half, half, half};
  foliage_color_neg_x_neg_z = wg.get_blended_biome(middle + CubePos{-half, 0, -half}).foliage_color;
  foliage_color_neg_x_pos_z = wg.get_blended_biome(middle + CubePos{-half, 0, +half}).foliage_color;
  foliage_color_pos_x_neg_z = wg.get_blended_biome(middle + CubePos{+half, 0, -half}).foliage_color;
  foliage_color_pos_x_pos_z = wg.get_blended_biome(middle + CubePos{+half, 0, +half}).foliage_color;
}

inline CubeId ChunkMeshData::get_cube_id(LocalPos at, bool check_if_neigbour_chunk) {
  if (check_if_neigbour_chunk) {
    auto chunk_pos_ = neigbour_chunk_pos({0, 0, 0}, at);
    size_t i = (chunk_pos_.x + 1) + (chunk_pos_.y + 1) * 3 + (chunk_pos_.z + 1) * 9;
    if (cubes[i].size() == 0) { return CubeId::AIR; }
    return cubes[i][local_pos_to_index(wrap_around_local_pos(at))];
  } else {
    return cubes[13][local_pos_to_index(at)];
  }
}

inline u8 ChunkMeshData::get_lightmap(LocalPos at, bool check_if_neigbour_chunk) {
  if (check_if_neigbour_chunk) {
    auto chunk_pos_ = neigbour_chunk_pos({0, 0, 0}, at);
    size_t i = (chunk_pos_.x + 1) + (chunk_pos_.y + 1) * 3 + (chunk_pos_.z + 1) * 9;
    if (cubes[i].size() == 0) { return 0; }
    return lightmaps[i][local_pos_to_index(wrap_around_local_pos(at))];
  } else {
    return lightmaps[13][local_pos_to_index(at)];
  }
}

ChunkMesh::ChunkMesh() {
}

rgb interpolate_foliage_color(float l_x, float l_y, rgb foliage_color_neg_x_neg_z, rgb foliage_color_neg_x_pos_z, rgb foliage_color_pos_x_neg_z, rgb foliage_color_pos_x_pos_z) {
  float t_x = l_x / 32.0f;
  float t_y = l_y / 32.0f;

  float r_neg_z = (1.0f - t_x) * foliage_color_neg_x_neg_z.r + t_x * foliage_color_pos_x_neg_z.r;
  float g_neg_z = (1.0f - t_x) * foliage_color_neg_x_neg_z.g + t_x * foliage_color_pos_x_neg_z.g;
  float b_neg_z = (1.0f - t_x) * foliage_color_neg_x_neg_z.b + t_x * foliage_color_pos_x_neg_z.b;

  float r_pos_z = (1.0f - t_x) * foliage_color_neg_x_pos_z.r + t_x * foliage_color_pos_x_pos_z.r;
  float g_pos_z = (1.0f - t_x) * foliage_color_neg_x_pos_z.g + t_x * foliage_color_pos_x_pos_z.g;
  float b_pos_z = (1.0f - t_x) * foliage_color_neg_x_pos_z.b + t_x * foliage_color_pos_x_pos_z.b;

  rgb result;
  result.r = static_cast<uint8_t>((1.0f - t_y) * r_neg_z + t_y * r_pos_z + 0.5f);
  result.g = static_cast<uint8_t>((1.0f - t_y) * g_neg_z + t_y * g_pos_z + 0.5f);
  result.b = static_cast<uint8_t>((1.0f - t_y) * b_neg_z + t_y * b_pos_z + 0.5f);

  return result;
}

ChunkMesh::ChunkMesh(std::unique_ptr<ChunkMeshData> data) {
  BENCHMARK("chunk meshing");
  const ChunkPos chunk_pos = data->get_chunk_pos();

  NeigbourCubeIds neigbour_cube_ids{};

  for (size_t i = 0; i < (size_t)Chunk::chunk_cube_count; i += 1) {
    LocalPos l = index_to_local_pos(i);
    if (data->get_cube_id(l, 13) == CubeId::AIR) { continue; }
    bool is_edge = l.x == 0 || l.x == Chunk::chunk_size - 1 || l.y == 0 || l.y == Chunk::chunk_size - 1 || l.z == 0 || l.z == Chunk::chunk_size - 1;
    auto& cube = cubes_get(data->get_cube_id(l, false));

    rgb foliage_color = interpolate_foliage_color(l.x, l.z, data->foliage_color_neg_x_neg_z, data->foliage_color_neg_x_pos_z, data->foliage_color_pos_x_neg_z, data->foliage_color_pos_x_pos_z);

    neigbour_cube_ids.center = data->get_cube_id(LocalPos{l.x, l.y, l.z}, false);
    // Straight neigbour_cube_ids
    neigbour_cube_ids.left = data->get_cube_id(l + LocalPos{-1, +0, +0}, is_edge);
    neigbour_cube_ids.right = data->get_cube_id(l + LocalPos{+1, +0, +0}, is_edge);
    neigbour_cube_ids.bottom = data->get_cube_id(l + LocalPos{+0, -1, +0}, is_edge);
    neigbour_cube_ids.top = data->get_cube_id(l + LocalPos{+0, +1, +0}, is_edge);
    neigbour_cube_ids.front = data->get_cube_id(l + LocalPos{+0, +0, -1}, is_edge);
    neigbour_cube_ids.back = data->get_cube_id(l + LocalPos{+0, +0, +1}, is_edge);

    // Edge neighbours
    neigbour_cube_ids.left_bottom = data->get_cube_id(l + LocalPos{-1, -1, +0}, is_edge);
    neigbour_cube_ids.right_bottom = data->get_cube_id(l + LocalPos{+1, -1, +0}, is_edge);
    neigbour_cube_ids.front_bottom = data->get_cube_id(l + LocalPos{+0, -1, -1}, is_edge);
    neigbour_cube_ids.back_bottom = data->get_cube_id(l + LocalPos{+0, -1, +1}, is_edge);

    neigbour_cube_ids.left_top = data->get_cube_id(l + LocalPos{-1, +1, +0}, is_edge);
    neigbour_cube_ids.right_top = data->get_cube_id(l + LocalPos{+1, +1, +0}, is_edge);
    neigbour_cube_ids.front_top = data->get_cube_id(l + LocalPos{+0, +1, -1}, is_edge);
    neigbour_cube_ids.back_top = data->get_cube_id(l + LocalPos{+0, +1, +1}, is_edge);

    neigbour_cube_ids.left_front = data->get_cube_id(l + LocalPos{-1, +0, -1}, is_edge);
    neigbour_cube_ids.right_front = data->get_cube_id(l + LocalPos{+1, +0, -1}, is_edge);
    neigbour_cube_ids.left_back = data->get_cube_id(l + LocalPos{-1, +0, +1}, is_edge);
    neigbour_cube_ids.right_back = data->get_cube_id(l + LocalPos{+1, +0, +1}, is_edge);

    // Corner neighbours
    neigbour_cube_ids.left_bottom_front = data->get_cube_id(l + LocalPos{-1, -1, -1}, is_edge);
    neigbour_cube_ids.left_bottom_back = data->get_cube_id(l + LocalPos{-1, -1, +1}, is_edge);
    neigbour_cube_ids.left_top_front = data->get_cube_id(l + LocalPos{-1, +1, -1}, is_edge);
    neigbour_cube_ids.left_top_back = data->get_cube_id(l + LocalPos{-1, +1, +1}, is_edge);
    neigbour_cube_ids.right_bottom_front = data->get_cube_id(l + LocalPos{+1, -1, -1}, is_edge);
    neigbour_cube_ids.right_bottom_back = data->get_cube_id(l + LocalPos{+1, -1, +1}, is_edge);
    neigbour_cube_ids.right_top_front = data->get_cube_id(l + LocalPos{+1, +1, -1}, is_edge);
    neigbour_cube_ids.right_top_back = data->get_cube_id(l + LocalPos{+1, +1, +1}, is_edge);

    // Brightness
    neigbour_cube_ids.brightness = data->get_lightmap(l, false);
    neigbour_cube_ids.brightness_left = data->get_lightmap(l + LocalPos{-1, +0, +0}, is_edge);
    neigbour_cube_ids.brightness_right = data->get_lightmap(l + LocalPos{+1, +0, +0}, is_edge);
    neigbour_cube_ids.brightness_bottom = data->get_lightmap(l + LocalPos{+0, -1, +0}, is_edge);
    neigbour_cube_ids.brightness_top = data->get_lightmap(l + LocalPos{+0, +1, +0}, is_edge);
    neigbour_cube_ids.brightness_front = data->get_lightmap(l + LocalPos{+0, +0, -1}, is_edge);
    neigbour_cube_ids.brightness_back = data->get_lightmap(l + LocalPos{+0, +0, +1}, is_edge);

    if (cube.draw_data.is_translucent) {
      cube.get_vertices(chunk_pos * Chunk::chunk_size + l, neigbour_cube_ids, std::nullopt, foliage_color, vertices_translucent);
    } else {
      cube.get_vertices(chunk_pos * Chunk::chunk_size + l, neigbour_cube_ids, std::nullopt, foliage_color, vertices);
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

    // Bind foliage color
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(CompactVertex), (void*)offsetof(CompactVertex, foliage_color));

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

    // Bind foliage color
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(CompactVertex), (void*)offsetof(CompactVertex, foliage_color));

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
