#pragma once
#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include "../common.hpp"
#include "../glad/glad.h"
#include "../vertex.hpp"
#include "widget.hpp"

class UI;

class Label final : public Widget {
public:
  static constexpr i32 max_text_length = 512;

private:
  std::string text;
  GLuint vao = 0;
  GLuint vbo = 0;
  std::vector<vertex2> vertices;
  glm::vec3 text_color = {1.0, 1.0, 1.0};

public:
  Label(const UI&, std::string);

  ~Label() override;

  WIDGET_DEF_SETTER_DIRTY(text);
  WIDGET_DEF_SETTER_DIRTY(text_color);

  void update() override;

  void draw() override;

protected:
  void update_mesh();

  void setup_buffers();
};