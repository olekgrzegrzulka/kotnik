#include "world_renderer.hpp"
#include <unordered_set>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include "chunk.hpp"
#include "chunk_renderer.hpp"
#include "chunk_worker.hpp"
#include "common.hpp"
#include "glad/glad.h"
#include "player.hpp"
#include "world.hpp"

static void sort_chunk_vector_by_manhattan_distance(std::vector<Chunk*>& vector, ChunkPos to) {
  std::sort(vector.begin(), vector.end(), [&](const Chunk* a, const Chunk* b) {
    ChunkPos first = glm::abs(to - a->position);
    ChunkPos second = glm::abs(to - b->position);
    return first.x + first.y + first.z < second.x + second.y + second.z;
  });
}

WorldRenderer::WorldRenderer(World& _world) : world(_world) {
  for (size_t i = 0; i < 4; i += 1) {
    chunk_mesh_workers.push_back(new ChunkMeshWorker);
  }
}

WorldRenderer::~WorldRenderer() {
  for (size_t i = 0; i < chunk_mesh_workers.size(); i += 1) {
    delete chunk_mesh_workers[i];
  }
}

void WorldRenderer::update(WorldPos camera_pos, const glm::mat4& camera_matrix) {
  // Unlock chunks locked by mesh worker threads
  for (const auto& worker : chunk_mesh_workers) {
    if (worker->try_collecting()) {
    }
  }

  static constexpr glm::vec3 light = {0.41f, 0.82f, 0.41f};
  /* matrix     */ glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera_matrix));
  /* light dir  */ glUniform3f(1, light.x, light.y, light.z);
  /* camera pos */ glUniform3f(2, camera_pos.x, camera_pos.y, camera_pos.z);
  /* alpha      */ glUniform1f(3, 1.0);

  for (auto& [chunk_pos, chunk] : world.chunks) {
    if (chunk->renderer->can_swap_buffers) {
      chunk->renderer->swap_buffers();
    }

    if (chunk->flags.awaiting_mesh_update) {
      chunks_awaiting_mesh_update.emplace(chunk_pos);
      chunk->flags.awaiting_mesh_update = false;

      if (chunk->flags.update_mesh_of_adjacent_chunk.left) {
        chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(-1, 0, 0));
        chunk->flags.update_mesh_of_adjacent_chunk.left = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.right) {
        chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(1, 0, 0));
        chunk->flags.update_mesh_of_adjacent_chunk.right = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.down) {
        chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, -1, 0));
        chunk->flags.update_mesh_of_adjacent_chunk.down = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.up) {
        chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 1, 0));
        chunk->flags.update_mesh_of_adjacent_chunk.up = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.front) {
        chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 0, -1));
        chunk->flags.update_mesh_of_adjacent_chunk.front = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.back) {
        chunks_awaiting_mesh_update.emplace(chunk_pos + ChunkPos(0, 0, 1));
        chunk->flags.update_mesh_of_adjacent_chunk.back = false;
      }
    }

    chunk->renderer->draw();
  }

  std::vector<Chunk*> chunks_sorted_by_distance_to_player;
  for (auto& [chunk_pos, chunk] : world.chunks) {
    chunks_sorted_by_distance_to_player.emplace_back(chunk.get());
  }
  ChunkPos player_chunk_pos = world.player->world_pos / static_cast<double>(CHUNK_SIZE);
  sort_chunk_vector_by_manhattan_distance(chunks_sorted_by_distance_to_player, player_chunk_pos);

  /* alpha      */ glUniform1f(3, 0.8);
  for (auto it = chunks_sorted_by_distance_to_player.rbegin(); it != chunks_sorted_by_distance_to_player.rend(); ++it) {
    Chunk* chunk = *it;
    chunk->renderer->draw_translucent();
  }

  std::unordered_set<ChunkPos, Vec3Hasher> new_chunks_awaiting_mesh_update;

  std::vector<ChunkPos> chunks_awaiting_mesh_update_sorted_by_distance;
  for (auto x : chunks_awaiting_mesh_update) {
    chunks_awaiting_mesh_update_sorted_by_distance.emplace_back(x);
  }

  for (const ChunkPos chunk_pos : chunks_awaiting_mesh_update_sorted_by_distance) {
    Chunk* chunk = world.get_chunk(chunk_pos);
    if (chunk == nullptr) { continue; }

    bool updated = false;
    for (const auto& worker : chunk_mesh_workers) {
      if (worker->run_job(chunk->position, world)) {
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