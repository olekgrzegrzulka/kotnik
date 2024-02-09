#pragma once
#include <atomic>
#include <vector>

class Chunk;
class ChunkRenderer;

// A thread-safe method of multithread chunk operations
struct ChunkWorker {
};

struct ChunkMeshWorker {
  bool run_job(Chunk* chunk);

  bool try_collecting();

  bool is_finished();

private:
  std::vector<Chunk*> surrounding_chunks;
  std::atomic<bool> finished = true;
};

struct ChunkTerrainGenWorker {
  bool run_job(Chunk* chunk);

  bool try_collecting();

  bool is_finished();

private:
  Chunk* chunk = nullptr;
  std::atomic<bool> finished = true;
};
