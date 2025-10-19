#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>
#include "chunk.hpp"
#include "chunk_mesh.hpp"
#include "chunk_worker.hpp"
#include "common.hpp"
#include "world.hpp"
#include "world_gen.hpp"

//
// ChunkMeshWorker
//

ChunkMeshWorker::ChunkMeshWorker() {
  thread = std::thread([=, this]() -> void {
    while (true) {
      chunk_queue_t queue_elem;
      {
        std::unique_lock lock(chunk_queue_mutex);
        cond_var.wait(lock, [&]() -> bool { return kill_thread || chunk_queue.size() > 0; });

        if (kill_thread) { break; }

        queue_elem = std::move(chunk_queue.back());
        chunk_queue.pop_back();
      }
      auto [chunk_pos, data] = std::move(queue_elem);

      ensure(data->is_valid());
      auto mesh = std::make_unique<ChunkMesh>(std::move(data));
      {
        std::unique_lock lock(chunks_finished_mutex);
        chunks_finished.emplace_back(chunk_pos, std::move(mesh));
      }
    }
  });
}

ChunkMeshWorker::~ChunkMeshWorker() {
  kill_thread = true;
  cond_var.notify_all();
  thread.join();
}

bool ChunkMeshWorker::add_to_queue(ChunkPos chunk_pos, World& world) {
  Chunk* c = world.get_chunk(chunk_pos);
  if (c && c->has_no_cubes()) {
    std::unique_lock lock(chunks_finished_mutex);
    chunks_finished.emplace_back(chunk_pos, std::make_unique<ChunkMesh>());
    return true;
  }

  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* chunk = world.get_chunk(chunk_pos + ChunkPos{x, y, z});
        if (!chunk) {
          return false;
        }
      }
    }
  }

  {
    auto data = std::make_unique<ChunkMeshData>(chunk_pos, world);
    if (!data->is_valid()) { return false; }
    std::unique_lock lock(chunk_queue_mutex);
    chunk_queue.emplace_back(chunk_pos, std::move(data));
  }

  cond_var.notify_all();

  return true;
}

bool ChunkMeshWorker::add_to_queue_no_mutex(ChunkPos chunk_pos, World& world) {
  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* chunk = world.get_chunk(chunk_pos + ChunkPos{x, y, z});
        if (!chunk) {
          return false;
        }
      }
    }
  }

  auto data = std::make_unique<ChunkMeshData>(chunk_pos, world);
  if (!data->is_valid()) { return false; }
  chunk_queue.emplace_back(chunk_pos, std::move(data));
  return true;
}

void ChunkMeshWorker::sort_queue_by_distance(ChunkPos to) {
  std::unique_lock lock(chunk_queue_mutex);

  sort_queue_by_distance_no_mutex(to);
}

void ChunkMeshWorker::sort_queue_by_distance_no_mutex(ChunkPos to) {
  using comp_t = const ChunkMeshWorker::chunk_queue_t&;
  std::sort(chunk_queue.begin(), chunk_queue.end(), [&to](comp_t a, comp_t b) -> bool {
    return manhattan_distance(a.first, to) > manhattan_distance(b.first, to);
  });
}

void ChunkMeshWorker::lock_queue() {
  chunk_queue_mutex.lock();
}

void ChunkMeshWorker::unlock_queue() {
  chunk_queue_mutex.unlock();
}

size_t ChunkMeshWorker::get_queue_size() {
  std::unique_lock lock(chunk_queue_mutex);
  return chunk_queue.size();
}

std::vector<std::pair<ChunkPos, std::unique_ptr<ChunkMesh>>> ChunkMeshWorker::collect_finished_chunks() {
  std::unique_lock lock(chunks_finished_mutex);
  return std::exchange(chunks_finished, {});
}

//
// TerrainGenWorker
//
ChunkTerrainGenWorker::ChunkTerrainGenWorker(std::shared_ptr<WorldGen> world_gen_) : world_gen(world_gen_) {
  thread = std::thread([=, this]() -> void {
    while (true) {
      glm::vec<2, i32> chunk_column_pos;
      {
        std::unique_lock lock(chunk_queue_mutex);
        cond_var.wait(lock, [&]() -> bool { return kill_thread || chunk_queue.size() > 0; });

        if (kill_thread) { break; }

        chunk_column_pos = chunk_queue.back();
        chunk_queue.pop_back();
      }

      auto chunks = world_gen->generate_chunk_column(chunk_column_pos);

      {
        std::unique_lock lock2(chunks_finished_mutex);
        for (auto&& chunk : chunks) {
          chunk->flags.awaiting_mesh_update = true;
        }
        chunks_finished.emplace_back(std::move(chunks));
      }
    }
  });
}

ChunkTerrainGenWorker::~ChunkTerrainGenWorker() {
  kill_thread = true;
  cond_var.notify_all();
  thread.join();
}

void ChunkTerrainGenWorker::add_to_queue(glm::vec<2, int> chunk_column_pos) {
  {
    std::unique_lock lock(chunk_queue_mutex);
    chunk_queue.emplace_back(chunk_column_pos);
  }
  cond_var.notify_all();
}

std::vector<std::vector<std::unique_ptr<Chunk>>> ChunkTerrainGenWorker::collect_finished_chunks() {
  std::unique_lock lock(chunks_finished_mutex);
  return std::exchange(chunks_finished, {});
}