#pragma once
#include <cstring>
#include <ctime>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <stdint.h>

#include "debug.hpp"
#include "math.hpp"
#include "types.hpp"

typedef glm::vec<3, i32> ChunkPos;
typedef glm::vec<3, i32> CubePos;
typedef glm::vec<3, i32> LocalPos;
typedef glm::vec<3, double> WorldPos;
typedef glm::vec<3, u8> LightLevel;

struct rgb {
  u8 r;
  u8 g;
  u8 b;

  auto operator<=>(const rgb& rhs) const = default;
};

struct rgba {
  u8 r;
  u8 g;
  u8 b;
  u8 a;

  auto operator<=>(const rgba& rhs) const = default;
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

[[maybe_unused]] static CubePos floor_position(WorldPos pos) {
  return CubePos{
      glm::floor(pos.x),
      glm::floor(pos.y),
      glm::floor(pos.z)};
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

struct RGBHasher {
  std::size_t operator()(const rgb& c) const {
    return c.r * 65536 + c.g * 256 + c.b;
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
