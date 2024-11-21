#pragma once
#include <cstddef>
#include <vector>
#include "common.hpp"
#include "glm/vec3.hpp"

template <class T>
class Array3D {
public:
  Array3D() = default;

  Array3D(size_t w, size_t d, size_t h) {
    end_x = w;
    end_y = d;
    end_z = h;

    data.resize(w * d * h);
  }

  Array3D(int begin_x_, int begin_y_, int begin_z_, int end_x_, int end_y_, int end_z_) {
    ensure(end_x_ >= begin_x_);
    ensure(end_y_ >= begin_y_);
    ensure(end_z_ >= begin_z_);

    begin_x = begin_x_;
    begin_y = begin_y_;
    begin_z = begin_z_;
    end_x = end_x_;
    end_y = end_y_;
    end_z = end_z_;

    size_t array_size = (end_x - begin_x) * (end_y - begin_y) * (end_z - begin_z);
    data.resize(array_size);
  }

  T at(glm::vec<3, int> at) const {
    ensure(has_index(at));
    at.x -= begin_x;
    at.y -= begin_y;
    at.z -= begin_z;

    return data.at(at.x + at.y * (end_x - begin_x) + at.z * (end_x - begin_x) * (end_y - begin_y));
  }

  void set(glm::vec<3, int> at, T to) {
    ensure(has_index(at));
    at.x -= begin_x;
    at.y -= begin_y;
    at.z -= begin_z;

    data[at.x + at.y * (end_x - begin_x) + at.z * (end_x - begin_x) * (end_y - begin_y)] = to;
  }

  bool has_index(glm::vec<3, int> at) const {
    return at.x >= begin_x && at.y >= begin_y && at.z >= begin_z &&
           at.x < end_x && at.y < end_y && at.z < end_z;
  }

private:
  int begin_x = 0;
  int begin_y = 0;
  int begin_z = 0;

  int end_x = 0;
  int end_y = 0;
  int end_z = 0;

  std::vector<T> data;
};