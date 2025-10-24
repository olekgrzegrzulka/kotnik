#pragma once
#include <glm/mat4x4.hpp>
#include "common.hpp"

void water_overlay_init();
void water_overlay_draw(const glm::mat4&);

void water_overlay_set_color(std::optional<rgba> color_);