#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stdexcept>
#include "glad/glad.h"
#include "stb_image.h"

GLuint load_texture(const char* path) {
  int width, height, channels;
  stbi_uc* data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

  if (data == nullptr) {
    throw std::runtime_error("Failed to load texture" + std::string(path));
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);

  return texture;
}

GLuint create_sampler() {
  GLuint texture_sampler;
  glCreateSamplers(1, &texture_sampler);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return texture_sampler;
}