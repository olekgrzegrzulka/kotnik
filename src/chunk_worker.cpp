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

bool ChunkMeshWorker::run_job(Chunk* chunk) {

  if (!is_finished()) {
    return false;
  }

  std::vector<Chunk*> _surrounding_chunks;
  _surrounding_chunks.emplace_back((Chunk*)chunk);

  World& world = chunk->world;

  static std::array<ChunkPos, 6> neigbour_offsets = {ChunkPos{-1, 0, 0}, ChunkPos{1, 0, 0}, ChunkPos{0, -1, 0}, ChunkPos{0, 1, 0}, ChunkPos{0, 0, -1}, ChunkPos{0, 0, 1}};

  for (auto offset : neigbour_offsets) {
    Chunk* neigb = const_cast<Chunk*>(world.get_chunk(chunk->position + offset));
    if (!neigb) { return false; }
    if (neigb->flags.is_read_locked()) { return false; }
    if (!(neigb->flags.ready)) { return false; }

    _surrounding_chunks.emplace_back(neigb);
  }

  surrounding_chunks = _surrounding_chunks;
  finished = false;

  for (Chunk* c : surrounding_chunks) {
    c->flags.threads_reading += 1;
  }

  std::thread thread([=, this]() {
    chunk->renderer->rebuild_mesh(this->surrounding_chunks);
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

//
// LightningWorker
//
void ChunkLightningWorker::add_to_queue(Chunk* chunk) {
  std::scoped_lock lock(queue_mutex);
  queue.emplace(chunk);
}

bool ChunkLightningWorker::run_job() {
  if (!is_finished()) { return false; }

  finished = false;

  std::thread thread([=, this]() {
    while (true) {
      // Get next chunk to light, break if queue empty
      Chunk* chunk = nullptr;
      {
        std::scoped_lock lock(queue_mutex);
        if (queue.empty()) { break; }
        chunk = queue.back();
        queue.pop();
      }

      if (!(chunk->flags.ready)) { continue; }
      std::unordered_set<CubePos, Vec3Hasher> _unused;
      chunk->generate_lightmap(_unused);
    }
    this->finished = true;
  });
  thread.detach();

  return true;
}

bool ChunkLightningWorker::is_finished() {
  return finished;
}