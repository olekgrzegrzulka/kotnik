#pragma once
#include "common.hpp"

struct AABB {
  WorldPos half_extents;
  WorldPos offset{};

  constexpr bool contains_point(WorldPos point) const {
    return (
        point.x >= offset.x - half_extents.x && point.x <= offset.x + half_extents.x &&
        point.y >= offset.y - half_extents.y && point.y <= offset.y + half_extents.y &&
        point.z >= offset.z - half_extents.z && point.z <= offset.z + half_extents.z);
  }

  static constexpr bool test(AABB first, WorldPos first_pos, AABB second, WorldPos second_pos) {
    if (std::abs(first_pos.x + first.offset.x - second.offset.x - second_pos.x) > (first.half_extents.x + second.half_extents.x)) return false;
    if (std::abs(first_pos.y + first.offset.y - second.offset.y - second_pos.y) > (first.half_extents.y + second.half_extents.y)) return false;
    if (std::abs(first_pos.z + first.offset.z - second.offset.z - second_pos.z) > (first.half_extents.z + second.half_extents.z)) return false;

    return true;
  }
};