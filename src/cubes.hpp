#pragma once
#include <algorithm>
#include <array>
#include <numeric>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include "aabb.hpp"
#include "common.hpp"

#define ATLAS_SIZE (16)

enum class SparseVertexNormal : uint16_t {
  NEGATIVE = 0,
  ZERO = 1,
  POSITIVE = 2,
};

using enum SparseVertexNormal;

// Layout of 32-bit pack:
// First 8 bits: x uv, which when incremented increments actual shader uv value by 0.5
// Next  8 bits: y uv, same as above
// Next  2 bits: x normal (0b00 = -1, 0b01 = 0, 0b10 = 1, 0b11 = 1)
// Next  2 bits: y normal, same as above
// Next  2 bits: z normal, same as above
// Next  8 bits: brightness
// Next  2 bits: unused :)
struct SparseVertex {
  constexpr SparseVertex(glm::vec<3, float> _pos, std::array<SparseVertexNormal, 3> normal) : pos(_pos) {
    pack |= ((0b11 & ((uint16_t)normal[0])) << 16);
    pack |= ((0b11 & ((uint16_t)normal[1])) << 18);
    pack |= ((0b11 & ((uint16_t)normal[2])) << 20);
  }

  glm::vec<3, float> pos;
  alignas(4) uint32_t pack{};
};

static_assert(sizeof(float) == 4);

static constexpr std::array<glm::vec2, 6> uvs = {
    glm::vec2(0.0, 0.0),
    glm::vec2(0.0, 1.0),
    glm::vec2(1.0, 1.0),
    glm::vec2(1.0, 1.0),
    glm::vec2(1.0, 0.0),
    glm::vec2(0.0, 0.0),
};

static constexpr std::array<SparseVertexNormal, 3> cube_normals_left = {NEGATIVE, ZERO, ZERO};
static const std::vector<SparseVertex> cube_vertices_left = {
    SparseVertex({0.0, +1.0, 0.0}, cube_normals_left),
    SparseVertex({0.0, 0.0, 0.0}, cube_normals_left),
    SparseVertex({0.0, 0.0, +1.0}, cube_normals_left),
    SparseVertex({0.0, 0.0, +1.0}, cube_normals_left),
    SparseVertex({0.0, +1.0, +1.0}, cube_normals_left),
    SparseVertex({0.0, +1.0, 0.0}, cube_normals_left),
};

static constexpr std::array<SparseVertexNormal, 3> cube_normals_right = {POSITIVE, ZERO, ZERO};
static const std::vector<SparseVertex> cube_vertices_right = {
    SparseVertex({+1.0, +1.0, +1.0}, cube_normals_right),
    SparseVertex({+1.0, 0.0, +1.0}, cube_normals_right),
    SparseVertex({+1.0, 0.0, 0.0}, cube_normals_right),
    SparseVertex({+1.0, 0.0, 0.0}, cube_normals_right),
    SparseVertex({+1.0, +1.0, 0.0}, cube_normals_right),
    SparseVertex({+1.0, +1.0, +1.0}, cube_normals_right),
};

static constexpr std::array<SparseVertexNormal, 3> cube_normals_front = {ZERO, ZERO, NEGATIVE};
static const std::vector<SparseVertex> cube_vertices_front = {
    SparseVertex({+1.0, +1.0, 0.0}, cube_normals_front),
    SparseVertex({+1.0, 0.0, 0.0}, cube_normals_front),
    SparseVertex({0.0, 0.0, 0.0}, cube_normals_front),
    SparseVertex({0.0, 0.0, 0.0}, cube_normals_front),
    SparseVertex({0.0, +1.0, 0.0}, cube_normals_front),
    SparseVertex({+1.0, +1.0, 0.0}, cube_normals_front),
};

static constexpr std::array<SparseVertexNormal, 3> cube_normals_back = {ZERO, ZERO, POSITIVE};
static const std::vector<SparseVertex> cube_vertices_back = {
    SparseVertex({0.0, +1.0, +1.0}, cube_normals_back),
    SparseVertex({0.0, 0.0, +1.0}, cube_normals_back),
    SparseVertex({+1.0, 0.0, +1.0}, cube_normals_back),
    SparseVertex({+1.0, 0.0, +1.0}, cube_normals_back),
    SparseVertex({+1.0, +1.0, +1.0}, cube_normals_back),
    SparseVertex({0.0, +1.0, +1.0}, cube_normals_back),
};

static constexpr std::array<SparseVertexNormal, 3> cube_normals_top = {ZERO, POSITIVE, ZERO};
static const std::vector<SparseVertex> cube_vertices_top = {
    SparseVertex({+1.0, +1.0, +1.0}, cube_normals_top),
    SparseVertex({+1.0, +1.0, 0.0}, cube_normals_top),
    SparseVertex({0.0, +1.0, 0.0}, cube_normals_top),
    SparseVertex({0.0, +1.0, 0.0}, cube_normals_top),
    SparseVertex({0.0, +1.0, +1.0}, cube_normals_top),
    SparseVertex({+1.0, +1.0, +1.0}, cube_normals_top),
};

static constexpr std::array<SparseVertexNormal, 3> cube_normals_bottom = {ZERO, NEGATIVE, ZERO};
static const std::vector<SparseVertex> cube_vertices_bottom = {
    SparseVertex({0.0, 0.0, 0.0}, cube_normals_bottom),
    SparseVertex({+1.0, 0.0, 0.0}, cube_normals_bottom),
    SparseVertex({+1.0, 0.0, +1.0}, cube_normals_bottom),
    SparseVertex({+1.0, 0.0, +1.0}, cube_normals_bottom),
    SparseVertex({0.0, 0.0, +1.0}, cube_normals_bottom),
    SparseVertex({0.0, 0.0, 0.0}, cube_normals_bottom),
};

struct CubeProperties {
  struct {
    std::vector<SparseVertex> vertices_left{};
    std::vector<SparseVertex> vertices_right{};
    std::vector<SparseVertex> vertices_front{};
    std::vector<SparseVertex> vertices_back{};
    std::vector<SparseVertex> vertices_bottom{};
    std::vector<SparseVertex> vertices_top{};
    std::vector<SparseVertex> vertices_misc{};

    std::unordered_map<Dir, bool> face_solidity{
        {Dir::LEFT, true},
        {Dir::RIGHT, true},
        {Dir::FRONT, true},
        {Dir::BACK, true},
        {Dir::BOTTOM, true},
        {Dir::TOP, true},
    };

  } render_data;

  std::vector<AABB> collider_aabb;

  double friction = 1.0;
};

static CubeProperties create_foliage_cube(glm::vec<2, uint32_t> uv) {
  CubeProperties cp;

  cp.render_data.vertices_misc = {
      SparseVertex({+1.0, +1.0, +1.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({+1.0, 0.0, +1.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, 0.0, 0.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, 0.0, 0.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, +1.0, 0.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({+1.0, +1.0, +1.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, +1.0, 0.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({0.0, 0.0, 0.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({+1.0, 0.0, +1.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({+1.0, 0.0, +1.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({+1.0, +1.0, +1.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({0.0, +1.0, 0.0}, {ZERO, ZERO, POSITIVE}),

      SparseVertex({+1.0, +1.0, +0.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({+1.0, 0.0, +0.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, 0.0, 1.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, 0.0, 1.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, +1.0, 1.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({+1.0, +1.0, 0.0}, {POSITIVE, ZERO, ZERO}),
      SparseVertex({0.0, +1.0, 1.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({0.0, 0.0, 1.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({+1.0, 0.0, 0.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({+1.0, 0.0, 0.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({+1.0, +1.0, 0.0}, {ZERO, ZERO, POSITIVE}),
      SparseVertex({0.0, +1.0, 1.0}, {ZERO, ZERO, POSITIVE}),
  };

  cp.render_data.face_solidity[Dir::LEFT] = false;
  cp.render_data.face_solidity[Dir::RIGHT] = false;
  cp.render_data.face_solidity[Dir::FRONT] = false;
  cp.render_data.face_solidity[Dir::BACK] = false;
  cp.render_data.face_solidity[Dir::BOTTOM] = false;
  cp.render_data.face_solidity[Dir::TOP] = false;

  for (size_t i = 0; i < cp.render_data.vertices_misc.size(); i += 1) {
    cp.render_data.vertices_misc[i].pack |= ((uint8_t)((uvs[i % 6].x + uv.x) * 2.0f));
    cp.render_data.vertices_misc[i].pack |= ((uint8_t)((uvs[i % 6].y + uv.y) * 2.0f)) << 8;
  }

  return cp;
}

static CubeProperties create_full_cube(glm::vec<2, uint32_t> uv, bool solid = true) {
  CubeProperties cp{
      .render_data{
          .vertices_left = cube_vertices_left,
          .vertices_right = cube_vertices_right,
          .vertices_front = cube_vertices_front,
          .vertices_back = cube_vertices_back,
          .vertices_bottom = cube_vertices_bottom,
          .vertices_top = cube_vertices_top,
          .vertices_misc = {},
      },
      .friction = 1.0f,
  };

  cp.collider_aabb = {
      AABB{.half_extents = {0.5, 0.5, 0.5}, .offset = {0.0, 0.0, 0.0}},
  };

  cp.render_data.face_solidity[Dir::LEFT] = solid;
  cp.render_data.face_solidity[Dir::RIGHT] = solid;
  cp.render_data.face_solidity[Dir::FRONT] = solid;
  cp.render_data.face_solidity[Dir::BACK] = solid;
  cp.render_data.face_solidity[Dir::BOTTOM] = solid;
  cp.render_data.face_solidity[Dir::TOP] = solid;

  for (size_t i = 0; i < 6; i += 1) {
    cp.render_data.vertices_left[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    cp.render_data.vertices_left[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
    cp.render_data.vertices_right[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    cp.render_data.vertices_right[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
    cp.render_data.vertices_front[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    cp.render_data.vertices_front[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
    cp.render_data.vertices_back[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    cp.render_data.vertices_back[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
    cp.render_data.vertices_top[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    cp.render_data.vertices_top[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
    cp.render_data.vertices_bottom[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    cp.render_data.vertices_bottom[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
    // cp.render_data.vertices_misc[i].pack |= ((uint8_t)((uvs[i].x + uv.x) * 2.0f));
    // cp.render_data.vertices_misc[i].pack |= ((uint8_t)((uvs[i].y + uv.y) * 2.0f)) << 8;
  }

  return cp;
}

static CubeProperties create_half_cube(glm::vec<2, uint32_t> uv) {
  auto cp = create_full_cube(uv);

  cp.render_data.face_solidity[Dir::LEFT] = false;
  cp.render_data.face_solidity[Dir::RIGHT] = false;
  cp.render_data.face_solidity[Dir::FRONT] = false;
  cp.render_data.face_solidity[Dir::BACK] = false;
  cp.render_data.face_solidity[Dir::BOTTOM] = true;
  cp.render_data.face_solidity[Dir::TOP] = false;

  cp.collider_aabb = {
      AABB{.half_extents = {0.5, 0.25, 0.5}, .offset = {0.0, -0.25, 0.0}},
  };

  auto halve_y = [](SparseVertex& v, bool halve_uv = true) {
    if (v.pos.y == 1.0f) {
      v.pos.y = 0.5f;
      if (halve_uv) {
        uint8_t uv_y = v.pack & 0xff00;
        uv_y += 1;
        v.pack |= (uv_y) << 8;
      }
    }
  };

  std::for_each(cp.render_data.vertices_left.begin(), cp.render_data.vertices_left.end(), halve_y);
  std::for_each(cp.render_data.vertices_right.begin(), cp.render_data.vertices_right.end(), halve_y);
  std::for_each(cp.render_data.vertices_front.begin(), cp.render_data.vertices_front.end(), halve_y);
  std::for_each(cp.render_data.vertices_back.begin(), cp.render_data.vertices_back.end(), halve_y);
  std::for_each(cp.render_data.vertices_top.begin(), cp.render_data.vertices_top.end(), [&](SparseVertex& v) { halve_y(v, false); });
  return cp;
}

static CubeProperties create_full_cube(glm::vec<2, uint32_t> uv_left, glm::vec<2, uint32_t> uv_right,
                                       glm::vec<2, uint32_t> uv_front, glm::vec<2, uint32_t> uv_back,
                                       glm::vec<2, uint32_t> uv_bottom, glm::vec<2, uint32_t> uv_top) {
  CubeProperties cp{
      .render_data{
          .vertices_left = cube_vertices_left,
          .vertices_right = cube_vertices_right,
          .vertices_front = cube_vertices_front,
          .vertices_back = cube_vertices_back,
          .vertices_bottom = cube_vertices_bottom,
          .vertices_top = cube_vertices_top,
          .vertices_misc = {},
      },
      .friction = 1.0f,
  };

  cp.collider_aabb = {
      AABB{.half_extents = {0.5, 0.5, 0.5}, .offset = {0.0, 0.0, 0.0}},
  };

  for (size_t i = 0; i < 6; i += 1) {
    cp.render_data.vertices_left[i].pack |= ((uint8_t)((uvs[i].x + uv_left.x) * 2.0f));
    cp.render_data.vertices_left[i].pack |= ((uint8_t)((uvs[i].y + uv_left.y) * 2.0f)) << 8;
    cp.render_data.vertices_right[i].pack |= ((uint8_t)((uvs[i].x + uv_right.x) * 2.0f));
    cp.render_data.vertices_right[i].pack |= ((uint8_t)((uvs[i].y + uv_right.y) * 2.0f)) << 8;
    cp.render_data.vertices_front[i].pack |= ((uint8_t)((uvs[i].x + uv_front.x) * 2.0f));
    cp.render_data.vertices_front[i].pack |= ((uint8_t)((uvs[i].y + uv_front.y) * 2.0f)) << 8;
    cp.render_data.vertices_back[i].pack |= ((uint8_t)((uvs[i].x + uv_back.x) * 2.0f));
    cp.render_data.vertices_back[i].pack |= ((uint8_t)((uvs[i].y + uv_back.y) * 2.0f)) << 8;
    cp.render_data.vertices_top[i].pack |= ((uint8_t)((uvs[i].x + uv_top.x) * 2.0f));
    cp.render_data.vertices_top[i].pack |= ((uint8_t)((uvs[i].y + uv_top.y) * 2.0f)) << 8;
    cp.render_data.vertices_bottom[i].pack |= ((uint8_t)((uvs[i].x + uv_bottom.x) * 2.0f));
    cp.render_data.vertices_bottom[i].pack |= ((uint8_t)((uvs[i].y + uv_bottom.y) * 2.0f)) << 8;
  }

  return cp;
}

static const std::array<CubeProperties, static_cast<size_t>(CubeId::CUBE_ID_SIZE)> cube_properties{
    /* Air */ CubeProperties(),
    /* Dirt */ create_full_cube({1, 0}),
    /* Grass */ create_full_cube({2, 0}, {2, 0}, {2, 0}, {2, 0}, {1, 0}, {3, 0}),
    /* Stone */ create_full_cube({4, 0}),
    /* Sand */ create_full_cube({5, 0}),
    /* Gravel */ create_full_cube({6, 0}),
    /* Wood */ create_full_cube({7, 0}, {7, 0}, {7, 0}, {7, 0}, {8, 0}, {8, 0}),
    /* Leaves */ create_full_cube({9, 0}, false),
    /* Grass plant */ create_foliage_cube({13, 0}),
};

static bool has_ambient_occlusion(Dir face, const SparseVertex& vertex, const std::unordered_map<uint32_t, CubeId>& neigbours) {
  // if (cube_properties.at((size_t)neigbours.at(dir_bitmask)).render_data.face_solidity.at(Dir::TOP) == false) {
  // return false;
  // }
  const bool is_left_vertex = vertex.pos.x < 0.1f;
  const bool is_right_vertex = vertex.pos.x > 0.9f;
  const bool is_bottom_vertex = vertex.pos.y < 0.1f;
  const bool is_top_vertex = vertex.pos.y > 0.9f;
  const bool is_front_vertex = vertex.pos.z < 0.1f;
  const bool is_back_vertex = vertex.pos.z > 0.9f;

  auto check_ao = [&](std::vector<Dir> neigbour_dirs) -> bool {
    uint32_t neigbour_bitmask = std::accumulate(neigbour_dirs.begin(), neigbour_dirs.end(), 0);

    if (!neigbours.contains(neigbour_bitmask)) { return false; }

    CubeId neigbour_id = neigbours.at(neigbour_bitmask);

    if (neigbour_id == CubeId::AIR) { return false; }

    for (Dir dir : neigbour_dirs) {
      if (cube_properties.at((size_t)neigbour_id).render_data.face_solidity.at(opposite_dir(dir)) == false) {
        return false;
      }
    }

    return true;
  };

  if (is_left_vertex && is_bottom_vertex && is_front_vertex && check_ao({Dir::LEFT, Dir::BOTTOM, Dir::FRONT})) {
    return true;
  }

  if (is_right_vertex && is_bottom_vertex && is_front_vertex && check_ao({Dir::RIGHT, Dir::BOTTOM, Dir::FRONT})) {
    return true;
  }

  if (is_left_vertex && is_top_vertex && is_front_vertex && check_ao({Dir::LEFT, Dir::TOP, Dir::FRONT})) {
    return true;
  }

  if (is_right_vertex && is_top_vertex && is_front_vertex && check_ao({Dir::RIGHT, Dir::TOP, Dir::FRONT})) {
    return true;
  }

  if (is_left_vertex && is_bottom_vertex && is_back_vertex && check_ao({Dir::LEFT, Dir::BOTTOM, Dir::BACK})) {
    return true;
  }

  if (is_right_vertex && is_bottom_vertex && is_back_vertex && check_ao({Dir::RIGHT, Dir::BOTTOM, Dir::BACK})) {
    return true;
  }

  if (is_left_vertex && is_top_vertex && is_back_vertex && check_ao({Dir::LEFT, Dir::TOP, Dir::BACK})) {
    return true;
  }

  if (is_right_vertex && is_top_vertex && is_back_vertex && check_ao({Dir::RIGHT, Dir::TOP, Dir::BACK})) {
    return true;
  }

  // Edges
  if (face == Dir::TOP && is_top_vertex) {
    if (is_left_vertex && check_ao({Dir::TOP, Dir::LEFT})) {
      return true;
    }
    if (is_right_vertex && check_ao({Dir::TOP, Dir::RIGHT})) {
      return true;
    }
    if (is_back_vertex && check_ao({Dir::TOP, Dir::BACK})) {
      return true;
    }
    if (is_front_vertex && check_ao({Dir::TOP, Dir::FRONT})) {
      return true;
    }
  }

  return false;
}

static void get_vertices(std::vector<SparseVertex>& vertices, glm::vec3 offset, CubeId cube, const std::unordered_map<uint32_t, CubeId>& neigbour_ids, const std::unordered_map<Dir, LightLevel>& light_levels) {
  if (cube == CubeId::AIR) { return; }

  std::vector<Dir> face_dirs;
  std::vector<const std::vector<SparseVertex>*> faces;

  const CubeProperties* const cube_props = &cube_properties.at((size_t)cube);

  auto can_render_face = [&](Dir dir) -> bool {
    bool has_neigbour = neigbour_ids.contains(dir);
    if (!has_neigbour) { return true; }

    const auto neigbour_id = neigbour_ids.at(dir);
    assert(cube_properties.size() > (size_t)neigbour_id);

    if (neigbour_id == CubeId::AIR) { return true; }

    const auto& neigbour_props = cube_properties.at((size_t)neigbour_id);

    return !(neigbour_props.render_data.face_solidity.at(dir));
  };

  if (can_render_face(Dir::LEFT)) {
    faces.emplace_back(&cube_props->render_data.vertices_left);
    face_dirs.emplace_back(Dir::LEFT);
  }
  if (can_render_face(Dir::RIGHT)) {
    faces.emplace_back(&cube_props->render_data.vertices_right);
    face_dirs.emplace_back(Dir::RIGHT);
  }
  if (can_render_face(Dir::TOP)) {
    faces.emplace_back(&cube_props->render_data.vertices_top);
    face_dirs.emplace_back(Dir::TOP);
  }
  if (can_render_face(Dir::BOTTOM)) {
    faces.emplace_back(&cube_props->render_data.vertices_bottom);
    face_dirs.emplace_back(Dir::BOTTOM);
  }
  if (can_render_face(Dir::FRONT)) {
    faces.emplace_back(&cube_props->render_data.vertices_front);
    face_dirs.emplace_back(Dir::FRONT);
  }
  if (can_render_face(Dir::BACK)) {
    faces.emplace_back(&cube_props->render_data.vertices_back);
    face_dirs.emplace_back(Dir::BACK);
  }

  faces.emplace_back(&cube_props->render_data.vertices_misc);
  face_dirs.emplace_back(Dir::NONE);

  if (faces.empty()) {
    return;
  }

  for (size_t face_i = 0; face_i < faces.size(); face_i += 1) {
    const auto* face = faces[face_i];
    Dir face_dir = face_dirs[face_i];

    for (size_t vertex_i = 0; vertex_i < face->size(); vertex_i += 1) {
      SparseVertex vertex = (*face)[vertex_i];

      uint8_t brightness = 255;

      // Lightning
      // if (face_dir == Dir::LEFT) {
      //   brightness = light_levels.at(Dir::LEFT).x;
      // }
      // if (face_dir == Dir::RIGHT) {
      //   brightness = light_levels.at(Dir::RIGHT).x;
      // }
      // if (face_dir == Dir::FRONT) {
      //   brightness = light_levels.at(Dir::FRONT).x;
      // }
      // if (face_dir == Dir::BACK) {
      //   brightness = light_levels.at(Dir::BACK).x;
      // }
      // if (face_dir == Dir::BOTTOM) {
      //   brightness = light_levels.at(Dir::BOTTOM).x;
      // }
      // if (face_dir == Dir::TOP) {
      //   brightness = light_levels.at(Dir::TOP).x;
      // }

      // Ambient occlusion
      if (face_dir != Dir::NONE && has_ambient_occlusion(face_dir, vertex, neigbour_ids)) {
        brightness = brightness * 0.75;
      }

      vertex.pos += offset;

      // Pack light
      vertex.pack |= brightness << 22;
      vertices.emplace_back(vertex);
    }
  }
}
