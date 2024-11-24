#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include "chunk.hpp"
#include "chunk_mesh.hpp"
#include "world_gen.hpp"

class Chunk;
class ChunkRenderer;

struct ChunkMeshWorker {
  using chunk_queue_t = std::pair<ChunkPos, std::unique_ptr<ChunkMeshData>>;

  ChunkMeshWorker() = default;

  bool add_to_queue(ChunkPos, World&);
  bool add_to_queue_no_mutex(ChunkPos, World&);

  void sort_queue_by_distance(ChunkPos);
  void sort_queue_by_distance_no_mutex(ChunkPos);

  void lock_queue();
  void unlock_queue();

  size_t get_queue_size();

  void update();

  std::vector<std::pair<ChunkPos, std::unique_ptr<ChunkMesh>>> collect_finished_chunks();

private:
  std::optional<chunk_queue_t> pop_from_queue();

  std::atomic<bool> is_running = false;

  std::vector<chunk_queue_t> chunk_queue;
  std::mutex chunk_queue_mutex;

  std::vector<std::pair<ChunkPos, std::unique_ptr<ChunkMesh>>> chunks_finished;
  std::mutex chunks_finished_mutex;
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