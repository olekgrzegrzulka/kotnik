#include <cmath>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <glm/vec3.hpp>
#include "../glad/glad.h"
#include "../stb_image.h"
// #include "../stb_image_write.h"
#include "../debug.hpp"
#include "../types.hpp"
#include "texture_atlas.hpp"

TextureAtlas::TextureAtlas() {
  image.resize(4 * atlas_size * atlas_size);
  free_16x16_squares.resize((atlas_size / 16) * atlas_size / 16, true);
}

bool TextureAtlas::add_texture(std::string id, std::string path) {
  if (textures.contains(id)) { return false; }

  i32 width, height, channels;
  stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  if (!data) {
    debug_error("failed to load texture" + path);
  }

  auto at = paste_texture(data, width, height);

  TextureAtlasData uv;
  uv.start = {at.x / (float)atlas_size - half_pixel(), at.y / (float)atlas_size - half_pixel()};
  uv.end = {(at.x + width) / (float)atlas_size + half_pixel(), (at.y + height) / (float)atlas_size + half_pixel()};
  uv.width = width;
  uv.height = height;
  textures[id] = uv;

  stbi_image_free(data);

  return true;
}

// void TextureAtlas::save_to_file(std::string filename) {
//   if (!filename.ends_with(".png")) { filename += ".png"; }
//   stbi_write_png(filename.c_str(), atlas_size, atlas_size, 4, image.data(), 4 * atlas_size);
// }

std::optional<std::reference_wrapper<TextureAtlasData>> TextureAtlas::get(std::string id) {
  auto it = textures.find(id);
  if (it != textures.end()) {
    return it->second;
  } else {
    return std::nullopt;
  }
}

void TextureAtlas::bind(u32 slot) {
  if (dirty) { regenerate_texture(); }
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindSampler(slot, sampler);
}

void TextureAtlas::regenerate_texture() {
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, atlas_size, atlas_size);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas_size, atlas_size, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  glCreateSamplers(1, &sampler);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  dirty = false;
}

std::optional<std::pair<i32, i32>> TextureAtlas::find_space_for_texture(i32 width, i32 height) {
  for (i32 square_x = 0; square_x < (atlas_size / 16); square_x += 1) {
    for (i32 square_y = 0; square_y < (atlas_size / 16); square_y += 1) {
      bool fail = false;
      for (i32 square_width = 0; square_width < std::ceil(width / 16.0); square_width += 1) {
        for (i32 square_height = 0; square_height < std::ceil(height / 16.0); square_height += 1) {
          size_t square_i = 16 * (square_y + square_height) + (square_x + square_width);
          if (square_x + square_width > (atlas_size / 16) || square_y + square_height > (atlas_size / 16)) {
            fail = true;
            break;
          }
          if (!free_16x16_squares[square_i]) {
            fail = true;
            break;
          }
        }
        if (fail) { break; }
      }

      if (!fail) {
        return std::make_pair(square_x * 16, square_y * 16);
      }
    }
  }
  debug_warn("fail");
  return std::nullopt;
}

void TextureAtlas::mark_space_for_texture(i32 x, i32 y, i32 width, i32 height) {
  i32 begin_x = x / 16;
  i32 begin_y = y / 16;
  i32 end_x = begin_x + std::ceil(width / 16.0);
  i32 end_y = begin_y + std::ceil(height / 16.0);

  for (i32 square_x = begin_x; square_x < end_x; square_x += 1) {
    for (i32 square_y = begin_y; square_y < end_y; square_y += 1) {
      ensure(free_16x16_squares[16 * square_y + square_x]);
      free_16x16_squares[16 * square_y + square_x] = false;
    }
  }
}

glm::vec<2, i32> TextureAtlas::paste_texture(stbi_uc* data, i32 width, i32 height) {
  auto [x, y] = find_space_for_texture(width, height).value_or(std::make_pair(0, 0));
  mark_space_for_texture(x, y, width, height);

  for (i32 oy = 0; oy < height; oy += 1) {
    for (i32 ox = 0; ox < width; ox += 1) {
      if ((x + ox) < atlas_size && (y + oy) < atlas_size) {
        for (i32 c = 0; c < 4; c += 1) {
          image[4 * ((y + oy) * atlas_size + (x + ox)) + c] = data[4 * (oy * width + ox) + c];
        }
      }
    }
  }

  return {x, y};
}
