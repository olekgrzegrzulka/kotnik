#pragma once
#include "common.hpp"

void skybox_init();
void skybox_draw(const glm::mat4&);

void skybox_set_color(std::optional<rgb> top,
                      std::optional<rgb> up,
                      std::optional<rgb> mid,
                      std::optional<rgb> down,
                      std::optional<rgb> bottom);