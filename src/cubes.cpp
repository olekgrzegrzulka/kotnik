#include "cubes.hpp"
#include <vector>
#include "common.hpp"
#include "world.hpp"
#include "world_renderer.hpp"

static std::vector<Cube> cube_array;

namespace full_cube_vertices {
static constexpr std::array<float, 6> u = {0.0, 0.0, 1.0, 1.0, 1.0, 0.0};

static constexpr std::array<float, 6> v = {0.0, 1.0, 1.0, 1.0, 0.0, 0.0};

static const std::array<CompactVertex, 6> left = {
    CompactVertex({0.0, +1.0, 0.0}, u[0], v[0], CompactVertexNormal::LEFT_FACE),
    CompactVertex({0.0, 0.0, 0.0}, u[1], v[1], CompactVertexNormal::LEFT_FACE),
    CompactVertex({0.0, 0.0, +1.0}, u[2], v[2], CompactVertexNormal::LEFT_FACE),
    CompactVertex({0.0, 0.0, +1.0}, u[3], v[3], CompactVertexNormal::LEFT_FACE),
    CompactVertex({0.0, +1.0, +1.0}, u[4], v[4], CompactVertexNormal::LEFT_FACE),
    CompactVertex({0.0, +1.0, 0.0}, u[5], v[5], CompactVertexNormal::LEFT_FACE),
};

static const std::vector<CompactVertex> right = {
    CompactVertex({+1.0, +1.0, +1.0}, u[0], v[0], CompactVertexNormal::RIGHT_FACE),
    CompactVertex({+1.0, 0.00, +1.0}, u[1], v[1], CompactVertexNormal::RIGHT_FACE),
    CompactVertex({+1.0, 0.00, 0.00}, u[2], v[2], CompactVertexNormal::RIGHT_FACE),
    CompactVertex({+1.0, 0.00, 0.00}, u[3], v[3], CompactVertexNormal::RIGHT_FACE),
    CompactVertex({+1.0, +1.0, 0.00}, u[4], v[4], CompactVertexNormal::RIGHT_FACE),
    CompactVertex({+1.0, +1.0, +1.0}, u[5], v[5], CompactVertexNormal::RIGHT_FACE),
};

static const std::vector<CompactVertex> front = {
    CompactVertex({+1.0, +1.0, 0.00}, u[0], v[0], CompactVertexNormal::FRONT_FACE),
    CompactVertex({+1.0, 0.00, 0.00}, u[1], v[1], CompactVertexNormal::FRONT_FACE),
    CompactVertex({0.00, 0.00, 0.00}, u[2], v[2], CompactVertexNormal::FRONT_FACE),
    CompactVertex({0.00, 0.00, 0.00}, u[3], v[3], CompactVertexNormal::FRONT_FACE),
    CompactVertex({0.00, +1.0, 0.00}, u[4], v[4], CompactVertexNormal::FRONT_FACE),
    CompactVertex({+1.0, +1.0, 0.00}, u[5], v[5], CompactVertexNormal::FRONT_FACE),
};

static const std::vector<CompactVertex> back = {
    CompactVertex({0.00, +1.0, +1.0}, u[0], v[0], CompactVertexNormal::BACK_FACE),
    CompactVertex({0.00, 0.00, +1.0}, u[1], v[1], CompactVertexNormal::BACK_FACE),
    CompactVertex({+1.0, 0.00, +1.0}, u[2], v[2], CompactVertexNormal::BACK_FACE),
    CompactVertex({+1.0, 0.00, +1.0}, u[3], v[3], CompactVertexNormal::BACK_FACE),
    CompactVertex({+1.0, +1.0, +1.0}, u[4], v[4], CompactVertexNormal::BACK_FACE),
    CompactVertex({0.00, +1.0, +1.0}, u[5], v[5], CompactVertexNormal::BACK_FACE),
};

static const std::vector<CompactVertex> bottom = {
    CompactVertex({0.00, 0.00, 0.00}, u[0], v[0], CompactVertexNormal::BOTTOM_FACE),
    CompactVertex({+1.0, 0.00, 0.00}, u[1], v[1], CompactVertexNormal::BOTTOM_FACE),
    CompactVertex({+1.0, 0.00, +1.0}, u[2], v[2], CompactVertexNormal::BOTTOM_FACE),
    CompactVertex({+1.0, 0.00, +1.0}, u[3], v[3], CompactVertexNormal::BOTTOM_FACE),
    CompactVertex({0.00, 0.00, +1.0}, u[4], v[4], CompactVertexNormal::BOTTOM_FACE),
    CompactVertex({0.00, 0.00, 0.00}, u[5], v[5], CompactVertexNormal::BOTTOM_FACE),
};

static const std::vector<CompactVertex> top = {
    CompactVertex({+1.0, +1.0, +1.0}, u[0], v[0], CompactVertexNormal::TOP_FACE),
    CompactVertex({+1.0, +1.0, 0.00}, u[1], v[1], CompactVertexNormal::TOP_FACE),
    CompactVertex({0.00, +1.0, 0.00}, u[2], v[2], CompactVertexNormal::TOP_FACE),
    CompactVertex({0.00, +1.0, 0.00}, u[3], v[3], CompactVertexNormal::TOP_FACE),
    CompactVertex({0.00, +1.0, +1.0}, u[4], v[4], CompactVertexNormal::TOP_FACE),
    CompactVertex({+1.0, +1.0, +1.0}, u[5], v[5], CompactVertexNormal::TOP_FACE),
};
}; // namespace full_cube_vertices
void cubes_init() {
  using enum Cube::CubeOccludeMode;

  cube_array.emplace_back(Cube{"Air"});
  cube_array[0].set_occlusion_mode(NEVER);

  cube_array.emplace_back(Cube{"Dirt"});
  cube_array[1].add_model_full_cube({1.0f, 0.0f}).add_collider();

  cube_array.emplace_back(Cube{"Grass"});
  cube_array[2]
      .add_collider()
      .add_model_full_cube(
          {2.0f, 0.0f}, {2.0f, 0.0f},
          {1.0f, 0.0f}, {3.0f, 0.0f},
          {2.0f, 0.0f}, {2.0f, 0.0f});

  cube_array.emplace_back(Cube{"Stone"});
  cube_array[3].add_model_full_cube({4.0f, 0.0f}).add_collider();

  cube_array.emplace_back(Cube{"Sand"});
  cube_array[4].add_model_full_cube({5.0f, 0.0f}).add_collider();

  cube_array.emplace_back(Cube{"Gravel"});
  cube_array[5].add_model_full_cube({6.0f, 0.0f}).add_collider();

  cube_array.emplace_back(Cube{"Wood"});
  cube_array[6].add_collider().add_model_full_cube(
      {7.0f, 0.0f}, {7.0f, 0.0f},
      {8.0f, 0.0f}, {8.0f, 0.0f},
      {7.0f, 0.0f}, {7.0f, 0.0f});

  cube_array.emplace_back(Cube{"Birch Wood"});
  cube_array[7].add_collider().add_model_full_cube(
      {7.0f, 1.0f}, {7.0f, 1.0f},
      {8.0f, 1.0f}, {8.0f, 1.0f},
      {7.0f, 1.0f}, {7.0f, 1.0f});

  cube_array.emplace_back(Cube{"Leaves"});
  cube_array[8].add_model_full_cube({9.0f, 0.0f}).add_collider();
  cube_array[8].set_ao(Cube::CubeAOMode::IF_SAME_ID).set_occlusion_mode(Cube::CubeOccludeMode::NEVER);

  cube_array.emplace_back(Cube{"Grass Plant"});
  cube_array[9].set_occlusion_mode(NEVER).set_ao(Cube::CubeAOMode::NEVER);
  cube_array[9].add_model_x_shape({13.0f, 0.0f}).add_model_x_shape({13.0f, 1.0f});
  cube_array[9].set_occlusion_mode(Cube::CubeOccludeMode::NEVER);

  cube_array.emplace_back(Cube{"Flower"});
  cube_array[10].add_model_x_shape({14.0f, 0.0f}).add_model_x_shape({15.0f, 0.0f});
  cube_array[10].add_model_x_shape({14.0f, 1.0f}).add_model_x_shape({15.0f, 1.0f});
  cube_array[10].set_ao(Cube::CubeAOMode::NEVER).set_occlusion_mode(Cube::CubeOccludeMode::NEVER);

  cube_array.emplace_back(Cube{"Water"}).add_model_full_cube({0.0f, 1.0f}).add_collider();
  cube_array[11].set_is_translucent(true).set_ao(Cube::CubeAOMode::NEVER);
  cube_array[11].set_occlusion_mode(Cube::CubeOccludeMode::IF_SAME_ID);

  cube_array.emplace_back(Cube{"Stone Bricks"});
  cube_array[12].add_model_full_cube({10.0f, 0.0f}).add_collider();
  cube_array.emplace_back(Cube{"Stone Bricks"});

  cube_array[13].add_model_full_cube({11.0f, 0.0f}).add_collider();

  cube_array.emplace_back(Cube{"Stone Tiles"});
  cube_array[14].add_model_full_cube({12.0f, 0.0f}).add_collider();
}

const Cube& cubes_get(CubeId cube_id) {
  ensure(!cube_array.empty());
  ensure(cube_id < CubeId::CUBE_ID_SIZE);
  return cube_array[(size_t)cube_id];
}

constexpr Cube& Cube::add_model_full_cube(glm::vec<2, float> uv) {
  return add_model_full_cube(uv, uv, uv, uv, uv, uv);
}

constexpr Cube& Cube::add_model_full_cube(
    glm::vec<2, float> uv_left, glm::vec<2, float> uv_right,
    glm::vec<2, float> uv_bottom, glm::vec<2, float> uv_top,
    glm::vec<2, float> uv_front, glm::vec<2, float> uv_back) {

  // Push vertices with proper UV coordinates
  draw_data.vertices.emplace_back(Cube::CubeVertices{});
  size_t i = draw_data.vertices.size() - 1;
  for (size_t j = 0; j < 6; j += 1) {
    auto vertex_left = full_cube_vertices::left[j];
    vertex_left.push_uv(uv_left.x, uv_left.y);
    draw_data.vertices[i].left.emplace_back(vertex_left);

    auto vertex_right = full_cube_vertices::right[j];
    vertex_right.push_uv(uv_right.x, uv_right.y);
    draw_data.vertices[i].right.emplace_back(vertex_right);

    auto vertex_bottom = full_cube_vertices::bottom[j];
    vertex_bottom.push_uv(uv_bottom.x, uv_bottom.y);
    draw_data.vertices[i].bottom.emplace_back(vertex_bottom);

    auto vertex_top = full_cube_vertices::top[j];
    vertex_top.push_uv(uv_top.x, uv_top.y);
    draw_data.vertices[i].top.emplace_back(vertex_top);

    auto vertex_front = full_cube_vertices::front[j];
    vertex_front.push_uv(uv_front.x, uv_front.y);
    draw_data.vertices[i].front.emplace_back(vertex_front);

    auto vertex_back = full_cube_vertices::back[j];
    vertex_back.push_uv(uv_back.x, uv_back.y);
    draw_data.vertices[i].back.emplace_back(vertex_back);
  }

  return *this;
}

constexpr Cube& Cube::add_model_x_shape(glm::vec<2, float> uv) {
  draw_data.vertices.emplace_back(Cube::CubeVertices{});
  size_t i = draw_data.vertices.size() - 1;

  using enum CompactVertexNormal;

  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  draw_data.vertices[i].vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  return *this;
};

void Cube::get_vertices(CubePos cube_pos, NeigbourCubeIds& neigbour_cube_ids, std::optional<i32> rng_opt, std::vector<CompactVertex>& vertices_list) const {
  if (draw_data.vertices.empty()) { return; }
  CubeId cube_id = neigbour_cube_ids.center.value();

  static FastNoiseLite noise_rng;
  i32 rng = 0;
  if (rng_opt.has_value()) {
    rng = rng_opt.value();
  } else if (draw_data.uses_rng()) {
    rng = noise_rng.GetNoise((float)cube_pos.x, (float)cube_pos.y, (float)cube_pos.z) * 100000.0;
  }

  size_t vertices_index = std::abs(rng) % draw_data.vertices.size();

  bool draw_left_face = [&]() -> bool {
    CubeId left_cube_id = neigbour_cube_ids.left.value();
    auto& left_cube = cubes_get(left_cube_id);
    if (left_cube.draw_data.occlude_adjacent_cube.right == Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (left_cube.draw_data.occlude_adjacent_cube.right == Cube::CubeOccludeMode::IF_SAME_ID && left_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_right_face = [&]() -> bool {
    CubeId right_cube_id = neigbour_cube_ids.right.value();
    auto& right_cube = cubes_get(right_cube_id);
    if (right_cube.draw_data.occlude_adjacent_cube.left == Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (right_cube.draw_data.occlude_adjacent_cube.left == Cube::CubeOccludeMode::IF_SAME_ID && right_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_bottom_face = [&]() -> bool {
    CubeId bottom_cube_id = neigbour_cube_ids.bottom.value();
    auto& bottom_cube = cubes_get(bottom_cube_id);
    if (bottom_cube.draw_data.occlude_adjacent_cube.top == Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (bottom_cube.draw_data.occlude_adjacent_cube.top == Cube::CubeOccludeMode::IF_SAME_ID && bottom_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_top_face = [&]() -> bool {
    CubeId top_cube_id = neigbour_cube_ids.top.value();
    auto& top_cube = cubes_get(top_cube_id);
    if (top_cube.draw_data.occlude_adjacent_cube.bottom == Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (top_cube.draw_data.occlude_adjacent_cube.bottom == Cube::CubeOccludeMode::IF_SAME_ID && top_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_front_face = [&]() -> bool {
    CubeId front_cube_id = neigbour_cube_ids.front.value();
    auto& front_cube = cubes_get(front_cube_id);
    if (front_cube.draw_data.occlude_adjacent_cube.back == Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (front_cube.draw_data.occlude_adjacent_cube.back == Cube::CubeOccludeMode::IF_SAME_ID && front_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_back_face = [&]() -> bool {
    CubeId back_cube_id = neigbour_cube_ids.back.value();
    auto& back_cube = cubes_get(back_cube_id);
    if (back_cube.draw_data.occlude_adjacent_cube.front == Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (back_cube.draw_data.occlude_adjacent_cube.front == Cube::CubeOccludeMode::IF_SAME_ID && back_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  auto reduce_vertex_brightness_for_ao = [&neigbour_cube_ids](CompactVertex& vertex, std::optional<float> x, std::optional<float> y, std::optional<float> z, const std::optional<CubeId>& neigbour) {
    if constexpr (!WorldRenderer::ambient_occlusion_enabled) { return; }

    if (vertex.pos.x == x.value_or(vertex.pos.x) && vertex.pos.y == y.value_or(vertex.pos.y) && vertex.pos.z == z.value_or(vertex.pos.z) &&
        neigbour.value_or(CubeId::AIR) != CubeId::AIR) {
      auto ao = cubes_get(neigbour.value_or(CubeId::AIR)).draw_data.ao;
      if (ao == Cube::CubeAOMode::ALWAYS) {
        vertex.pack.brightness = 255 - WorldRenderer::ambient_occlusion_intensity;
      } else if ((ao == Cube::CubeAOMode::IF_SAME_ID && neigbour_cube_ids.center.value() == neigbour.value())) {
        vertex.pack.brightness = 255 - WorldRenderer::ambient_occlusion_intensity;
      }
    }
  };

  for (auto vertex : draw_data.vertices[vertices_index].vertices) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  if (draw_left_face) {
    for (auto vertex : draw_data.vertices[vertices_index].left) {
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, 1.0, neigbour_cube_ids.left_top_back);
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, 0.0, neigbour_cube_ids.left_top_front);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, 1.0, neigbour_cube_ids.left_bottom_back);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, 0.0, neigbour_cube_ids.left_bottom_front);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 1.0, neigbour_cube_ids.left_back);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 0.0, neigbour_cube_ids.left_front);
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, {}, neigbour_cube_ids.left_top);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, {}, neigbour_cube_ids.left_bottom);

      vertex.pos += cube_pos;
      vertices_list.emplace_back(vertex);
    }
  }

  if (draw_right_face) {
    for (auto vertex : draw_data.vertices[vertices_index].right) {
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, 1.0, neigbour_cube_ids.right_top_back);
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, 0.0, neigbour_cube_ids.right_top_front);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, 1.0, neigbour_cube_ids.right_bottom_back);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, 0.0, neigbour_cube_ids.right_bottom_front);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 1.0, neigbour_cube_ids.right_back);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 0.0, neigbour_cube_ids.right_front);
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, {}, neigbour_cube_ids.right_top);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, {}, neigbour_cube_ids.right_bottom);

      vertex.pos += cube_pos;
      vertices_list.emplace_back(vertex);
    }
  }

  if (draw_bottom_face) {
    for (auto vertex : draw_data.vertices[vertices_index].bottom) {
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, 0.0, neigbour_cube_ids.left_bottom_front);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, 0.0, neigbour_cube_ids.right_bottom_front);
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, 1.0, neigbour_cube_ids.left_bottom_back);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, 1.0, neigbour_cube_ids.right_bottom_back);
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, {}, neigbour_cube_ids.left_bottom);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, {}, neigbour_cube_ids.right_bottom);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 0.0, neigbour_cube_ids.front_bottom);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 1.0, neigbour_cube_ids.back_bottom);

      vertex.pos += cube_pos;
      vertices_list.emplace_back(vertex);
    }
  }

  if (draw_top_face) {
    for (auto vertex : draw_data.vertices[vertices_index].top) {
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, 0.0, neigbour_cube_ids.left_top_front);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, 0.0, neigbour_cube_ids.right_top_front);
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, 1.0, neigbour_cube_ids.left_top_back);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, 1.0, neigbour_cube_ids.right_top_back);
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, {}, neigbour_cube_ids.left_top);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, {}, neigbour_cube_ids.right_top);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 0.0, neigbour_cube_ids.front_top);
      reduce_vertex_brightness_for_ao(vertex, {}, {}, 1.0, neigbour_cube_ids.back_top);

      vertex.pos += cube_pos;
      vertices_list.emplace_back(vertex);
    }
  }

  if (draw_front_face) {
    for (auto vertex : draw_data.vertices[vertices_index].front) {
      reduce_vertex_brightness_for_ao(vertex, 0.0, 1.0, {}, neigbour_cube_ids.left_top_front);
      reduce_vertex_brightness_for_ao(vertex, 1.0, 1.0, {}, neigbour_cube_ids.right_top_front);
      reduce_vertex_brightness_for_ao(vertex, 0.0, 0.0, {}, neigbour_cube_ids.left_bottom_front);
      reduce_vertex_brightness_for_ao(vertex, 1.0, 0.0, {}, neigbour_cube_ids.right_bottom_front);
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, {}, neigbour_cube_ids.left_front);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, {}, neigbour_cube_ids.right_front);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, {}, neigbour_cube_ids.front_bottom);
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, {}, neigbour_cube_ids.front_top);

      vertex.pos += cube_pos;
      vertices_list.emplace_back(vertex);
    }
  }

  if (draw_back_face) {
    for (auto vertex : draw_data.vertices[vertices_index].back) {
      reduce_vertex_brightness_for_ao(vertex, 0.0, 1.0, {}, neigbour_cube_ids.left_top_back);
      reduce_vertex_brightness_for_ao(vertex, 1.0, 1.0, {}, neigbour_cube_ids.right_top_back);
      reduce_vertex_brightness_for_ao(vertex, 0.0, 0.0, {}, neigbour_cube_ids.left_bottom_back);
      reduce_vertex_brightness_for_ao(vertex, 1.0, 0.0, {}, neigbour_cube_ids.right_bottom_back);
      reduce_vertex_brightness_for_ao(vertex, 0.0, {}, {}, neigbour_cube_ids.left_back);
      reduce_vertex_brightness_for_ao(vertex, 1.0, {}, {}, neigbour_cube_ids.right_back);
      reduce_vertex_brightness_for_ao(vertex, {}, 0.0, {}, neigbour_cube_ids.back_bottom);
      reduce_vertex_brightness_for_ao(vertex, {}, 1.0, {}, neigbour_cube_ids.back_top);

      vertex.pos += cube_pos;
      vertices_list.emplace_back(vertex);
    }
  }
}

void Cube::get_vertices(CubePos cube_pos, i32 rng, std::vector<CompactVertex>& vertices_list) const {
  if (draw_data.vertices.empty()) { return; }
  if (draw_data.vertices.size() == 0) { return; }
  size_t vertices_index = std::abs(rng) % draw_data.vertices.size();
  for (auto vertex : draw_data.vertices[vertices_index].vertices) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices[vertices_index].left) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices[vertices_index].right) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices[vertices_index].bottom) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices[vertices_index].top) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices[vertices_index].front) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices[vertices_index].back) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }
}
