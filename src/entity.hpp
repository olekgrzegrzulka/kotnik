#pragma once
#include "chunk.hpp"
#include "common.hpp"

class World;

class Entity {
public:
  WorldPos world_pos;
  World& world;

public:
  Entity(World& _world) : world(_world) {}
  virtual ~Entity() {}
  virtual void update() {}

  WorldPos get_world_pos() {
    return world_pos;
  }
};