#pragma once
#include <chrono>
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <stdint.h>

// Type definitions

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

typedef glm::vec<3, i32> ChunkPos;
typedef glm::vec<3, i32> CubePos;
typedef glm::vec<3, i32> LocalPos;
typedef glm::vec<3, double> WorldPos;
typedef glm::vec<3, u8> LightLevel;

enum Dir {
  NONE = 0,
  LEFT = 1,
  RIGHT = 2,
  FRONT = 4,
  BACK = 8,
  TOP = 16,
  BOTTOM = 32,
};

template <typename T>
constexpr glm::vec<3, T> Vec3Left = {-1, 0, 0};

template <typename T>
constexpr glm::vec<3, T> Vec3Right = {1, 0, 0};

template <typename T>
constexpr glm::vec<3, T> Vec3Up = {0, 1, 0};

template <typename T>
constexpr glm::vec<3, T> Vec3Down = {0, -1, 0};

template <typename T>
constexpr glm::vec<3, T> Vec3Forward = {0, 0, -1};

template <typename T>
constexpr glm::vec<3, T> Vec3Back = {0, 0, 1};

// Debugging

template <typename T>
static void print(glm::vec<3, T> vec3) {
  std::cout << "[" << vec3.x << ", " << vec3.y << ", " << vec3.z << "]" << std::endl;
}

template <typename T>
static void print(std::vector<T> vec) {
  for (T elem : vec) {
    print(elem);
  }
}

template <typename Arg>
static void print(Arg&& arg) {
  std::cout << arg << std::endl;
}

template <typename... Args, typename FirstArg>
static void print(FirstArg&& first_arg, Args&&... args) {
  std::cout << first_arg;
  print(args...);
}

#define __FILENAME__ strrchr("/" __FILE__, '/') + 1

#define debug_log(...) \
  print("\033[1;36m", "[LOG] \033[1;37m", __FILENAME__, ":", __LINE__, " ", "\033[0m", __VA_ARGS__)

#define debug_log_no_filename(...) \
  print("\033[1;36m", "[LOG]\033[0m ", __VA_ARGS__)

#define debug_warn(...) \
  print("\033[1;33m", "[WARN] \033[1;37m", __FILENAME__, ":", __LINE__, " ", "\033[0m", __VA_ARGS__)

#define debug_error(...)                                                                               \
  print("\033[1;31m", "[ERROR] \033[1;37m", __FILENAME__, ":", __LINE__, " ", "\033[0m", __VA_ARGS__); \
  exit(1)

#define ensure(condition)               \
  do {                                  \
    if (!(condition)) {                 \
      debug_error("Assertion failed!"); \
    }                                   \
  } while (false);

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
      debug_log_no_filename("took ", milliseconds);
    } else {
      debug_log_no_filename(message, " took ", milliseconds);
    }
  }
};

// Utility

static constexpr Dir opposite_dir(Dir dir) {
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

  debug_error("opposite_dir(): invalid input direction");
  return Dir::NONE;
}

// FIXME: better hashing function
struct Vec2Hasher {
  template <typename T>
  std::size_t operator()(const glm::vec<2, T>& vec3) const {
    return vec3.x * 1048576 + vec3.y;
  }
};

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
static glm::vec<3, T> lerp_vec3(glm::vec<3, T> from, glm::vec<3, T> to, float a) {
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

template <typename T>
static void sort_vector_by_manhattan_distance(std::vector<glm::vec<3, T>>& vector, glm::vec<3, T> to) {
  using Vec3T = glm::vec<3, T>;
  std::sort(vector.begin(), vector.end(), [&](const Vec3T a, const Vec3T b) {
    ChunkPos first = glm::abs(to - a);
    ChunkPos second = glm::abs(to - b);
    return first.x + first.y + first.z < second.x + second.y + second.z;
  });
}