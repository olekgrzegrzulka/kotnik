#pragma once
#include <unordered_set>
#include <vector>
#include "chunk_renderer.hpp"
#include "chunk_worker.hpp"
#include "common.hpp"
#include "player.hpp"
#include "world.hpp"

class WorldRenderer final {
  World& world;
  std::unordered_set<ChunkPos, Vec3Hasher> chunks_awaiting_mesh_update;
  std::vector<ChunkMeshWorker*> chunk_mesh_workers;

  template <typename T>
  void sort_vector_by_distance(std::vector<glm::vec<3, T>>& vector, glm::vec<3, T> pos) {
    using Vec3T = glm::vec<3, T>;
    std::sort(vector.begin(), vector.end(), [&](const Vec3T a, const Vec3T b) {
      Vec3T first = pos - a;
      Vec3T second = pos - b;
      return std::abs(first.x) + std::abs(first.y) + std::abs(first.z) < std::abs(second.x) + std::abs(second.y) + std::abs(second.z);
    });
  }

public:
  WorldRenderer(World& _world) : world(_world) {
    for (size_t i = 0; i < 16; i += 1) {
      chunk_mesh_workers.push_back(new ChunkMeshWorker);
    }
  }

  ~WorldRenderer() {
    for (size_t i = 0; i < chunk_mesh_workers.size(); i += 1) {
      delete chunk_mesh_workers[i];
    }
  }

  void update(WorldPos camera_pos, const glm::mat4& camera_matrix) {
    // Unlock chunks locked by mesh worker threads
    for (const auto& worker : chunk_mesh_workers) {
      if (worker->try_collecting()) {
      }
    }

    for (auto& [chunk_pos, chunk] : world.chunks) {
      if (chunk->renderer->can_swap_buffers) {
        chunk->renderer->swap_buffers();
      }

      if (chunk->flags.update_geometry) {
        chunks_awaiting_mesh_update.emplace(chunk_pos);

        if (chunk->flags.update_geometry_of_adjacent_chunks.left) {
          chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(-1, 0, 0));
          chunk->flags.update_geometry_of_adjacent_chunks.left = false;
        }

        if (chunk->flags.update_geometry_of_adjacent_chunks.right) {
          chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(1, 0, 0));
          chunk->flags.update_geometry_of_adjacent_chunks.right = false;
        }

        if (chunk->flags.update_geometry_of_adjacent_chunks.down) {
          chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, -1, 0));
          chunk->flags.update_geometry_of_adjacent_chunks.down = false;
        }

        if (chunk->flags.update_geometry_of_adjacent_chunks.up) {
          chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 1, 0));
          chunk->flags.update_geometry_of_adjacent_chunks.up = false;
        }

        if (chunk->flags.update_geometry_of_adjacent_chunks.front) {
          chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 0, -1));
          chunk->flags.update_geometry_of_adjacent_chunks.front = false;
        }

        if (chunk->flags.update_geometry_of_adjacent_chunks.back) {
          chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 0, 1));
          chunk->flags.update_geometry_of_adjacent_chunks.back = false;
        }

        chunk->flags.update_geometry = false;
      }

      if (chunk->flags.ready) {
        chunk->renderer->draw(camera_pos, camera_matrix);
      }
    }

    std::unordered_set<ChunkPos, Vec3Hasher> new_chunks_awaiting_mesh_update;

    std::vector<ChunkPos> chunks_awaiting_mesh_update_sorted_by_distance;
    for (auto x : chunks_awaiting_mesh_update) {
      chunks_awaiting_mesh_update_sorted_by_distance.emplace_back(x);
    }
    sort_vector_by_distance(chunks_awaiting_mesh_update_sorted_by_distance, (world.player) ? world_pos_to_chunk_pos(world.player->get_world_pos()) : ChunkPos{0, 0, 0});

    for (const ChunkPos chunk_pos : chunks_awaiting_mesh_update_sorted_by_distance) {
      const Chunk* chunk = world.get_chunk(chunk_pos);
      if (chunk == nullptr) { continue; }
      if (!chunk->flags.ready) { continue; }

      bool updated = false;
      for (const auto& worker : chunk_mesh_workers) {
        if (worker->run_job(const_cast<Chunk*>(chunk))) {
          updated = true;
          break;
        }
      }

      if (!updated) {
        new_chunks_awaiting_mesh_update.emplace(chunk->position);
      }
    }

    chunks_awaiting_mesh_update = new_chunks_awaiting_mesh_update;
  }
};