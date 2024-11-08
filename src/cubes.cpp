#include "cubes.hpp"
#include "aabb.hpp"
#include "world.hpp"
#include "world_renderer.hpp"

using CompactVertex = cubes::CompactVertex;
using CompactVertexNormal = cubes::CompactVertexNormal;

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

cubes::cubes() {
  using enum Cube::CubeOccludeMode;
  cube_array[0] = Cube{.name = "Air"};
  cube_array[0].set_occlusion_mode(NEVER);

  cube_array[1] = create_full_cube_with_single_uv("Dirt", {1.0f, 0.0f});

  cube_array[2] = create_full_cube_with_per_face_uv("Grass",
                                                    {2.0f, 0.0f}, {2.0f, 0.0f},
                                                    {1.0f, 0.0f}, {3.0f, 0.0f},
                                                    {2.0f, 0.0f}, {2.0f, 0.0f});

  cube_array[3] = create_full_cube_with_single_uv("Stone", {4.0f, 0.0f});

  cube_array[4] = create_full_cube_with_single_uv("Sand", {5.0f, 0.0f});

  cube_array[5] = create_full_cube_with_single_uv("Gravel", {6.0f, 0.0f});

  cube_array[6] = create_full_cube_with_per_face_uv("Wood",
                                                    {7.0f, 0.0f}, {7.0f, 0.0f},
                                                    {8.0f, 0.0f}, {8.0f, 0.0f},
                                                    {7.0f, 0.0f}, {7.0f, 0.0f});

  cube_array[7] = create_full_cube_with_single_uv("Leaves", {9.0f, 0.0f});
  cube_array[7].set_occlusion_mode(NEVER).set_ao(false);

  cube_array[8] = create_x_shape_cube("Grass Plant", {13.0f, 0.0f});
  cube_array[9] = create_full_cube_with_single_uv("Water", {0.0f, 1.0f});

  cube_array[9].set_occlusion_mode(IF_SAME_ID).set_is_translucent(true).set_ao(false);

  cube_array[10] = create_full_cube_with_single_uv("Stone Bricks", {10.0f, 0.0f});
  cube_array[11] = create_full_cube_with_single_uv("Stone Bricks", {11.0f, 0.0f});
  cube_array[12] = create_full_cube_with_single_uv("Stone Tiles", {12.0f, 0.0f});
}

constexpr cubes::Cube cubes::create_full_cube_with_single_uv(
    std::string name, glm::vec<2, float> uv,
    bool is_translucent, Cube::CubeOccludeMode occlude_mode) {

  return create_full_cube_with_per_face_uv(
      name, uv, uv, uv, uv, uv, uv,
      is_translucent, occlude_mode);
}

constexpr cubes::Cube cubes::create_full_cube_with_per_face_uv(
    std::string name,
    glm::vec<2, float> uv_left, glm::vec<2, float> uv_right,
    glm::vec<2, float> uv_bottom, glm::vec<2, float> uv_top,
    glm::vec<2, float> uv_front, glm::vec<2, float> uv_back,
    bool is_translucent, Cube::CubeOccludeMode occlude_mode) {

  using enum Cube::CubeOccludeMode;
  auto cube = Cube{
      .name = name,
      .draw_data{
          .is_translucent = is_translucent,
          .occlude_adjacent_cube{

              .left = occlude_mode,
              .right = occlude_mode,
              .bottom = occlude_mode,
              .top = occlude_mode,
              .front = occlude_mode,
              .back = occlude_mode,
          },
          .vertices{},
      },
  };

  // Push vertices with proper UV coordinates
  for (size_t i = 0; i < 6; i += 1) {
    auto vertex_left = full_cube_vertices::left[i];
    vertex_left.push_uv(uv_left.x, uv_left.y);
    cube.draw_data.vertices.left.emplace_back(vertex_left);

    auto vertex_right = full_cube_vertices::right[i];
    vertex_right.push_uv(uv_right.x, uv_right.y);
    cube.draw_data.vertices.right.emplace_back(vertex_right);

    auto vertex_bottom = full_cube_vertices::bottom[i];
    vertex_bottom.push_uv(uv_bottom.x, uv_bottom.y);
    cube.draw_data.vertices.bottom.emplace_back(vertex_bottom);

    auto vertex_top = full_cube_vertices::top[i];
    vertex_top.push_uv(uv_top.x, uv_top.y);
    cube.draw_data.vertices.top.emplace_back(vertex_top);

    auto vertex_front = full_cube_vertices::front[i];
    vertex_front.push_uv(uv_front.x, uv_front.y);
    cube.draw_data.vertices.front.emplace_back(vertex_front);

    auto vertex_back = full_cube_vertices::back[i];
    vertex_back.push_uv(uv_back.x, uv_back.y);
    cube.draw_data.vertices.back.emplace_back(vertex_back);
  }

  cube.collider_aabbs = {AABB{{0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}}};

  return cube;
}

constexpr cubes::Cube cubes::create_x_shape_cube(std::string name, glm::vec<2, float> uv) {
  using enum Cube::CubeOccludeMode;
  using enum CompactVertexNormal;

  cubes::Cube cube{
      .name = name,
      .draw_data{
          .occlude_adjacent_cube{
              .left = NEVER,
              .right = NEVER,
              .bottom = NEVER,
              .top = NEVER,
              .front = NEVER,
              .back = NEVER,
          },
          .ao = false,
      }};

  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 1.0, 1.0}, 1.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({0.0, 0.0, 1.0}, 1.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 0.0, 0.0}, 0.0 + uv.x, 1.0 + uv.y, RIGHT_FACE));
  cube.draw_data.vertices.vertices.emplace_back(CompactVertex({1.0, 1.0, 0.0}, 0.0 + uv.x, 0.0 + uv.y, RIGHT_FACE));

  return cube;
};

void cubes::Cube::get_vertices(CubePos cube_pos, NeigbourCubeIds neigbour_cube_ids, std::vector<cubes::CompactVertex>& vertices_list) const {

  CubeId cube_id = neigbour_cube_ids.center.value();

  bool draw_left_face = [&] -> bool {
    CubeId left_cube_id = neigbour_cube_ids.left.value();
    auto& left_cube = cubes::get(left_cube_id);
    if (left_cube.draw_data.occlude_adjacent_cube.right == cubes::Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (left_cube.draw_data.occlude_adjacent_cube.right == cubes::Cube::CubeOccludeMode::IF_SAME_ID && left_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_right_face = [&] -> bool {
    CubeId right_cube_id = neigbour_cube_ids.right.value();
    auto& right_cube = cubes::get(right_cube_id);
    if (right_cube.draw_data.occlude_adjacent_cube.left == cubes::Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (right_cube.draw_data.occlude_adjacent_cube.left == cubes::Cube::CubeOccludeMode::IF_SAME_ID && right_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_bottom_face = [&] -> bool {
    CubeId bottom_cube_id = neigbour_cube_ids.bottom.value();
    auto& bottom_cube = cubes::get(bottom_cube_id);
    if (bottom_cube.draw_data.occlude_adjacent_cube.top == cubes::Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (bottom_cube.draw_data.occlude_adjacent_cube.top == cubes::Cube::CubeOccludeMode::IF_SAME_ID && bottom_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_top_face = [&] -> bool {
    CubeId top_cube_id = neigbour_cube_ids.top.value();
    auto& top_cube = cubes::get(top_cube_id);
    if (top_cube.draw_data.occlude_adjacent_cube.bottom == cubes::Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (top_cube.draw_data.occlude_adjacent_cube.bottom == cubes::Cube::CubeOccludeMode::IF_SAME_ID && top_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_front_face = [&] -> bool {
    CubeId front_cube_id = neigbour_cube_ids.front.value();
    auto& front_cube = cubes::get(front_cube_id);
    if (front_cube.draw_data.occlude_adjacent_cube.back == cubes::Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (front_cube.draw_data.occlude_adjacent_cube.back == cubes::Cube::CubeOccludeMode::IF_SAME_ID && front_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  bool draw_back_face = [&] -> bool {
    CubeId back_cube_id = neigbour_cube_ids.back.value();
    auto& back_cube = cubes::get(back_cube_id);
    if (back_cube.draw_data.occlude_adjacent_cube.front == cubes::Cube::CubeOccludeMode::ALWAYS) {
      return false;
    }

    if (back_cube.draw_data.occlude_adjacent_cube.front == cubes::Cube::CubeOccludeMode::IF_SAME_ID && back_cube_id == cube_id) {
      return false;
    }

    return true;
  }();

  auto reduce_vertex_brightness_for_ao = [](cubes::CompactVertex& vertex, std::optional<float> x, std::optional<float> y, std::optional<float> z, const std::optional<CubeId>& neigbour) {
    if constexpr (WorldRenderer::ambient_occlusion_enabled) {
      if (vertex.pos.x == x.value_or(vertex.pos.x) && vertex.pos.y == y.value_or(vertex.pos.y) && vertex.pos.z == z.value_or(vertex.pos.z) &&
          neigbour.value_or(CubeId::AIR) != CubeId::AIR) {
        bool ao = cubes::get(neigbour.value_or(CubeId::AIR)).draw_data.ao;
        if (ao) {
          vertex.pack.brightness = 160;
        }
      }
    }
  };

  for (auto vertex : draw_data.vertices.vertices) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  if (draw_left_face) {
    for (auto vertex : draw_data.vertices.left) {
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
    for (auto vertex : draw_data.vertices.right) {
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
    for (auto vertex : draw_data.vertices.bottom) {
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
    for (auto vertex : draw_data.vertices.top) {
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
    for (auto vertex : draw_data.vertices.front) {
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
    for (auto vertex : draw_data.vertices.back) {
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

void cubes::Cube::get_vertices(CubePos cube_pos, std::vector<cubes::CompactVertex>& vertices_list) const {
  for (auto vertex : draw_data.vertices.vertices) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices.left) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices.right) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices.bottom) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices.top) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices.front) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }

  for (auto vertex : draw_data.vertices.back) {
    vertex.pos += cube_pos;
    vertices_list.emplace_back(vertex);
  }
}
