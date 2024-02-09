#pragma once
#include <glm/glm.hpp>
#include <stdint.h>
#include "common.hpp"

#define ATLAS_SIZE (16)

static constexpr glm::vec<2, float> get_uv_offset(uint32_t x, uint32_t y) {
  return {
      (float)(x) / (float)ATLAS_SIZE,
      (float)(y) / (float)ATLAS_SIZE,
  };
}

static glm::vec<2, float> get_cube_uv_offset(CubeId cube, Dir face) {
  if (cube == CubeId::DIRT) { return (get_uv_offset(1, 0)); }
  if (cube == CubeId::GRASS) {
    if (face == Dir::TOP) {
      return (get_uv_offset(3, 0));
    } else if (face == Dir::BOTTOM) {
      return (get_uv_offset(1, 0));
    } else {
      return (get_uv_offset(2, 0));
    }
  }
  if (cube == CubeId::STONE) {
    return (get_uv_offset(4, 0));
  }
  if (cube == CubeId::SAND) {
    return (get_uv_offset(5, 0));
  }
  if (cube == CubeId::GRAVEL) {
    return (get_uv_offset(6, 0));
  }
  if (cube == CubeId::WOOD) {
    return (get_uv_offset(7, 0));
  }
  return get_uv_offset(0, 0);
}