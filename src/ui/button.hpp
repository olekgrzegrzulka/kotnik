#pragma once
#include <functional>
#include <string>
#include "../glad/glad.h"
#include "label.hpp"
#include "sprite.hpp"
#include "widget.hpp"

class UI;

enum class ButtonState {
  IDLE,
  HOVERED,
  PRESSED,
  DISABLED,
};

class Button : public Sprite {
protected:
  Label& label;
  std::function<void()> lambda = nullptr;
  ButtonState state;

  glm::vec2 uv_start_idle = glm::vec2(0.0f / 16.0f, 0.0f / 16.0f);
  glm::vec2 uv_end_idle = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f);
  glm::vec2 uv_start_hovered = glm::vec2(0.0f / 16.0f, 1.0f / 16.0f);
  glm::vec2 uv_end_hovered = glm::vec2(1.0f / 16.0f, 2.0f / 16.0f);
  glm::vec2 uv_start_pressed = glm::vec2(0.0f / 16.0f, 2.0f / 16.0f);
  glm::vec2 uv_end_pressed = glm::vec2(1.0f / 16.0f, 3.0f / 16.0f);
  glm::vec2 uv_start_disabled = glm::vec2(0.0f / 16.0f, 3.0f / 16.0f);
  glm::vec2 uv_end_disabled = glm::vec2(1.0f / 16.0f, 4.0f / 16.0f);

public:
  Button(const UI& ui_) : Sprite::Sprite(ui_), label(add_child<Label>("")) {
    set_sprite_idle();
    set_nine_slice_margin(3.0f);
    set_nine_slice_scale(1.0f);
  }

  Button(const UI& ui_, std::string label_) : Sprite::Sprite(ui_), label(add_child<Label>(label_)) {
    set_sprite_idle();
    set_nine_slice_margin(3.0f);
    set_nine_slice_scale(1.0f);
  }

  Label& get_label() { return label; }

  virtual void update() override;

  virtual void draw() override {
    Sprite::draw();
  }

  void on_press(std::function<void()> lambda_) {
    lambda = lambda_;
  }

  void set_disabled(bool disabled) {
    if (disabled) {
      state = ButtonState::DISABLED;
    } else {
      state = ButtonState::IDLE;
    }
  }

  WIDGET_DEF_GETTER(state);

  void set_uv_start_idle(glm::vec2 to) {
    if (uv_start_idle == to) { return; }
    uv_start_idle = to;
    if (state == ButtonState::IDLE) { dirty = true; }
  }
  void set_uv_end_idle(glm::vec2 to) {
    if (uv_end_idle == to) { return; }
    uv_end_idle = to;
    if (state == ButtonState::IDLE) { dirty = true; }
  }
  void set_uv_start_hovered(glm::vec2 to) {
    if (uv_start_hovered == to) { return; }
    uv_start_hovered = to;
    if (state == ButtonState::HOVERED) { dirty = true; }
  }
  void set_uv_end_hovered(glm::vec2 to) {
    if (uv_end_hovered == to) { return; }
    uv_end_hovered = to;
    if (state == ButtonState::HOVERED) { dirty = true; }
  }
  void set_uv_start_pressed(glm::vec2 to) {
    if (uv_start_pressed == to) { return; }
    uv_start_pressed = to;
    if (state == ButtonState::PRESSED) { dirty = true; }
  }
  void set_uv_end_pressed(glm::vec2 to) {
    if (uv_end_pressed == to) { return; }
    uv_end_pressed = to;
    if (state == ButtonState::PRESSED) { dirty = true; }
  }
  void set_uv_start_disabled(glm::vec2 to) {
    if (uv_start_disabled == to) { return; }
    uv_start_disabled = to;
    if (state == ButtonState::DISABLED) { dirty = true; }
  }
  void set_uv_end_disabled(glm::vec2 to) {
    if (uv_end_disabled == to) { return; }
    uv_end_disabled = to;
    if (state == ButtonState::DISABLED) { dirty = true; }
  }

protected:
  void set_sprite_idle() {
    set_uv_start(uv_start_idle);
    set_uv_end(uv_end_idle);
  }

  void set_sprite_hovered() {
    set_uv_start(uv_start_hovered);
    set_uv_end(uv_end_hovered);
  }

  void set_sprite_pressed() {
    set_uv_start(uv_start_pressed);
    set_uv_end(uv_end_pressed);
  }

  void set_sprite_disabled() {
    set_uv_start(uv_start_disabled);
    set_uv_end(uv_end_disabled);
  }

  void pressed() {
    if (lambda) { lambda(); }
  }

  // Returns true if button's state was changed
  bool set_state(ButtonState state_) {
    if (state_ == state) { return false; }
    auto prev_state = state;
    state = state_;
    on_state_changed(prev_state);
    return true;
  }

  void on_state_changed(ButtonState /* prev_state */) {
    if (state == ButtonState::HOVERED) {
      set_sprite_hovered();
    } else if (state == ButtonState::IDLE) {
      set_sprite_idle();
    } else if (state == ButtonState::PRESSED) {
      set_sprite_pressed();
    } else if (state == ButtonState::DISABLED) {
      set_sprite_disabled();
    }
  }
};