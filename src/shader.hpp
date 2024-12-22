#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>
#include "common.hpp"
#include "glad/glad.h"

class Shader {
public:
  Shader(std::string file_name) {
    vertex_shader = compile_vertex_shader(read_shader_file(file_name + ".vert"));
    fragment_shader = compile_fragment_shader(read_shader_file(file_name + ".frag"));
    shader_program = create_shader_program();

    GLint uniform_count = 0;
    glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &uniform_count);
    for (GLint i = 0; i < uniform_count; i++) {
      char name[256];
      GLsizei length;
      glGetActiveUniform(shader_program, i, sizeof(name), &length, nullptr, nullptr, name);
      std::string uniform_name{name};
      ensure(!uniforms.contains(uniform_name));
      uniforms[uniform_name] = glGetUniformLocation(shader_program, uniform_name.c_str());
    }
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

  void use() const {
    glUseProgram(shader_program);
  }

  void set_uniform_u32(std::string name, u32 value) const {
    auto location = get_uniform_location(name);
    glUniform1ui(location, value);
  }

  void set_uniform_u32(std::string name, u32 value1, u32 value2) const {
    auto location = get_uniform_location(name);
    glUniform2ui(location, value1, value2);
  }

  void set_uniform_u32(std::string name, u32 value1, u32 value2, u32 value3) const {
    auto location = get_uniform_location(name);
    glUniform3ui(location, value1, value2, value3);
  }

  void set_uniform_i32(std::string name, i32 value) const {
    auto location = get_uniform_location(name);
    glUniform1i(location, value);
  }

  void set_uniform_i32(std::string name, i32 value1, i32 value2) const {
    auto location = get_uniform_location(name);
    glUniform2i(location, value1, value2);
  }

  void set_uniform_i32(std::string name, i32 value1, i32 value2, i32 value3) const {
    auto location = get_uniform_location(name);
    glUniform3i(location, value1, value2, value3);
  }

  void set_uniform_float(std::string name, float value) const {
    auto location = get_uniform_location(name);
    glUniform1f(location, value);
  }

  void set_uniform_float(std::string name, float value1, float value2) const {
    auto location = get_uniform_location(name);
    glUniform2f(location, value1, value2);
  }

  void set_uniform_float(std::string name, float value1, float value2, float value3) const {
    auto location = get_uniform_location(name);
    glUniform3f(location, value1, value2, value3);
  }

  void set_uniform_mat4(std::string name, glm::mat4 value) const {
    auto location = get_uniform_location(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }

private:
  GLint get_uniform_location(const std::string& name) const {
    auto it = uniforms.find(name);
    if (it == uniforms.end()) {
      debug_error("No uniform named ", name, " in shader");
      return -1;
    }
    return it->second;
  }

  std::string read_shader_file(std::string file_name) const {
    std::ifstream file{"./shaders/" + file_name};

    if (!file.is_open()) {
      debug_error("Failed to open shader file " + file_name);
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

      debug_error("Shader compilation error:\n", info_log);
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
  std::unordered_map<std::string, GLint> uniforms;
};