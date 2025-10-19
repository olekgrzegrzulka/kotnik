#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include "../common.hpp"
#include "widget.hpp"

class UI;

struct vertex_sprite {
  glm::vec<2, float> pos;
  glm::vec<2, float> uv;
  float nine_slice_margin = 0.0f;
  float nine_slice_scale = 1.0f;
  glm::vec<2, u32> widget_size = {1.0f, 1.0f};
  glm::vec<2, float> uv_start = {0.0f, 0.0f};
  glm::vec<2, float> uv_end = {1.0f, 1.0f};
};

class Sprite : public Widget {
protected:
  float nine_slice_margin = 0.0f;
  float nine_slice_scale = 1.0f;
  glm::vec<2, float> uv_start = {0.0f, 0.0f};
  glm::vec<2, float> uv_end = {1.0f, 1.0f};

protected:
  u32 vbo = 0;
  u32 vao = 0;
  std::vector<vertex_sprite> vertices;

public:
  Sprite(const UI& ui_);

  ~Sprite() override;

  void update() override;

  void draw() override;

  void update_mesh();

  void setup_buffers();

  WIDGET_DEF_SETTER_DIRTY(nine_slice_margin)
  WIDGET_DEF_SETTER_DIRTY(nine_slice_scale)
  WIDGET_DEF_SETTER_DIRTY(uv_start)
  WIDGET_DEF_SETTER_DIRTY(uv_end)

  WIDGET_DEF_GETTER(nine_slice_margin)
  WIDGET_DEF_GETTER(nine_slice_scale)
  WIDGET_DEF_GETTER(uv_start)
  WIDGET_DEF_GETTER(uv_end)
};