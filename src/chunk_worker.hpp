#pragma once
#include <atomic>
#include <memory>
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
  std::atomic<bool> finished = true;
};

struct ChunkTerrainGenWorker {
  ChunkTerrainGenWorker(std::shared_ptr<WorldGen> _world_gen) : world_gen(_world_gen) {}

  std::shared_ptr<WorldGen> world_gen;

  void add_to_queue(ChunkPos);

  void update();

  std::vector<std::unique_ptr<Chunk>> collect_finished_chunks();

private:
  std::optional<ChunkPos> pop_from_queue();

  std::atomic<bool> is_running = false;

  std::vector<ChunkPos> chunk_queue;
  std::mutex chunk_queue_mutex;

  std::vector<std::unique_ptr<Chunk>> chunks_finished;
  std::mutex chunks_finished_mutex;
};