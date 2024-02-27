#include "chunk_renderer.hpp"
#include <chrono>
#include <unordered_map>
#include "chunk.hpp"
#include "cubes.hpp"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define AMBIENT_OCCLUSION (true)

ChunkRenderer::ChunkRenderer(Chunk& _chunk) : chunk(_chunk) {
}

std::unordered_map<uint32_t, CubeId> get_neigbours(const Chunk& chunk, LocalPos _local_pos, bool edges, bool corners, std::vector<Chunk*> chunk_list) {
  std::unordered_map<uint32_t, CubeId> map;

  auto _get_neigbour = [&](int x, int y, int z) {
    if (!corners && x != 0 && y != 0 && z != 0) { return; }
    if (!edges) {
      if (x != 0 && y != 0 && z == 0) { return; }
      if (x != 0 && z != 0 && y == 0) { return; }
      if (y != 0 && z != 0 && x == 0) { return; }
    }

    uint32_t dir = 0;
    if (x == -1) { dir += Dir::LEFT; }
    if (x == 1) { dir += Dir::RIGHT; }
    if (y == -1) { dir += Dir::BOTTOM; }
    if (y == 1) { dir += Dir::TOP; }
    if (z == -1) { dir += Dir::FRONT; }
    if (z == 1) { dir += Dir::BACK; }

    LocalPos local_pos_neigbour = _local_pos + LocalPos{x, y, z};

    if (is_local_pos_valid(local_pos_neigbour)) {
      map.insert({dir, chunk.get_cube(local_pos_neigbour)});
    } else {
      CubePos cube_pos_neigbour = local_pos_to_cube_pos(chunk.position, local_pos_neigbour);
      for (Chunk* chunk_neigb : chunk_list) {
        auto [neigb_chunk_pos, neigb_local_pos] = cube_to_local(cube_pos_neigbour);
        if (neigb_chunk_pos == chunk_neigb->position) {
          map.insert({dir, chunk_neigb->get_cube(neigb_local_pos)});
          break;
        }
      }
    }
  };

  _get_neigbour(-1, -1, -1);
  _get_neigbour(-1, -1, 0);
  _get_neigbour(-1, -1, 1);
  _get_neigbour(-1, 0, -1);
  _get_neigbour(-1, 0, 0);
  _get_neigbour(-1, 0, 1);
  _get_neigbour(-1, 1, -1);
  _get_neigbour(-1, 1, 0);
  _get_neigbour(-1, 1, 1);

  _get_neigbour(0, -1, -1);
  _get_neigbour(0, -1, 0);
  _get_neigbour(0, -1, 1);
  _get_neigbour(0, 0, -1);
  _get_neigbour(0, 0, 1);
  _get_neigbour(0, 1, -1);
  _get_neigbour(0, 1, 0);
  _get_neigbour(0, 1, 1);

  _get_neigbour(1, -1, -1);
  _get_neigbour(1, -1, 0);
  _get_neigbour(1, -1, 1);
  _get_neigbour(1, 0, -1);
  _get_neigbour(1, 0, 0);
  _get_neigbour(1, 0, 1);
  _get_neigbour(1, 1, -1);
  _get_neigbour(1, 1, 0);
  _get_neigbour(1, 1, 1);

  return map;
}

inline static void get_cubes_visible_faces(const std::vector<const Chunk*>& chunks, Chunk& chunk, LocalPos local_pos, std::vector<SparseVertex>& vertices, std::vector<Chunk*> chunk_list) {
  assert(is_local_pos_valid(local_pos));
  if (chunk.is_cube_occluded(local_pos)) { return; }
  if (!chunk.is_solid(local_pos)) { return; }
  CubePos cube_pos = local_pos_to_cube_pos(chunk.position, local_pos);
  auto neigbours = get_neigbours(chunk, local_pos, /* edges = */ AMBIENT_OCCLUSION, /* corners = */ AMBIENT_OCCLUSION, chunk_list);
  // return;

  std::unordered_map<Dir, LightLevel> light_levels;
  light_levels[Dir::LEFT] = {0, 0, 0};   /*world.get_lightmap(cube_pos + LocalPos{-1, 0, 0});*/
  light_levels[Dir::RIGHT] = {0, 0, 0};  /*world.get_lightmap(cube_pos + LocalPos{1, 0, 0});*/
  light_levels[Dir::BOTTOM] = {0, 0, 0}; /*world.get_lightmap(cube_pos + LocalPos{0, -1, 0});*/
  light_levels[Dir::TOP] = {0, 0, 0};    /*world.get_lightmap(cube_pos + LocalPos{0, 1, 0});*/
  light_levels[Dir::FRONT] = {0, 0, 0};  /*world.get_lightmap(cube_pos + LocalPos{0, 0, -1});*/
  light_levels[Dir::BACK] = {0, 0, 0};   /*world.get_lightmap(cube_pos + LocalPos{0, 0, 1});*/

  get_vertices(vertices, cube_pos, chunk.get_cube(local_pos), neigbours, light_levels);
}

inline static void get_chunks_visible_faces(const std::vector<const Chunk*>& chunks, Chunk& chunk, std::vector<SparseVertex>& vertices, std::vector<Chunk*> chunk_list) {
  vertices.reserve(500);

  for (size_t x = 0; x < CHUNK_SIZE; x += 1) {
    for (size_t y = 0; y < CHUNK_SIZE; y += 1) {
      for (size_t z = 0; z < CHUNK_SIZE; z += 1) {
        get_cubes_visible_faces(chunks, chunk, {x, y, z}, vertices, chunk_list);
      }
    }
  }
}

void ChunkRenderer::recreate_geometry(std::vector<Chunk*> chunk_list) {
  // auto start_time = std::chrono::high_resolution_clock::now();

  if (is_running) { return; }
  is_running = true;

  assert(chunk.flags.ready);

  std::vector<const Chunk*> chunks = {};

  vertices[building_index].clear();
  get_chunks_visible_faces(chunks, chunk, vertices[building_index], chunk_list);

  is_running = false;
  can_swap_buffers = true;

  // auto end_time = std::chrono::high_resolution_clock::now();
  // auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  // std::cout << "recreate_geometry() took " << milliseconds << "." << std::endl;
}

void ChunkRenderer::swap_buffers() {
  can_swap_buffers = false;
  std::swap(building_index, drawing_index);

  // Create and bind VAO
  glGenVertexArrays(1, (GLuint*)&vaos[drawing_index]);

  glBindVertexArray(vaos[drawing_index]);

  // Create vertex VBO
  glGenBuffers(1, &vbos[drawing_index]);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[drawing_index]);
  glBufferData(GL_ARRAY_BUFFER, vertices[drawing_index].size() * sizeof(SparseVertex), vertices[drawing_index].data(), GL_STATIC_DRAW);

  // Bind vertex position
  glBindBuffer(GL_ARRAY_BUFFER, vbos[drawing_index]);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SparseVertex), (void*)offsetof(SparseVertex, pos));
  // Bind pack
  glEnableVertexAttribArray(2);
  glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(SparseVertex), (void*)offsetof(SparseVertex, pack));

  // Unbind buffers
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  vertices[building_index].clear();
}

void ChunkRenderer::draw(WorldPos camera_pos, const glm::mat4& camera_matrix) const {
  glm::vec3 light = {0.5f, 1.0f, 0.5f};
  light = glm::normalize(light);

  glBindVertexArray(vaos[drawing_index]);

  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera_matrix));
  glUniform3f(1, light.x, light.y, light.z);
  glUniform3f(2, camera_pos.x, camera_pos.y, camera_pos.z);

  glDrawArrays(GL_TRIANGLES, 0, vertices[drawing_index].size());

  glBindVertexArray(0);
}
