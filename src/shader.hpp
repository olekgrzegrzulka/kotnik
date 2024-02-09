#pragma once
#include <iostream>
#include <string>
#include "glad/glad.h"

GLuint compile_shader(std::string source, GLint type) {
  GLuint shader;
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

GLuint compile_vertex_shader(std::string source) {
  return compile_shader(source, GL_VERTEX_SHADER);
}

GLuint compile_fragment_shader(std::string source) {
  return compile_shader(source, GL_FRAGMENT_SHADER);
}
