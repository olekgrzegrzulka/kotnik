#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include "glad/glad.h"

static std::string read_file(const std::string& filename) {
  std::ifstream file(filename, std::ios::in);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file '" + filename + "'!");
  }
  std::string source;
  source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  return source;
}

static GLuint compile_shader(std::string source, GLint type) {
  static GLuint shader;
  shader = glCreateShader(type);
  const char* test = source.c_str();
  glShaderSource(shader, 1, &test, nullptr);
  glCompileShader(shader);

  // Check compilation
  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    char* infoLog = new char[maxLength];
    glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

    std::cout << "Shader compilation error:\n";
    std::cout << infoLog << "\n";

    delete[] infoLog;
    glDeleteShader(shader);
    exit(1);
  }

  return shader;
}

static GLuint compile_vertex_shader(std::string source) {
  return compile_shader(source, GL_VERTEX_SHADER);
}

GLuint compile_fragment_shader(std::string source) {
  return compile_shader(source, GL_FRAGMENT_SHADER);
}
