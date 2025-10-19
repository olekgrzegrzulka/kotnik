#pragma once
#include <atomic>
#include <condition_variable>
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

  ChunkMeshWorker();
  ~ChunkMeshWorker();

  bool add_to_queue(ChunkPos, World&);

  void sort_queue_by_distance(ChunkPos);

  size_t get_queue_size();

  std::vector<std::pair<ChunkPos, std::unique_ptr<ChunkMesh>>> collect_finished_chunks();

private:
  bool add_to_queue_no_mutex(ChunkPos, World&);
  void sort_queue_by_distance_no_mutex(ChunkPos);
  void lock_queue();
  void unlock_queue();

  std::thread thread;
  std::atomic<bool> kill_thread = false;

  std::vector<chunk_queue_t> chunk_queue;
  std::mutex chunk_queue_mutex;
  std::condition_variable cond_var;

  std::vector<std::pair<ChunkPos, std::unique_ptr<ChunkMesh>>> chunks_finished;
  std::mutex chunks_finished_mutex;
};

struct ChunkTerrainGenWorker {
  ChunkTerrainGenWorker(std::shared_ptr<WorldGen>);
  ~ChunkTerrainGenWorker();

  std::shared_ptr<WorldGen> world_gen;

  void add_to_queue(glm::vec<2, int>);

  std::vector<std::vector<std::unique_ptr<Chunk>>> collect_finished_chunks();

private:
  std::thread thread;
  std::atomic<bool> kill_thread = false;

  std::vector<glm::vec<2, int>> chunk_queue;
  std::mutex chunk_queue_mutex;
  std::condition_variable cond_var;

  std::vector<std::vector<std::unique_ptr<Chunk>>> chunks_finished;
  std::mutex chunks_finished_mutex;
};