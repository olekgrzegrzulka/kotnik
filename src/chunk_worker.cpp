#include <atomic>
#include <mutex>
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
        if (neigbour->flags.is_read_locked()) { return false; }
        if (!(neigbour->flags.ready)) { return false; }
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

  for (Chunk* chunk : surrounding_chunks) {
    assert(chunk->flags.threads_reading > 0);
    chunk->flags.threads_reading -= 1;
  }

  surrounding_chunks.clear();
  return true;
}

bool ChunkMeshWorker::is_finished() {
  return finished && surrounding_chunks.empty();
}

//
// TerrainGenWorker
//

void ChunkTerrainGenWorker::add_to_queue(Chunk* chunk) {
  if (!chunk) { return; }
  if (chunk->flags.ready) { return; }
  if (chunk->flags.is_write_locked()) { return; }
  if (chunk->flags.is_being_generated) { return; }
  chunk->flags.is_being_generated = true;

  std::scoped_lock lock(chunk_queue_mutex);

  chunk_queue.emplace_back(chunk);
}
Chunk* ChunkTerrainGenWorker::pop_from_queue() {
  std::scoped_lock lock(chunk_queue_mutex);

  if (chunk_queue.empty()) { return nullptr; }

  Chunk* chunk = chunk_queue.back();
  chunk_queue.pop_back();
  return chunk;
}

void ChunkTerrainGenWorker::update() {
  {
    std::scoped_lock lock(chunks_finished_mutex);

    for (Chunk* chunk : chunks_finished) {
      assert(!(chunk->flags.ready));
      assert(chunk->flags.thread_writing);

      chunk->flags.ready = true;
      chunk->flags.thread_writing = false;
      chunk->flags.is_being_generated = false;
    }

    chunks_finished.clear();
  }

  if (!is_running) {
    is_running = true;

    std::thread thread([=, this]() {
      while (true) {
        Chunk* chunk = pop_from_queue();
        if (!chunk) { break; }
        if (chunk->flags.is_write_locked()) { continue; }

        chunk->flags.thread_writing = true;
        world_gen->generate_chunk(chunk);
        // chunk->flags.thread_writing = false;

        std::scoped_lock lock(chunks_finished_mutex);
        chunks_finished.emplace_back(chunk);
      }
      is_running = false;
    });
    thread.detach();
  }
}