#pragma once
#include <vector>
#include "chunk.hpp"
#include "common.hpp"
#include "cubes.hpp"
#include "random.hpp"
#include "world.hpp"

class WorldMainMenu : public World {
public:
  WorldMainMenu() : World::World() {
    const i32 logo_size_x = logo[0].size() * logo_scale;
    const i32 logo_size_y = logo.size() * logo_scale;
    const i32 logo_size_z = logo_scale;

    bool tree_planted = false;

    for (i32 y = -2; y <= 1; y += 1) {
      for (i32 z = -2; z <= 1; z += 1) {
        for (i32 x = -2; x <= 2; x += 1) {
          ChunkPos chunk_pos = ChunkPos{x, y, z};
          chunks[chunk_pos] = std::make_unique<Chunk>(chunk_pos);
        }
      }
    }

    for (i32 y = 0; y < logo_size_y; y += 1) {
      for (i32 z = 0; z < logo_size_z; z += 1) {
        for (i32 x = 0; x < logo_size_x; x += 1) {
          const i32 logo_index_x = x / logo_scale;
          const i32 logo_index_y = y / logo_scale;
          const bool logo_value = logo[logo_index_y][logo_index_x];

          if (logo_value == false) { continue; }

          CubePos cube_pos = CubePos{x, logo_size_y - 1 - y, z};

          i32 depth = 0;
          for (size_t i = 1; i < 4; i += 1) {
            bool solid = get_cube(cube_pos + ChunkPos{0, i, 0}) != CubeId::AIR;
            if (solid) {
              depth += 1;
            } else {
              break;
            }
          }

          CubeId cube = CubeId::GRASS;
          if (depth == 1) { cube = CubeId::DIRT; }
          if (depth == 2) {
            if (StaticRandom::get().next<i32>(0, 3) == 0) {
              cube = CubeId::STONE;
            } else {
              cube = CubeId::DIRT;
            }
          }
          if (depth >= 3) { cube = CubeId::STONE; }
          set_cube(cube_pos, cube);
          if (get_cube(cube_pos + CubePos{0, 1, 0}) == CubeId::AIR) {
            i32 rng = StaticRandom::get().next<i32>(0, 100);
            if (rng >= 0 && rng <= 20) {
              set_cube(cube_pos + CubePos{0, 1, 0}, CubeId::GRASS_PLANT);
            } else if (rng >= 21 && rng <= 30) {
              set_cube(cube_pos + CubePos{0, 1, 0}, CubeId::FLOWER);
            }
            if (!tree_planted && y == 0 && StaticRandom::get().next<i32>(0, 200) == 0) {
              // tree_planted = true;
              gen_tree_poplar(cube_pos + CubePos{0, 1, 0});
            }
          }
        }
      }
    }
  }
  glm::vec<3, i32> get_logo_size() {
    return {
        logo_scale * logo[0].size(),
        logo_scale * logo.size(),
        logo_scale,
    };
  }

private:
  const i32 logo_scale = 2;
  const std::vector<std::vector<bool>> logo = {
      {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
      {1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0},
      {1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  };

  // FIXME: this has been adapted from world_gen.cpp, move tree-generating functions to world perhaps?
  void gen_tree_poplar(CubePos cube_pos) {
    i32 tree_height = StaticRandom::get().next<i32>(4, 6);

    for (i32 ox = -2; ox <= 2; ox += 1) {
      for (i32 oy = tree_height - 2; oy <= tree_height + 1; oy += 1) {
        for (i32 oz = -2; oz <= 2; oz += 1) {
          bool ox_edge = ox == -2 || ox == 2;
          bool oy_edge = oy == tree_height - 2 || oy == tree_height + 1;
          bool oz_edge = oz == -2 || oz == 2;
          LocalPos cube_pos_leaves = {cube_pos.x + ox, cube_pos.y + oy, cube_pos.z + oz};
          if ((int)ox_edge + (int)oy_edge + (int)oz_edge >= 2) { continue; }
          set_cube(cube_pos_leaves, CubeId::LEAVES);
        }
      }
    }

    for (i32 i = 0; i <= tree_height; i += 1) {
      CubePos cube_pos_trunk = {cube_pos.x, cube_pos.y + i, cube_pos.z};
      set_cube(cube_pos_trunk, CubeId::WOOD);
    }
  }
};