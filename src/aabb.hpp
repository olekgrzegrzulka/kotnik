#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "common.hpp"
#include "world.hpp"
// #include "player.hpp"

struct AABB {
  WorldPos half_extents;
  WorldPos offset{};

  // Returns a vector of cubes that this AABB placed at entity_pos overlaps.
  // needs_to_be_solid can be set to false to also detect non-solid cubes
  std::vector<CubePos> get_overlapping_cubes(const World& world, WorldPos entity_pos, bool needs_to_be_solid = true) {
    std::vector<CubePos> overlapping_cubes;
    CubePos min = floor_position(entity_pos - half_extents + offset);
    CubePos max = floor_position(entity_pos + half_extents + offset);

    for (int16_t x = min.x; x <= max.x; x += 1) {
      for (int16_t y = min.y; y <= max.y; y += 1) {
        for (int16_t z = min.z; z <= max.z; z += 1) {
          CubePos overlapping_pos = CubePos{x, y, z};
          if (!needs_to_be_solid || world.is_solid(overlapping_pos)) {
            overlapping_cubes.emplace_back(overlapping_pos);
          }
        }
      }
    }
    return overlapping_cubes;
  }

  // Returns if this AABB placed at entity_pos overlaps any of world's solid cube
  bool is_overlapping_cube(const World& world, WorldPos entity_pos, CubePos cube_world_pos) {
    auto v = get_overlapping_cubes(world, entity_pos, false);
    return std::find(v.begin(), v.end(), cube_world_pos) != v.end();
  }

  bool is_overlapping_any_cube(const World& world, WorldPos entity_pos) {
    CubePos min = {
        glm::floor(entity_pos.x - half_extents.x + offset.x),
        glm::floor(entity_pos.y - half_extents.y + offset.y),
        glm::floor(entity_pos.z - half_extents.z + offset.z)};

    CubePos max = {
        glm::floor(entity_pos.x + half_extents.x + offset.x),
        glm::floor(entity_pos.y + half_extents.y + offset.y),
        glm::floor(entity_pos.z + half_extents.z + offset.z)};
    for (int16_t x = min.x; x <= max.x; x += 1) {
      for (int16_t y = min.y; y <= max.y; y += 1) {
        for (int16_t z = min.z; z <= max.z; z += 1) {
          CubePos overlapping_pos = CubePos{x, y, z};
          if (world.is_solid(overlapping_pos)) {
            return true;
          }
        }
      }
    }
    return false;
  }
};