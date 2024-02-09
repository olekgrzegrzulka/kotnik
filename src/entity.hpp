#pragma once
#include "common.hpp"
#include "world.hpp"

class Entity {
public:
  WorldPos world_pos;
  World& world;

public:
  Entity(World& _world) : world(_world) {}
  virtual ~Entity() {}
  virtual void update() {}
};