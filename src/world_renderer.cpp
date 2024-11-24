#include "world_renderer.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <unordered_set>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include "chunk.hpp"
#include "chunk_mesh.hpp"
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
  for (size_t i = 0; i < 2; i += 1) {
    chunk_mesh_workers.push_back(new ChunkMeshWorker);
  }
}

WorldRenderer::~WorldRenderer() {
  for (size_t i = 0; i < chunk_mesh_workers.size(); i += 1) {
    delete chunk_mesh_workers[i];
  }
}

void WorldRenderer::add_chunk_for_mesh_update(ChunkPos chunk_pos) {
  Chunk* chunk = world.get_chunk(chunk_pos);
  if (!chunk) { return; }
  if (!(chunk->flags.awaiting_mesh_update)) { return; }
  if (chunks_awaiting_mesh_update.contains(chunk_pos)) { return; }
  // chunk->flags.awaiting_mesh_update = true;
  // if (chunks_awaiting_mesh_update.contains(chunk_pos)) { return; }
  chunk->flags.awaiting_mesh_update = false;
  chunks_awaiting_mesh_update.emplace(chunk_pos);
}

void WorldRenderer::update(WorldPos camera_pos, const glm::mat4& camera_matrix) {
  for (auto& worker : chunk_mesh_workers) {
    worker->update();
  }

  // Retrieve new meshes for chunks
  for (const auto& worker : chunk_mesh_workers) {
    for (auto& [chunk_pos, chunk_mesh] : worker->collect_finished_chunks()) {
      Chunk* chunk = world.get_chunk(chunk_pos);
      if (!chunk) { continue; }
      auto prev_mesh = std::move(chunk->mesh);
      chunk->mesh = std::move(chunk_mesh);
      chunk->mesh->inherit_buffers_from_previous_mesh(prev_mesh.get());
      chunk->flags.awaiting_mesh_update = false;
      // ensure(chunks_being_meshed.contains(chunk_pos));
      chunks_being_meshed.erase(chunk_pos);
    }
  }

  static constexpr glm::vec3 light = {0.41f, 0.82f, 0.41f};
  /* matrix     */ glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera_matrix));
  /* light dir  */ glUniform3f(1, light.x, light.y, light.z);
  /* camera pos */ glUniform3f(2, camera_pos.x, camera_pos.y, camera_pos.z);
  /* alpha      */ glUniform1f(3, 1.0);

  for (auto& [chunk_pos, chunk] : world.chunks) {
    // Propagate chunk mesh update request to adjacent chunks and update meshes
    if (chunk->flags.awaiting_mesh_update) {
      add_chunk_for_mesh_update(chunk_pos);

      if (chunk->flags.update_mesh_of_adjacent_chunk.left) {
        Chunk* adjacent_chunk = world.get_chunk(chunk_pos + ChunkPos(-1, 0, 0));
        if (adjacent_chunk) {
          adjacent_chunk->flags.awaiting_mesh_update = true;
          add_chunk_for_mesh_update(chunk_pos + ChunkPos(-1, 0, 0));
        }
        chunk->flags.update_mesh_of_adjacent_chunk.left = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.right) {
        Chunk* adjacent_chunk = world.get_chunk(chunk_pos + ChunkPos(1, 0, 0));
        if (adjacent_chunk) {
          adjacent_chunk->flags.awaiting_mesh_update = true;
          add_chunk_for_mesh_update(chunk_pos + ChunkPos(1, 0, 0));
        }
        chunk->flags.update_mesh_of_adjacent_chunk.right = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.down) {
        Chunk* adjacent_chunk = world.get_chunk(chunk_pos + ChunkPos(0, -1, 0));
        if (adjacent_chunk) {
          adjacent_chunk->flags.awaiting_mesh_update = true;
          add_chunk_for_mesh_update(chunk_pos + ChunkPos(0, -1, 0));
        }
        chunk->flags.update_mesh_of_adjacent_chunk.down = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.up) {
        Chunk* adjacent_chunk = world.get_chunk(chunk_pos + ChunkPos(0, 1, 0));
        if (adjacent_chunk) {
          adjacent_chunk->flags.awaiting_mesh_update = true;
          add_chunk_for_mesh_update(chunk_pos + ChunkPos(0, 1, 0));
        }
        chunk->flags.update_mesh_of_adjacent_chunk.up = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.front) {
        Chunk* adjacent_chunk = world.get_chunk(chunk_pos + ChunkPos(0, 0, -1));
        if (adjacent_chunk) {
          adjacent_chunk->flags.awaiting_mesh_update = true;
          add_chunk_for_mesh_update(chunk_pos + ChunkPos(0, 0, -1));
        }
        chunk->flags.update_mesh_of_adjacent_chunk.front = false;
      }

      if (chunk->flags.update_mesh_of_adjacent_chunk.back) {
        Chunk* adjacent_chunk = world.get_chunk(chunk_pos + ChunkPos(0, 0, 1));
        if (adjacent_chunk) {
          adjacent_chunk->flags.awaiting_mesh_update = true;
          add_chunk_for_mesh_update(chunk_pos + ChunkPos(0, 0, 1));
        }
        chunk->flags.update_mesh_of_adjacent_chunk.back = false;
      }
    }
    chunk->mesh->draw();
  }

  std::vector<Chunk*> chunks_sorted_by_distance_to_player;
  for (auto& [chunk_pos, chunk] : world.chunks) {
    chunks_sorted_by_distance_to_player.emplace_back(chunk.get());
  }
  ChunkPos player_chunk_pos = (world.player) ? world_pos_to_chunk_pos(world.player->world_pos) : ChunkPos{0, 0, 0};
  sort_chunk_vector_by_manhattan_distance(chunks_sorted_by_distance_to_player, player_chunk_pos);

  /* alpha      */ glUniform1f(3, 0.8);
  for (auto it = chunks_sorted_by_distance_to_player.rbegin(); it != chunks_sorted_by_distance_to_player.rend(); ++it) {
    Chunk* chunk = *it;
    chunk->mesh->draw_translucent();
  }

  std::vector<ChunkPos> chunks_awaiting_mesh_update_sorted_by_distance;
  for (auto x : chunks_awaiting_mesh_update) {
    chunks_awaiting_mesh_update_sorted_by_distance.emplace_back(x);
  }

  std::multimap<i32, Chunk*, std::greater<i32>> chunks_for_remeshing;
  // Skip the edge chunks as they don't have all neigbours and can't be meshed

  for (int x = -world.chunk_load_distance + 1; x <= world.chunk_load_distance - 1; x += 1) {
    for (int z = -world.chunk_load_distance + 1; z <= world.chunk_load_distance - 1; z += 1) {
      for (int y = -world.chunk_load_distance + 1; y <= world.chunk_load_distance - 1; y += 1) {
        ChunkPos chunk_pos = player_chunk_pos + ChunkPos{x, y, z};
        if (!chunks_awaiting_mesh_update.contains(chunk_pos)) {
          continue;
        }
        Chunk* chunk = world.get_chunk(chunk_pos);
        if (!chunk) {
          chunks_awaiting_mesh_update.erase(chunk_pos);
          continue;
        }
        i32 distance_to_chunk = manhattan_distance(chunk->position, player_chunk_pos);
        chunks_for_remeshing.emplace(distance_to_chunk, chunk);
      }
    }
  }

  if (!chunks_for_remeshing.empty()) {
    for (auto& worker : chunk_mesh_workers) {
      // worker->lock_queue();
    }

    size_t i = 0;
    for (auto& [_, chunk] : chunks_for_remeshing) {
      size_t worker_index = i % chunk_mesh_workers.size();
      // bool success = chunk_mesh_workers[worker_index]->add_to_queue_no_mutex(chunk->position, world);
      bool success = chunk_mesh_workers[worker_index]->add_to_queue(chunk->position, world);
      if (success) {
        chunks_being_meshed.emplace(chunk->position);
        chunks_awaiting_mesh_update.erase(chunk->position);
      }

      i += 1;
    }

    for (auto& worker : chunk_mesh_workers) {
      worker->sort_queue_by_distance(player_chunk_pos);
      // worker->sort_queue_by_distance_no_mutex(player_chunk_pos);
      // worker->unlock_queue();
    }
  }
}