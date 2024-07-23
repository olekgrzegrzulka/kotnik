#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <glm/glm.hpp>
#include "aabb.hpp"
#include "common.hpp"
#include "entity.hpp"
#include "input.hpp"
#include "world.hpp"

struct KeyState {
public:
  typedef std::variant<Input::Key, Input::Mouse> input_t;

  KeyState(input_t _key) : key(_key) {
  }

  bool just_pressed() const {
    return _just_pressed;
  }

  bool held() const {
    return _held;
  }

  bool just_released() const {
    return _just_released;
  }

  void update() {
    bool new_key_state = get_key();
    _just_pressed = false;
    _just_released = false;
    _held = false;
    if ((!key_state) && new_key_state) { _just_pressed = true; }
    if (key_state && (!new_key_state)) { _just_released = true; }
    if (key_state && new_key_state) { _held = true; }
    key_state = new_key_state;
  }

private:
  input_t key;
  bool key_state = false;
  bool _just_pressed = false;
  bool _just_released = false;
  bool _held = false;

  bool get_key() {
    if (auto value = std::get_if<Input::Key>(&key)) {
      return Input::key_pressed(*value);
    } else if (auto value2 = std::get_if<Input::Mouse>(&key)) {
      return Input::mouse_pressed(*value2);
    }
    warn("KeyState::get_key(): bad key");
    return false;
  }
};

class Player final : public Entity {
public:
  WorldPos camera_offset{0.0, 0.7, 0.0};
  WorldPos velocity{};
  double pitch = 0.0;
  double yaw = 0.0;
  bool is_flying = true;

  AABB aabb{{0.15, 0.9, 0.15}};
  AABB aabb_ground{{0.17, 0.1, 0.17}, {0.0, -0.83, 0.0}};

  CubeId cube_to_place = CubeId::DIRT;

  std::unordered_map<std::string, KeyState> key_states{
      {"strafe_left", {Input::Key::KEY_A}},
      {"strafe_right", {Input::Key::KEY_D}},
      {"ascend", {Input::Key::KEY_SPACE}},
      {"descend", {Input::Key::KEY_LEFT_SHIFT}},
      {"forward", {Input::Key::KEY_W}},
      {"back", {Input::Key::KEY_S}},
      {"destroy", {Input::Mouse::MOUSE_BUTTON_LEFT}},
      {"place", {Input::Mouse::MOUSE_BUTTON_RIGHT}},
      {"next_cube", {Input::Key::KEY_Q}},
      {"switch_flying", {Input::Key::KEY_F}},
      {"reload_lightning", {Input::Key::KEY_2}},
  };

public:
  Player(World& _world) : Entity(_world) {
    world_pos = {0.0, 50.0, 0.0};
  }

  ~Player() override {
  }

  WorldPos get_looking_dir(double rotated_by = 0.0) const {
    return glm::normalize(
        WorldPos{glm::cos(yaw + rotated_by) * glm::cos(pitch),
                 glm::sin(pitch),
                 glm::sin(yaw + rotated_by) * glm::cos(pitch)});
  };

  WorldPos get_walking_dir() const {
    if (!(input_held("forward") || input_held("back") || input_held("strafe_left") || input_held("strafe_right"))) return WorldPos(0.0);
    const double pi = std::numbers::pi_v<double>;
    double angle = std::atan2((double)input_held("back") - (double)input_held("forward"), (double)input_held("strafe_right") - (double)input_held("strafe_left"));
    angle += pi * 0.5;
    WorldPos dir = get_looking_dir(angle);
    dir.y = 0.0;
    return glm::normalize(dir);
  };

  bool input_held(const std::string& input) const {
    if (!key_states.contains(input)) {
      print("Player::input_held(): invalid input type of ", input);
      return false;
    }
    return key_states.at(input).held();
  }

  bool input_just_pressed(const std::string& input) {
    if (!key_states.contains(input)) {
      print("Player::input_held(): invalid input type of ", input);
      return false;
    }
    return key_states.at(input).just_pressed();
  }

  bool input_just_released(const std::string& input) {
    if (!key_states.contains(input)) {
      print("Player::input_held(): invalid input type of ", input);
      return false;
    }
    return key_states.at(input).just_released();
  }

  void handle_input() {
    const double pi = std::numbers::pi_v<double>;
    const double pitch_margin = 0.05;

    for (auto& [_, key] : key_states) {
      key.update();
    }

    yaw += Input::get_mouse_delta().x * 0.0014;
    pitch -= Input::get_mouse_delta().y * 0.0014;

    yaw = std::fmod(yaw, 2 * pi);
    pitch = std::fmod(pitch, 2 * pi);

    pitch = glm::clamp(
        pitch,
        -0.5 * pi + pitch_margin,
        +0.5 * pi - pitch_margin);

    if (input_just_released("switch_flying")) {
      is_flying = !is_flying;
    }

    if (input_just_released("reload_lightning")) {
      world.request_player_chunk_light_update();
    }
  }

  virtual void update() override {
    handle_input();

    update_velocity();

    handle_collisions();

    world_pos += velocity;

    handle_cube_manipulation();
  }

  std::optional<CubePos> get_cube_indicator_pos() const {

    std::optional<CubePos> raycast_pos = world.raycast_get_solid_cube(world_pos + camera_offset, world_pos + camera_offset + get_looking_dir() * 5.0);
    // If player is looking directly at a cube, then we return it
    if (raycast_pos.has_value()) {
      return raycast_pos;
      // Else we find a neigbour of the cube on which we are standing (excluding top and bottom ones) and see which one the player is looking towards the most

    } else {
      CubePos standing_cube_pos = floor_position(world_pos) - CubePos(0, 1, 0);
      if (!world.is_solid(standing_cube_pos)) { return {}; }

      std::vector<CubePos> neigh_cubes = {
          standing_cube_pos + CubePos{-1, 0, 0},
          standing_cube_pos + CubePos{1, 0, 0},
          standing_cube_pos + CubePos{0, 0, -1},
          standing_cube_pos + CubePos{0, 0, 1},
      };
      // If the dot product of local position of neigbour and player looking vector is high enough, we return the neigbour cube
      for (auto neigh_cube : neigh_cubes) {
        if (!world.is_solid(neigh_cube)) {
          WorldPos neigh_pos = WorldPos{neigh_cube} + WorldPos{0.5, 0.5, 0.5};
          double dot = glm::dot(glm::normalize(neigh_pos - world_pos), glm::normalize(get_looking_dir()));
          if (dot > 0.97) {
            return neigh_cube;
          }
        }
      }
    }

    return {};
  }

  void handle_cube_manipulation() {
    std::optional<CubePos> indicator_pos = get_cube_indicator_pos();

    if (input_just_pressed("next_cube")) {
      cube_to_place = (CubeId)(((int)cube_to_place + 1) % ((int)CubeId::CUBE_ID_SIZE));
    }

    // Cube breaking
    if (input_just_pressed("destroy")) {
      if (indicator_pos.has_value()) {
        world.set_cube(indicator_pos.value(), CubeId::AIR);
      }
    }

    // Cube placing
    if (input_just_pressed("place")) {
      CubePos raycast_pos = floor_position(world.query_raycast_solid(world_pos + camera_offset, world_pos + camera_offset + get_looking_dir() * 5.0));
      std::vector<CubePos> poses = {raycast_pos};
      if (indicator_pos.has_value()) { poses.emplace_back(indicator_pos.value()); }

      for (auto cube_pos : poses) {
        if (world.is_solid(cube_pos) || !world.has_solid_neigbour(cube_pos) || world.aabb_is_overlapping_cube(aabb, world_pos, cube_pos)) { continue; }
        world.set_cube(floor_position(cube_pos), cube_to_place);
        break;
      }
    }
  }

  void update_velocity() {
    const double walking_speed = (is_flying) ? 0.5 : 0.1;
    static const double jump_power = 0.18;

    auto walking_vector = (get_walking_dir() * walking_speed);

    velocity.x = std::lerp(velocity.x, walking_vector.x, 0.2);
    velocity.z = std::lerp(velocity.z, walking_vector.z, 0.2);

    if (glm::length(walking_vector) == 0.0) {
      if (std::abs(velocity.x) < 0.0001) {
        velocity.x *= 0.5;
      }
      if (std::abs(velocity.y) < 0.0001) {
        velocity.y *= 0.5;
      }
    }

    if (is_flying) {
      velocity.y += 0.02 * (input_held("ascend") - input_held("descend"));
      velocity.y *= 0.92;
    } else {
      if (velocity.y == 0.0 && world.aabb_get_solid_cubes(aabb_ground, world_pos).size() > 0) {
        velocity.y += jump_power * (input_held("ascend"));
      }
      if (!is_flying) { velocity.y += world.physical_properties.gravity; }
      velocity.y -= world.physical_properties.air_friction * std::pow(velocity.y, 2.0) * glm::sign(velocity.y);
      if (glm::length(velocity) < 0.03) { velocity *= 0.99; }
    }
  }

  void handle_collisions() {
    WorldPos player_target_position = world_pos + velocity;
    WorldPos move_vector_individual_axis{};
    const int ITERATIONS = 4;
    for (int i = 0; i <= ITERATIONS; i += 1) {
      const double a = (double)i / ITERATIONS;
      const WorldPos test_position = world_pos + WorldPos{a * (player_target_position.x - world_pos.x), 0, 0};
      if (world.aabb_get_solid_cubes(aabb, test_position).size() > 0) {
        break;
      }
      move_vector_individual_axis.x = test_position.x - world_pos.x;
    }

    for (int i = 0; i <= ITERATIONS; i += 1) {
      const double a = (double)i / ITERATIONS;
      const WorldPos test_position = world_pos + WorldPos{0, a * (player_target_position.y - world_pos.y), 0};
      if (world.aabb_get_solid_cubes(aabb, test_position).size() > 0) {
        break;
      }
      move_vector_individual_axis.y = test_position.y - world_pos.y;
    }

    for (int i = 0; i <= ITERATIONS; i += 1) {
      const double a = (double)i / ITERATIONS;
      const WorldPos test_position = world_pos + WorldPos{0, 0, a * (player_target_position.z - world_pos.z)};
      if (world.aabb_get_solid_cubes(aabb, test_position).size() > 0) {
        break;
      }
      move_vector_individual_axis.z = test_position.z - world_pos.z;
    }

    velocity = move_vector_individual_axis;

    // To prevent getting stuck inside cubes, push out the player away from average position of all overlapping cubes
    for (int i = 20; i > 0 && world.aabb_get_solid_cubes(aabb, world_pos + velocity).size() > 0; i -= 1) {
      velocity -= move_vector_individual_axis * 0.05;
    }
  }
};