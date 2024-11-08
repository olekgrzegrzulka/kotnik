#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include "common.hpp"
#include "glad/glad.h"

struct Shader {
public:
  Shader(std::string file_name) {
    vertex_shader = compile_vertex_shader(read_shader_file(file_name + ".vert"));
    fragment_shader = compile_fragment_shader(read_shader_file(file_name + ".frag"));
    shader_program = create_shader_program();
  }

  ~Shader() {
    if (vertex_shader != 0) {
      glDeleteShader(vertex_shader);
    }

    if (fragment_shader != 0) {
      glDeleteShader(fragment_shader);
    }

    if (shader_program != 0) {
      glDeleteProgram(shader_program);
    }
  };

  void use() {
    glUseProgram(shader_program);
  }

private:
  std::string read_shader_file(std::string file_name) const {
    std::ifstream file{"./shaders/" + file_name};

    if (!file.is_open()) {
      debug_error("failed to open shader file " + file_name);
    }

    std::stringstream file_string;
    file_string << file.rdbuf();

    return file_string.str();
  }

  GLuint compile_shader(std::string source_, GLint type) const {
    GLuint shader;
    shader = glCreateShader(type);
    const char* source = source_.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check compilation
    GLint compile_status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE) {
      GLint max_log_length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_log_length);

      char* info_log = new char[max_log_length];
      glGetShaderInfoLog(shader, max_log_length, &max_log_length, info_log);

      debug_error("shader compilation error:\n", info_log);
      delete[] info_log;
    }

    return shader;
  }

  GLuint compile_vertex_shader(std::string source) const {
    return compile_shader(source, GL_VERTEX_SHADER);
  }

  GLuint compile_fragment_shader(std::string source) const {
    return compile_shader(source, GL_FRAGMENT_SHADER);
  }

  GLuint create_shader_program() const {
    GLuint shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glLinkProgram(shader_program_);
    return shader_program_;
  }

public:
  GLuint shader_program{};
  GLuint vertex_shader{};
  GLuint fragment_shader{};
};