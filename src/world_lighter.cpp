#include "world_lighter.hpp"
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <unordered_map>
#include "array3d.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "world.hpp"

void WorldLighter::update() {
  for (auto&& chunk : chunks_to_be_lit) {
  }
}

void WorldLighter::spread_light(i32 chunk_x, i32 chunk_z) {
  ScopeTimer sc{"spread_light()"};

  Array3D<Chunk*, 3, World::max_chunk_y - World::min_chunk_y + 1, 3> chunks{chunk_x - 1, World::min_chunk_y, chunk_z - 1};
  for (i32 x = chunk_x - 1; x <= chunk_x + 1; x += 1) {
    for (i32 z = chunk_z - 1; z <= chunk_z + 1; z += 1) {
      for (i32 y = World::min_chunk_y; y <= World::max_chunk_y; y += 1) {
        Chunk* chunk = world.get_chunk({x, y, z});
        chunks.set({x, y, z}, chunk);
        if (chunk) { chunk->flags.awaiting_mesh_update = true; }
      }
    }
  }

  Array3D<i32, Chunk::chunk_size * 3, 1, Chunk::chunk_size * 3> heightmap{-Chunk::chunk_size, 0, -Chunk::chunk_size};
  heightmap.fill(World::min_chunk_y * Chunk::chunk_size);
  int i = 0;
  for (i32 x = -Chunk::chunk_size; x < Chunk::chunk_size * 2; x += 1) {

    for (i32 z = -Chunk::chunk_size; z < Chunk::chunk_size * 2; z += 1) {

      for (i32 chunk_y = World::max_chunk_y; chunk_y >= World::min_chunk_y; chunk_y -= 1) {
        auto [chunk_pos, local_pos] = cube_to_local({x, 0, z});
        chunk_pos += ChunkPos{chunk_x, chunk_y, chunk_z};
        if (std::abs(chunk_pos.x - chunk_x) > 1 || std::abs(chunk_pos.z - chunk_z) > 1) {
          debug_warn(chunk_x, " ", chunk_z);
          debug_warn(chunk_pos);
          debug_warn(local_pos);
          debug_warn(CubePos{x, chunk_y * Chunk::chunk_size, z});
          debug_error("");
        }
        Chunk* chunk = chunks.at(chunk_pos);
        if (!chunk) { continue; }
        auto y_local = chunk->get_heightmap(local_pos.x, local_pos.z);
        if (y_local.has_value()) {
          heightmap.set({x, 0, z}, y_local.value() + chunk->position.y * Chunk::chunk_size);
          break;
        }
      }
    }
  }

  for (i32 x_local_sunlit = 0; x_local_sunlit < Chunk::chunk_size; x_local_sunlit += 1) {
    for (i32 z_local_sunlit = 0; z_local_sunlit < Chunk::chunk_size; z_local_sunlit += 1) {
      auto y_heightmap = heightmap.at({x_local_sunlit, 0, z_local_sunlit});
      for (i32 y = World::max_chunk_y * Chunk::chunk_size + Chunk::chunk_size - 1; y > y_heightmap; y -= 1) {
        CubePos sunlit_cube_pos = {
            x_local_sunlit + chunk_x * Chunk::chunk_size,
            y,
            z_local_sunlit + chunk_z * Chunk::chunk_size,
        };
        {
          auto [chunk_pos, local_pos] = cube_to_local(sunlit_cube_pos);
          Chunk* chunk = chunks.at(chunk_pos);
          chunk->set_lightmap(local_pos, std::numeric_limits<u8>::max());
        }
      }
    }
  }

  for (i32 x_local_sunlit = 0; x_local_sunlit < Chunk::chunk_size; x_local_sunlit += 1) {
    for (i32 z_local_sunlit = 0; z_local_sunlit < Chunk::chunk_size; z_local_sunlit += 1) {
      auto y_heightmap = heightmap.at({x_local_sunlit, 0, z_local_sunlit});
      for (i32 y = World::max_chunk_y * Chunk::chunk_size + Chunk::chunk_size - 1; y > y_heightmap; y -= 1) {

        CubePos sunlit_cube_pos = {
            x_local_sunlit + chunk_x * Chunk::chunk_size,
            y,
            z_local_sunlit + chunk_z * Chunk::chunk_size,
        };

        std::vector<CubePos> cubes_to_be_lit = {sunlit_cube_pos};

        while (!cubes_to_be_lit.empty()) {
          auto cube_pos = cubes_to_be_lit.back();
          cubes_to_be_lit.pop_back();

          auto [chunk_pos, local_pos] = cube_to_local(cube_pos);
          Chunk* chunk = chunks.at(chunk_pos);
          i32 brightness = chunk->get_lightmap(local_pos);

          if (brightness > 0) {
            for (CubePos n : {CubePos{-1, 0, 0}, {+1, 0, 0}, {0, -1, 0}, {0, +1, 0}, {0, 0, -1}, {0, 0, +1}}) {
              auto cube_pos_n = cube_pos + n;
              auto [chunk_pos_n, local_pos_n] = cube_to_local(cube_pos_n);
              if (chunk_pos_n.y > World::max_chunk_y || chunk_pos_n.y < World::min_chunk_y) { continue; }
              if (chunk_pos_n.x > chunk_x + 1 || chunk_pos_n.x < chunk_x - 1) { continue; }
              if (chunk_pos_n.z > chunk_z + 1 || chunk_pos_n.z < chunk_z - 1) { continue; }
              Chunk* chunk_n = chunks.at(chunk_pos_n);
              if (!chunk_n) { continue; }

              CubeId cube_id_n = chunks.at(chunk_pos_n)->get_cube(local_pos_n);
              if ((cube_id_n == CubeId::AIR || cube_id_n == CubeId::GRASS_PLANT || cube_id_n == CubeId::FLOWER) && chunk_n->get_lightmap(local_pos_n) < brightness - 32) {
                chunk_n->set_lightmap(local_pos_n, brightness - 32);
                cubes_to_be_lit.emplace_back(cube_pos_n);
                i += 1;
              }
            }
          }
        }
      }
    }
  }
  debug_warn(i);
}