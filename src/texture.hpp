#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stdexcept>
#include "glad/glad.h"
#include "stb_image.h"

GLuint load_texture(const char* path) {
  int width, height, channels;
  stbi_uc* image = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

  if (image == nullptr) {
    throw std::runtime_error("Failed to load texture" + std::string(path));
  }

  GLuint texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glTextureStorage2D(texture, 4, GL_RGBA8, width, height);
  glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glGenerateTextureMipmap(texture);

  stbi_image_free(image);

  return texture;
}

GLuint create_sampler(const char* path) {
  GLuint texture = load_texture(path);

  GLuint texture_sampler;
  glCreateSamplers(1, &texture_sampler);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(texture_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return texture_sampler;
}