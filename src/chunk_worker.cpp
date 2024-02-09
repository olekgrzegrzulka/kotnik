#include <algorithm>
#include <atomic>
#include <execution>
#include <stdexcept>
#include <thread>
#include <vector>
#include "chunk.hpp"
#include "chunk_renderer2.hpp"
#include "chunk_worker.hpp"
#include "fast_noise_lite.h"
#include "terrain_gen.hpp"
#include "world.hpp"

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

  for (int x : {-1, 0, 1}) {
    for (int y : {-1, 0, 1}) {
      for (int z : {-1, 0, 1}) {
        if (x == 0 && y == 0 && z == 0) { continue; }
        if (Chunk* neigb = (Chunk*)(world.get_chunk(chunk->position + ChunkPos(x, y, z)))) { _surrounding_chunks.emplace_back(neigb); }
      }
    }
  }

  for (Chunk* c : _surrounding_chunks) {
    if (c->flags.locked) {
      return false;
    }
    if (!(c->flags.ready)) {
      return false;
    }
  }

  surrounding_chunks = _surrounding_chunks;

  finished = false;

  for (Chunk* c : surrounding_chunks) {
    c->flags.locked = true;
  }

  std::thread thread([=, this]() {
    chunk->renderer->recreate_geometry(this->surrounding_chunks);
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
    assert(chunk->flags.locked);
    chunk->flags.locked = false;
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

void generate_chunks_terrain(Chunk* chunk) {
  assert((chunk->flags.locked));
  TerrainGen::generate_chunk(chunk);
}

bool ChunkTerrainGenWorker::run_job(Chunk* _chunk) {

  if (_chunk->flags.ready) { return false; }
  if (_chunk->flags.locked) { return false; }
  if (!finished) { return false; }
  if (chunk) { return false; }

  chunk = _chunk;
  chunk->flags.locked = true;
  finished = false;

  std::thread thread([=, this]() {
    generate_chunks_terrain(this->chunk);
    this->finished = true;
  });
  thread.detach();

  return true;
}

bool ChunkTerrainGenWorker::try_collecting() {
  if (!finished) { return false; }
  if (!chunk) { return false; }

  assert(!(chunk->flags.ready));
  assert(chunk->flags.locked);

  chunk->flags.ready = true;
  chunk->flags.locked = false;
  chunk = nullptr;

  return true;
}

bool ChunkTerrainGenWorker::is_finished() {
  return finished && chunk == nullptr;
}