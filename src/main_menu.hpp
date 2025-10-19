#pragma once
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include "clouds.hpp"
#include "common.hpp"
#include "shader.hpp"
#include "skybox.hpp"
#include "texture.hpp"
#include "ui/button.hpp"
#include "ui/ui.hpp"
#include "world_main_menu.hpp"
#include "world_renderer.hpp"

class MainMenu {
public:
  MainMenu(Shader& cube_shader, Texture& atlas_texture, UI& ui_)
      : world_renderer(world, cube_shader, atlas_texture),
        ui(ui_),
        button_worlds(ui.add_widget<Button>("Play")),
        button_settings(ui.add_widget<Button>("Settings")),
        button_quit(ui.add_widget<Button>("Quit")) {
    world.worldgen_enabled = false;
    world.add_entity<Player>({0, 20, 0});

    button_worlds.set_width(180);
    button_worlds.set_height(60);
    button_worlds.set_anchor(Anchor::TOP_CENTER);
    button_worlds.set_screen_anchor(Anchor::CENTER_CENTER);
    button_worlds.set_y(0);

    button_settings.set_width(180);
    button_settings.set_height(60);
    button_settings.set_anchor(Anchor::TOP_CENTER);
    button_settings.set_screen_anchor(Anchor::CENTER_CENTER);
    button_settings.set_y(80);

    button_quit.set_width(180);
    button_quit.set_height(60);
    button_quit.set_anchor(Anchor::TOP_CENTER);
    button_quit.set_screen_anchor(Anchor::CENTER_CENTER);
    button_quit.set_y(160);
  }

  void update() {
  }

  void draw(i32 window_width, i32 window_height) {
    // Drawing skybox and clouds
    glm::vec3 up = {0.0f, 1.0f, 0.0f};

    glViewport(0, 0, window_width, window_height);
    float aspect_ratio = window_width / (float)(window_height);

    glm::mat4 camera_matrix = glm::mat4(1.0);
    camera_matrix *= glm::perspective(glm::radians<float>(70.0), aspect_ratio, 0.01f, 1000.0f);
    camera_matrix *= glm::lookAt(
        {0.0, 0.7, 0.0},
        {0.0, 1.0, 3.0},
        up);

    glDisable(GL_DEPTH_TEST);
    skybox_draw(camera_matrix);
    clouds_draw(camera_matrix);

    // Drawing logo
    auto logo_size = world.get_logo_size();
    glm::vec3 camera_pos{logo_size.x * 0.5f, logo_size.y * 0.5f, 30.0f};
    glm::vec3 camera_look{0.0f, 0.0f, -1.0f};
    glViewport(0, window_height * 0.5,
               window_width, window_height * 0.5);
    aspect_ratio = window_width / (float)(window_height * 0.5f);

    camera_matrix = glm::mat4(1.0);
    camera_matrix *= glm::perspective(glm::radians<float>(60.0), aspect_ratio, 0.01f, 1000.0f);

    camera_matrix *= glm::lookAt(
        {0.0, 0.0, 0.0},
        camera_look,
        up);

    glEnable(GL_DEPTH_TEST);
    world_renderer.update(camera_pos, camera_matrix);

    glViewport(0, 0, window_width, window_height);
  }

public:
  WorldMainMenu world;
  WorldRenderer world_renderer;
  UI& ui;
  Button& button_worlds;
  Button& button_settings;
  Button& button_quit;
};