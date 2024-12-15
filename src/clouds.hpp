#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include "common.hpp"

void clouds_init();
void clouds_draw(const glm::mat4&);
void clouds_set_color(rgb);
