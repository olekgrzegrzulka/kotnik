#pragma once
#include <chrono>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <glm/glm.hpp>
#include <stdint.h>

typedef glm::vec<3, int32_t> ChunkPos;
typedef glm::vec<3, int32_t> CubePos;
typedef glm::vec<3, int32_t> LocalPos;
typedef glm::vec<3, double> WorldPos;
typedef glm::vec<3, uint8_t> LightLevel;

enum class CubeId : uint16_t {
  AIR = 0,
  DIRT,
  GRASS,
  STONE,
  SAND,
  GRAVEL,
  WOOD,
  LEAVES,
  GRASS_PLANT,

  CUBE_ID_SIZE,
};

enum Dir {
  NONE = 0,
  LEFT = 1,
  RIGHT = 2,
  FRONT = 4,
  BACK = 8,
  TOP = 16,
  BOTTOM = 32,
};

static Dir opposite_dir(Dir dir) {
  if (dir == Dir::NONE) {
    return Dir::NONE;
  } else if (dir == Dir::LEFT) {
    return Dir::RIGHT;
  } else if (dir == Dir::RIGHT) {
    return Dir::LEFT;
  } else if (dir == Dir::FRONT) {
    return Dir::BACK;
  } else if (dir == Dir::BACK) {
    return Dir::FRONT;
  } else if (dir == Dir::TOP) {
    return Dir::BOTTOM;
  } else if (dir == Dir::BOTTOM) {
    return Dir::TOP;
  }

  return Dir::NONE;
}

enum CubeFlag {
  SOLID = 1,
};

template <typename T>
void print(glm::vec<3, T> vec3) {
  std::cout << "[" << vec3.x << ", " << vec3.y << ", " << vec3.z << "]" << std::endl;
}

// FIXME: better hashing function
struct Vec3Hasher {
  template <typename T>
  std::size_t operator()(const glm::vec<3, T>& vec3) const {
    return vec3.x * 1048576 + vec3.y * 1024 + vec3.z;
  }
};

struct Vec3Comparator {
  template <typename T>
  bool operator()(const glm::vec<3, T>& lhs, const glm::vec<3, T>& rhs) const {
    return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y) || (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
  }
};

template <typename T>
void print(std::vector<T> vec) {
  for (T elem : vec) {
    print(elem);
  }
}

// typedef std::set<LocalPos, Vec3Comparator> CubeSet;
// typedef std::map<LocalPos, Vec3Comparator> CubeMap;

template <typename Arg>
void print(Arg&& arg) {
  std::cout << arg << std::endl;
}

template <typename... Args, typename FirstArg>
static void print(FirstArg&& first_arg, Args&&... args) {
  std::cout << first_arg << " ";
  print(args...);
}

template <typename T>
glm::vec<3, T> lerp_vec3(glm::vec<3, T> from, glm::vec<3, T> to, float a) {
  return glm::vec<3, T>{
      std::lerp(from.x, to.x, a),
      std::lerp(from.y, to.y, a),
      std::lerp(from.z, to.z, a)};
}

static CubePos floor_position(WorldPos pos) {
  return CubePos{
      glm::floor(pos.x),
      glm::floor(pos.y),
      glm::floor(pos.z)};
}

struct ScopeTimer {
  std::string message;
  std::chrono::time_point<std::chrono::system_clock> start_time;

  ScopeTimer(std::string _message = "") : message(_message) {
    start_time = std::chrono::high_resolution_clock::now();
  }

  ~ScopeTimer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (message.empty()) {
      std::cout << "Took " << milliseconds << "." << std::endl;
    } else {
      std::cout << message << " took " << milliseconds << "." << std::endl;
    }
  }
};