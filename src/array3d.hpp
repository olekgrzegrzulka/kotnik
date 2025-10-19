#pragma once
#include <array>
#include <cstddef>
#include <glm/vec3.hpp>
#include "common.hpp"

template <class T, size_t W, size_t H, size_t D>
class Array3D {
  static constexpr size_t Size = W * H * D;

public:
  constexpr Array3D() {
    begin_x = 0;
    begin_y = 0;
    begin_z = 0;
    end_x = W;
    end_y = H;
    end_z = D;
  }

  constexpr Array3D(i32 begin_x_, i32 begin_y_, i32 begin_z_) {
    begin_x = begin_x_;
    begin_y = begin_y_;
    begin_z = begin_z_;
    end_x = begin_x_ + W;
    end_y = begin_y_ + H;
    end_z = begin_z_ + D;
  }

  constexpr const T& at(glm::vec<3, i32> at) const {
    ensure(has_index(at));
    at.x -= begin_x;
    at.y -= begin_y;
    at.z -= begin_z;

    ensure(at.x + at.y * W + at.z * W * H >= 0 && at.x + at.y * W + at.z * W * H < Size);
    return data.at(at.x + at.y * W + at.z * W * H);
  }

  template <typename... Args>
  constexpr void set(glm::vec<3, int> at, Args... to) {
    ensure(has_index(at));
    at.x -= begin_x;
    at.y -= begin_y;
    at.z -= begin_z;

    ensure(at.x + at.y * W + at.z * W * H >= 0 && at.x + at.y * W + at.z * W * H < Size);
    data[at.x + at.y * W + at.z * W * H] = std::forward<T>(to...);
  }

  constexpr bool has_index(glm::vec<3, int> at) const {
    return at.x >= begin_x && at.y >= begin_y && at.z >= begin_z &&
           at.x < end_x && at.y < end_y && at.z < end_z;
  }

  template <typename... Args>
  constexpr void fill(Args... value) {
    data.fill(value...);
  }

private:
  int begin_x = 0;
  int begin_y = 0;
  int begin_z = 0;

  int end_x = 0;
  int end_y = 0;
  int end_z = 0;

  std::array<T, Size> data{};
};