#include "settings_menu.hpp"
#include <algorithm>
#include <array>
#include "../input.hpp"
#include "button.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "sprite.hpp"
#include "widget.hpp"

#include "../world.hpp"          // World::chunk_load_distance
#include "../world_renderer.hpp" // WorldRenderer::fog_start, fog_end

constexpr i32 total_width = 400;
constexpr i32 total_height = 300;
constexpr i32 inner_margin = 3;

constexpr i32 tab_panel_inner_margin = 0;
constexpr i32 tab_width = 80;
constexpr i32 tab_height = 22;
constexpr i32 top_bar_height = 22;

SettingsMenu::SettingsMenu(UI& ui) : Widget::Widget(ui),
                                     panel(add_child<Sprite>()),
                                     top_bar_panel(panel.add_child<Widget>()),
                                     tab_panel(panel.add_child<Sprite>()),
                                     inner_panel(panel.add_child<Sprite>()),
                                     tab1(tab_panel.add_child<Button>()),
                                     tab2(tab_panel.add_child<Button>()),
                                     tab3(tab_panel.add_child<Button>()),
                                     tab1_label_shadow(tab1.add_child<Label>()),
                                     tab2_label_shadow(tab2.add_child<Label>()),
                                     tab3_label_shadow(tab3.add_child<Label>()),
                                     tab1_label(tab1.add_child<Label>()),
                                     tab2_label(tab2.add_child<Label>()),
                                     tab3_label(tab3.add_child<Label>()) {

  panel.set_texture("panel_rectangular");
  panel.set_nine_slice_margin(4);
  panel.set_width(total_width);
  panel.set_height(total_height);
  panel.get_layout().enabled = true;
  panel.get_layout().direction = LayoutDirection::TOP_TO_BOTTOM;
  panel.get_layout().fit_to_contents = true;
  panel.get_layout().margin = 2;
  panel.get_layout().spacing = 0;
  panel.get_layout().expand_children = true;

  top_bar_panel.set_height(22);
  Label& top_bar_label_shadow = top_bar_panel.add_child<Label>("Settings");
  top_bar_label_shadow.set_anchor(Anchor::CENTER_LEFT);
  top_bar_label_shadow.set_screen_anchor(Anchor::CENTER_LEFT);
  top_bar_label_shadow.set_x(1);
  top_bar_label_shadow.set_y(1);
  top_bar_label_shadow.set_text_color({0.1f, 0.1f, 0.1f});
  Label& top_bar_label = top_bar_panel.add_child<Label>("Settings");
  top_bar_label.set_anchor(Anchor::CENTER_LEFT);
  top_bar_label.set_screen_anchor(Anchor::CENTER_LEFT);

  tab_panel.set_texture("panel_rounded");
  tab_panel.get_layout().enabled = true;
  tab_panel.get_layout().margin = 0;
  tab_panel.get_layout().spacing = 0;
  tab_panel.get_layout().direction = LayoutDirection::LEFT_TO_RIGHT;
  tab_panel.set_nine_slice_margin(4);
  tab_panel.set_height(22);

  inner_panel.set_texture("panel_rounded");
  inner_panel.set_nine_slice_margin(4);
  inner_panel.set_height(200);
  inner_panel.get_layout().enabled = true;
  inner_panel.get_layout().direction = LayoutDirection::TOP_TO_BOTTOM;
  inner_panel.get_layout().margin = 6;
  inner_panel.get_layout().spacing = 0;
  inner_panel.get_layout().expand_children = true;

  tab1_label.set_text("General");
  tab1_label_shadow.set_text("General");
  tab2_label.set_text("Controls");
  tab2_label_shadow.set_text("Controls");
  tab3_label.set_text("Debug");
  tab3_label_shadow.set_text("Debug");

  {
    auto& x_button = top_bar_panel.add_child<Button>("X");
    x_button.set_anchor(Anchor::CENTER_RIGHT);
    x_button.set_screen_anchor(Anchor::CENTER_RIGHT);
    x_button.set_x(-2);
    x_button.set_width(top_bar_panel.get_height() - 2);
    x_button.set_height(top_bar_panel.get_height() - 2);
    x_button.on_press([&]() {
      set_process(false);
    });
  }

  {
    auto& container_simulation_distance = inner_panel.add_child<Widget>();
    container_simulation_distance.set_height(20);
    container_simulation_distance.get_layout().enabled = true;
    container_simulation_distance.get_layout().direction = LayoutDirection::LEFT_TO_RIGHT;
    container_simulation_distance.get_layout().margin = 0;
    container_simulation_distance.get_layout().spacing = 0;
    container_simulation_distance.get_layout().expand_children = true;
    container_simulation_distance.get_layout().fill = true;
    auto& label = container_simulation_distance.add_child<Label>("Simulation distance");
    label.set_label_anchor(Anchor::CENTER_LEFT);
    label.set_weight(0.8f);
    auto& slider = container_simulation_distance.add_child<Slider>();
    slider.set_weight(1.0f);
    slider.set_min_value(2);
    slider.set_max_value(8);
    slider.set_value(World::chunk_load_distance);

    slider.on_value_changed([&](i32 value) {
      World::chunk_load_distance = value;
    });
  }

  {
    auto& container_fog_density = inner_panel.add_child<Widget>();
    container_fog_density.set_height(20);
    container_fog_density.get_layout().enabled = true;
    container_fog_density.get_layout().direction = LayoutDirection::LEFT_TO_RIGHT;
    container_fog_density.get_layout().margin = 0;
    container_fog_density.get_layout().spacing = 0;
    container_fog_density.get_layout().expand_children = true;
    container_fog_density.get_layout().fill = true;
    auto& label = container_fog_density.add_child<Label>("Fog density");
    label.set_label_anchor(Anchor::CENTER_LEFT);
    label.set_weight(0.8f);
    auto& slider = container_fog_density.add_child<Slider>();
    slider.set_weight(1.0f);
    slider.set_min_value(8.0f);
    slider.set_max_value(256.0f);
    slider.set_value(WorldRenderer::fog_start);

    slider.on_value_changed([&](i32 value) {
      WorldRenderer::fog_start = value;
    });
  }

  for (auto* label : {&tab1_label, &tab2_label, &tab3_label}) {
    label->set_anchor(Anchor::CENTER);
    label->set_screen_anchor(Anchor::CENTER);
  }

  for (auto* label : {&tab1_label_shadow, &tab2_label_shadow, &tab3_label_shadow}) {
    label->set_anchor(Anchor::CENTER);
    label->set_screen_anchor(Anchor::CENTER);
    label->set_text_color({0.1f, 0.1f, 0.1f});
    label->set_pos(1, 1);
  }

  tab1.on_press([&] {
    tab2.depress();
    tab3.depress();
  });

  tab2.on_press([&] {
    tab1.depress();
    tab3.depress();
  });

  tab3.on_press([&] {
    tab1.depress();
    tab2.depress();
  });

  for (size_t i = 0; i < 3; i += 1) {
    tabs[i]->set_texture_idle("tab_idle");
    tabs[i]->set_texture_hovered("tab_hovered");
    tabs[i]->set_texture_pressed("tab_pressed");
    tabs[i]->set_texture_disabled("tab_disabled");
    tabs[i]->set_texture("tab_idle");

    tabs[i]->set_width(tab_width);
    tabs[i]->set_height(tab_height - tab_panel_inner_margin);
    tabs[i]->set_nine_slice_margin(6);
    tabs[i]->set_switch_mode(true);
  }
}

void SettingsMenu::update() {
  bool mouse_on_top_bar_x = Input::get_mouse_x() >= top_bar_panel.get_position(Anchor::TOP_LEFT).x &&
                            Input::get_mouse_x() < top_bar_panel.get_position(Anchor::BOTTOM_RIGHT).x;
  bool mouse_on_top_bar_y = Input::get_mouse_y() >= top_bar_panel.get_position(Anchor::TOP_LEFT).y &&
                            Input::get_mouse_y() < top_bar_panel.get_position(Anchor::BOTTOM_RIGHT).y;
  bool mouse_on_top_bar = mouse_on_top_bar_x && mouse_on_top_bar_y;
  bool lmb_pressed = Input::mouse_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_pressed = Input::mouse_just_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_released = Input::mouse_just_released(Input::Mouse::MOUSE_BUTTON_LEFT);

  static i32 x_rel = 0;
  static i32 y_rel = 0;

  if (lmb_just_pressed && mouse_on_top_bar) {
    is_dragged = true;
    x_rel = Input::get_mouse_x() - panel.get_position(Anchor::TOP_LEFT).x;
    y_rel = Input::get_mouse_y() - panel.get_position(Anchor::TOP_LEFT).y;
  }
  if (lmb_just_released) {
    is_dragged = false;
  }

  if (is_dragged) {
    panel.set_x(Input::get_mouse_x() - x_rel);
    panel.set_y(Input::get_mouse_y() - y_rel);
  }

  // Clamp panel to window size
  panel.set_x(std::clamp(panel.get_x(), 0, Input::get_window_size().x - total_width));
  panel.set_y(std::clamp(panel.get_y(), 0, Input::get_window_size().y - total_height));
}

void SettingsMenu::draw() {
}