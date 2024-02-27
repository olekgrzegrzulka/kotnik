#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "common.hpp"

struct AABB {
  WorldPos half_extents;
  WorldPos offset{};

  static constexpr bool test(AABB first, WorldPos first_pos, AABB second, WorldPos second_pos) {
    if (std::abs(first_pos.x + first.offset.x - second_pos.x) > (first.half_extents.x + second.half_extents.x)) return false;
    if (std::abs(first_pos.y + first.offset.y - second_pos.y) > (first.half_extents.y + second.half_extents.y)) return false;
    if (std::abs(first_pos.z + first.offset.z - second_pos.z) > (first.half_extents.z + second.half_extents.z)) return false;

    return true;
  }
};