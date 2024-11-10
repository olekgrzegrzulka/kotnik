#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include "world_gen.hpp"

class Chunk;
class ChunkRenderer;

// A thread-safe method of multithread chunk operations
struct ChunkWorker {
};

struct ChunkMeshWorker {
  bool run_job(ChunkPos, World&);

  bool try_collecting();

  bool is_finished();

private:
  std::vector<Chunk*> surrounding_chunks;
  std::atomic<bool> finished = true;
};

struct ChunkTerrainGenWorker {
  ChunkTerrainGenWorker(std::shared_ptr<WorldGen> _world_gen) : world_gen(_world_gen) {}

  std::shared_ptr<WorldGen> world_gen;

  void add_to_queue(Chunk* chunk);

  void update();

private:
  Chunk* pop_from_queue();

  std::vector<Chunk*> chunk_queue;
  std::vector<Chunk*> chunks_finished;
  std::mutex chunk_queue_mutex;
  std::mutex chunks_finished_mutex;
  std::atomic<bool> is_running = false;
};