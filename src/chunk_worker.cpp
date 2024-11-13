#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <ratio>
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

bool ChunkMeshWorker::add_to_queue(ChunkPos chunk_pos, World& world) {
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
  if (is_running) { std::scoped_lock lock(chunk_queue_mutex); }
  chunk_queue.emplace_back(chunk_pos, std::move(data));
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

void ChunkMeshWorker::lock_queue() {
  chunk_queue_mutex.lock();
}

void ChunkMeshWorker::unlock_queue() {
  chunk_queue_mutex.unlock();
}

size_t ChunkMeshWorker::get_queue_size() {
  std::scoped_lock lock(chunk_queue_mutex);
  return chunk_queue.size();
}

std::optional<std::pair<ChunkPos, std::unique_ptr<ChunkMeshData>>> ChunkMeshWorker::pop_from_queue() {
  std::scoped_lock lock(chunk_queue_mutex);

  if (chunk_queue.empty()) { return std::nullopt; }

  auto data = std::move(chunk_queue.back());
  chunk_queue.pop_back();
  return data;
}

std::vector<std::pair<ChunkPos, std::unique_ptr<ChunkMesh>>> ChunkMeshWorker::collect_finished_chunks() {
  std::scoped_lock lock(chunks_finished_mutex);
  return std::exchange(chunks_finished, {});
}

void ChunkMeshWorker::update() {
  if (is_running) { return; }
  if (chunk_queue.size() == 0) { return; }

  is_running = true;

  std::thread thread([=, this]() {
    while (true) {
      decltype(chunk_queue) chunk_queue_;
      chunk_queue_mutex.lock();
      std::swap(chunk_queue, chunk_queue_);
      chunk_queue_mutex.unlock();

      if (chunk_queue_.size() == 0) { break; }
      for (auto& elem : chunk_queue_) {
        auto [chunk_pos, data] = std::move(elem);
        ensure(data->is_valid());
        auto mesh = std::make_unique<ChunkMesh>(std::move(data));
        {
          std::scoped_lock lock(chunks_finished_mutex);
          chunks_finished.emplace_back(chunk_pos, std::move(mesh));
        }
      }
    }
    is_running = false;
  });
  thread.detach();
}

//
// TerrainGenWorker
//

void ChunkTerrainGenWorker::add_to_queue(ChunkPos chunk_pos) {
  std::scoped_lock lock(chunk_queue_mutex);
  chunk_queue.emplace_back(chunk_pos);
}

std::optional<ChunkPos> ChunkTerrainGenWorker::pop_from_queue() {
  std::scoped_lock lock(chunk_queue_mutex);

  if (chunk_queue.empty()) { return std::nullopt; }

  ChunkPos chunk_pos = chunk_queue.back();
  chunk_queue.pop_back();
  return chunk_pos;
}

std::vector<std::unique_ptr<Chunk>> ChunkTerrainGenWorker::collect_finished_chunks() {
  std::scoped_lock lock(chunks_finished_mutex);
  return std::exchange(chunks_finished, {});
  ;
}

void ChunkTerrainGenWorker::update() {
  if (!is_running) {
    is_running = true;

    std::thread thread([=, this]() {
      while (true) {
        std::optional<ChunkPos> chunk_pos = pop_from_queue();
        if (!chunk_pos) { break; }

        auto chunk = std::make_unique<Chunk>(chunk_pos.value());
        world_gen->generate_chunk(chunk.get());

        {
          std::scoped_lock lock(chunks_finished_mutex);
          chunk->flags.awaiting_mesh_update = true;
          chunks_finished.emplace_back(std::move(chunk));
        }
      }
      is_running = false;
    });
    thread.detach();
  }
}