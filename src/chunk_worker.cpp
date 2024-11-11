#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include "chunk.hpp"
#include "chunk_renderer.hpp"
#include "chunk_worker.hpp"
#include "world.hpp"
#include "world_gen.hpp"

//
// ChunkMeshWorker
//

bool ChunkMeshWorker::run_job(ChunkPos chunk_pos, World& world) {

  if (!is_finished()) {
    return false;
  }

  Chunk* chunk = world.get_chunk(chunk_pos);

  for (i32 x = -1; x <= 1; x += 1) {
    for (i32 y = -1; y <= 1; y += 1) {
      for (i32 z = -1; z <= 1; z += 1) {
        Chunk* neigbour = world.get_chunk(chunk_pos + ChunkPos{x, y, z});
        if (!neigbour) { return false; }
      }
    }
  }

  ChunkMeshData chunk_mesh_data{chunk_pos, world};

  finished = false;

  std::thread thread([=, this]() {
    chunk->renderer->rebuild_mesh(std::move(chunk_mesh_data));
    this->finished = true;
  });
  thread.detach();

  return true;
}

bool ChunkMeshWorker::try_collecting() {
  if (!finished) {
    return false;
  }

  return true;
}

bool ChunkMeshWorker::is_finished() {
  return finished;
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
  return std::move(chunks_finished);
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