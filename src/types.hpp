#pragma once
#include <stdint.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

enum Dir {
  NONE = 0,
  LEFT = 1,
  RIGHT = 2,
  FRONT = 4,
  BACK = 8,
  TOP = 16,
  BOTTOM = 32,
};

static constexpr Dir opposite_dir(Dir dir) {
  if (dir == Dir::LEFT) {
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
