#include <vector>
#include "glad/glad.h"
#include "shader.hpp"
#include "types.hpp"

class FrameBuffer final {
public:
  std::vector<float> vertices = {
      -1.0f, -1.0f, +0.0f, +0.0f,
      +1.0f, -1.0f, +1.0f, +0.0f,
      +1.0f, +1.0f, +1.0f, 1.0f,
      -1.0f, +1.0f, +0.0f, 1.0f};
  std::vector<GLuint> vertex_indices = {0, 1, 2, 2, 3, 0};

  Shader shader;
  GLuint texture, depth;
  GLuint fbo, vbo, vao, ebo;
  GLuint sampler;

  i32 width = 0;
  i32 height = 0;

  FrameBuffer(i32 width_, i32 height_);

  void resize(i32 width_, i32 height_);

  void attach();

  void draw(i32 width_, i32 height_);
};
