#pragma once
#include <limits>
#include <random>
#include <span>
#include "common.hpp"

class Random {
private:
  std::default_random_engine rng;

public:
  Random() {
    std::random_device rd;
    rng.seed(rd());
  }

  Random(i32 seed) {
    rng.seed(seed);
  }

  template <typename T>
  T next(i32 from = std::numeric_limits<i32>::min(), i32 to = std::numeric_limits<i32>::max()) {
    std::uniform_int_distribution<i32> distribution(from, to);
    return distribution(rng);
  }

  template <class T>
  T pick(std::span<T> values, std::span<int> weights) {
    ensure(values.size() == weights.size());

    std::discrete_distribution<int> distribution(weights.begin(), weights.end());
    return values[distribution(rng)];
  }
};

namespace StaticRandom {
static Random& get() {
  thread_local Random r;
  return r;
}
}; // namespace StaticRandom
