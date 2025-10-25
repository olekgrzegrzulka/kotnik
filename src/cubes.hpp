#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "aabb.hpp"
#include "common.hpp"

struct NeigbourCubeIds;

enum class CubeId : u16 {
  AIR = 0,
  DIRT,
  GRASS,
  STONE,
  SAND,
  GRAVEL,
  WOOD,
  WOOD_BIRCH,
  LEAVES,
  GRASS_PLANT,
  FLOWER,
  WATER,
  STONE_BRICKS,
  STONE_BRICKS2,
  STONE_TILES,

  CUBE_ID_SIZE,
};

enum class CompactVertexNormal : u8 {
  /* 0 = Negative axis
     1 = Zero
     2 = Positive axis
                00 ZZ YY XX  */
  LEFT_FACE = 0b00'01'01'00,
  RIGHT_FACE = 0b00'01'01'11,
  FRONT_FACE = 0b00'00'01'01,
  BACK_FACE = 0b00'11'01'01,
  TOP_FACE = 0b00'01'11'01,
  BOTTOM_FACE = 0b00'01'00'01,
};

struct CompactVertex {
  glm::vec<3, float> pos;
  struct {
    u8 u;
    u8 v;
    u8 normal;
    u8 brightness = 255;
  } pack;

  struct {
    u8 r{};
    u8 g{};
    u8 b{};
    u8 unused{};
  } foliage_color;

  constexpr CompactVertex(glm::vec<3, float> pos_, float u_, float v_, CompactVertexNormal normal_)
      : pos(pos_) {
    set_uv(u_, v_);
    pack.normal = static_cast<u8>(normal_);
  }

  constexpr void set_uv(float u_, float v_) {
    pack.u = static_cast<u8>(u_ / 0.5f);
    pack.v = static_cast<u8>(v_ / 0.5f);
  }

  constexpr void push_uv(float u_, float v_) {
    pack.u += static_cast<u8>(u_ / 0.5f);
    pack.v += static_cast<u8>(v_ / 0.5f);
  }
};

static_assert(sizeof(float) == 4);
static_assert(sizeof(CompactVertexNormal) == 1);
static_assert(sizeof(CompactVertex) == 20);

struct Cube {
  std::string name = "";
  std::vector<AABB> collider_aabbs;

  constexpr Cube() {}
  constexpr Cube(std::string name_) : name{name_} {}

  Cube(const Cube&) = delete;
  Cube& operator=(const Cube&) = delete;

  Cube(Cube&&) = default;
  Cube& operator=(Cube&&) = delete;

  enum class CubeOccludeMode {
    NEVER,
    ALWAYS,
    IF_SAME_ID,
  };

  enum class CubeAOMode {
    NEVER,
    ALWAYS,
    IF_SAME_ID,
  };

  struct CubeVertices {
    std::vector<CompactVertex> vertices{};

    std::vector<CompactVertex> left{};
    std::vector<CompactVertex> right{};
    std::vector<CompactVertex> bottom{};
    std::vector<CompactVertex> top{};
    std::vector<CompactVertex> front{};
    std::vector<CompactVertex> back{};
  };

  struct { // draw
    // Translucent cubes will be drawn after non-translucent cubes, sorted by their distance to camera
    bool is_translucent = false;

    bool uses_rng() const { return vertices.size() > 1; }

    struct {
      using enum CubeOccludeMode;
      CubeOccludeMode left = ALWAYS;
      CubeOccludeMode right = ALWAYS;
      CubeOccludeMode bottom = ALWAYS;
      CubeOccludeMode top = ALWAYS;
      CubeOccludeMode front = ALWAYS;
      CubeOccludeMode back = ALWAYS;
    } occlude_adjacent_cube;

    // List of vertices for each direction is used for occlusion in the given direction
    std::vector<CubeVertices> vertices;

    CubeAOMode ao = CubeAOMode::ALWAYS;
  } draw_data;

  void get_vertices(CubePos, NeigbourCubeIds&, std::optional<i32> rng, rgb foliage_color, std::vector<CompactVertex>& vertices_list) const;

  void get_vertices(CubePos, i32 rng, rgb foliage_color, std::vector<CompactVertex>& vertices_list) const;

  Cube& set_occlusion_mode(CubeOccludeMode mode) {
    using enum CubeOccludeMode;
    draw_data.occlude_adjacent_cube.left = mode;
    draw_data.occlude_adjacent_cube.right = mode;
    draw_data.occlude_adjacent_cube.bottom = mode;
    draw_data.occlude_adjacent_cube.top = mode;
    draw_data.occlude_adjacent_cube.front = mode;
    draw_data.occlude_adjacent_cube.back = mode;
    return *this;
  };

  Cube& set_ao(CubeAOMode mode) {
    draw_data.ao = mode;
    return *this;
  }

  Cube& set_is_translucent(bool state) {
    draw_data.is_translucent = state;
    return *this;
  }

  constexpr Cube& add_model_full_cube(glm::vec<2, float> uv);
  constexpr Cube& add_model_full_cube(glm::vec<2, float> uv_left, glm::vec<2, float> uv_right,
                                      glm::vec<2, float> uv_bottom, glm::vec<2, float> uv_top,
                                      glm::vec<2, float> uv_front, glm::vec<2, float> uv_back);
  constexpr Cube& add_model_x_shape(glm::vec<2, float> uv);

  constexpr Cube& add_collider(AABB aabb = AABB{{0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}}) {
    collider_aabbs.emplace_back(aabb);
    return *this;
  }
};

void cubes_init();

const Cube& cubes_get(CubeId cube_id);