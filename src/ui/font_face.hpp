#pragma once
#include <string>
#include <unordered_map>
#include <ft2build.h>
#include <glm/vec2.hpp>
#include "../types.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftimage.h>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

struct FontGlyph {
  glm::vec<2, i32> size{};
  glm::vec<2, i32> bearing{};
  glm::vec<2, i32> advance{};
  glm::vec2 uv_start{};
  glm::vec2 uv_end{};
};

class FontFace {
public:
  FontFace() = default;

  FontFace(FT_Library& freetype_lib, std::string path, i32 pixel_height);

  ~FontFace();

  void bind(u32 slot) const;

  const FontGlyph* find_glyph(u32 charcode) const;

  u32 render_to_texture(std::string text);

  [[maybe_unused]] void save_glyph_texture_to_file(const char* filename) const;

private:
  bool try_creating_glyph_data(i32 texture_dimensions, i32 pixel_height);

private:
  FT_Face freetype_face;
  u32 texture = 0;
  u32 sampler = 0;
  std::unordered_map<u32, FontGlyph> glyph_map;
};
