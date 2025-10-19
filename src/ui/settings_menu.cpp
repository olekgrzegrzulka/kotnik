#include "settings_menu.hpp"
#include <algorithm>
#include <array>
#include "../input.hpp"
#include "button.hpp"
#include "slider.hpp"
#include "sprite.hpp"
#include "widget.hpp"

constexpr i32 total_width = 400;
constexpr i32 total_height = 300;
constexpr i32 inner_margin = 3;

constexpr i32 tab_panel_inner_margin = 0;
constexpr i32 tab_width = 80;
constexpr i32 tab_height = 22;
constexpr i32 top_bar_height = 22;

SettingsMenu::SettingsMenu(const UI& ui) : Widget::Widget(ui),
                                           panel(add_child<Sprite>()),
                                           inner_panel(add_child<Sprite>()),
                                           tab_panel(add_child<Sprite>()),
                                           tab1(add_child<Button>()),
                                           tab2(add_child<Button>()),
                                           tab3(add_child<Button>()),
                                           tab1_label_shadow(add_child<Label>()),
                                           tab2_label_shadow(add_child<Label>()),
                                           tab3_label_shadow(add_child<Label>()),
                                           tab1_label(add_child<Label>()),
                                           tab2_label(add_child<Label>()),
                                           tab3_label(add_child<Label>()),
                                           slider1(add_child<Slider>()),
                                           slider2(add_child<Slider>()) {

  panel.set_uv_start({1.0 / 16.0, 0.0 / 16.0});
  panel.set_uv_end({2.0 / 16.0, 1.0 / 16.0});
  panel.set_nine_slice_margin(2);
  panel.set_width(total_width);
  panel.set_height(total_height);

  tab_panel.set_uv_start({1.0 / 16.0, 0.0 / 16.0});
  tab_panel.set_uv_end({2.0 / 16.0, 1.0 / 16.0});
  tab_panel.set_nine_slice_margin(2);
  tab_panel.set_width(total_width - 2 * inner_margin);
  tab_panel.set_height(tab_height);

  inner_panel.set_uv_start({1.0 / 16.0, 0.0 / 16.0});
  inner_panel.set_uv_end({2.0 / 16.0, 1.0 / 16.0});
  inner_panel.set_nine_slice_margin(2);
  inner_panel.set_width(total_width - 2 * inner_margin);
  inner_panel.set_height(total_height - 2 * inner_margin - tab_height - top_bar_height);

  tab1_label.set_text("General");
  tab1_label_shadow.set_text("General");
  tab2_label.set_text("Controls");
  tab2_label_shadow.set_text("Controls");
  tab3_label.set_text("Debug");
  tab3_label_shadow.set_text("Debug");

  tab1.on_depress({
      // tab1.p
  });

  for (size_t i = 0; i < 3; i += 1) {
    tabs[i]->set_uv_start_idle({3.0 / 16.0, 0.0 / 16.0});
    tabs[i]->set_uv_end_idle({4.0 / 16.0, 1.0 / 16.0});
    tabs[i]->set_uv_start_hovered({3.0 / 16.0, 1.0 / 16.0});
    tabs[i]->set_uv_end_hovered({4.0 / 16.0, 2.0 / 16.0});
    tabs[i]->set_uv_start_pressed({3.0 / 16.0, 2.0 / 16.0});
    tabs[i]->set_uv_end_pressed({4.0 / 16.0, 3.0 / 16.0});
    tabs[i]->set_uv_start_disabled({3.0 / 16.0, 3.0 / 16.0});
    tabs[i]->set_uv_end_disabled({4.0 / 16.0, 4.0 / 16.0});

    tabs[i]->set_uv_start({3.0 / 16.0, 0.0 / 16.0});
    tabs[i]->set_uv_end({4.0 / 16.0, 1.0 / 16.0});

    tabs[i]->set_width(tab_width);
    tabs[i]->set_height(tab_height - tab_panel_inner_margin);
    tabs[i]->set_nine_slice_margin(4);
    tabs[i]->set_switch_mode(true);

    tab_labels[i]->set_anchor(Anchor::CENTER_CENTER);
    tab_label_shadows[i]->set_anchor(Anchor::CENTER_CENTER);
    tab_label_shadows[i]->set_text_color({0.1f, 0.1f, 0.1f});
  }

  slider1.set_width(total_width * 0.8);
  slider2.set_width(total_width * 0.8);
  slider1.set_anchor(Anchor::CENTER_CENTER);
  slider2.set_anchor(Anchor::CENTER_CENTER);
}

void SettingsMenu::update() {
  bool mouse_on_top_bar_x = Input::get_mouse_x() >= panel.get_position(Anchor::TOP_LEFT).x &&
                            Input::get_mouse_x() < panel.get_position(Anchor::BOTTOM_RIGHT).x;
  bool mouse_on_top_bar_y = Input::get_mouse_y() >= panel.get_position(Anchor::TOP_LEFT).y &&
                            Input::get_mouse_y() < panel.get_position(Anchor::TOP_LEFT).y + top_bar_height;
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

  tab_panel.set_x(panel.get_x() + inner_margin);
  tab_panel.set_y(panel.get_y() + top_bar_height);

  inner_panel.set_x(panel.get_x() + inner_margin);
  inner_panel.set_y(tab_panel.get_y() + tab_height / 2);

  inner_panel.set_height(
      panel.get_position(Anchor::BOTTOM_CENTER).y -
      inner_panel.get_position(Anchor::TOP_CENTER).y - inner_margin);

  for (size_t i = 0; i < 3; i += 1) {
    tabs[i]->set_x(tab_panel.get_x() + tab_panel_inner_margin + i * tab_width);
    tabs[i]->set_y(tab_panel.get_y() + tab_panel_inner_margin);

    tab_labels[i]->set_x(tabs[i]->get_position(Anchor::CENTER_CENTER).x);
    tab_labels[i]->set_y(tabs[i]->get_position(Anchor::CENTER_CENTER).y);
    tab_label_shadows[i]->set_x(tabs[i]->get_position(Anchor::CENTER_CENTER).x + 1);
    tab_label_shadows[i]->set_y(tabs[i]->get_position(Anchor::CENTER_CENTER).y + 1);
  }

  slider1.set_x(inner_panel.get_position(Anchor::CENTER_CENTER).x);
  slider1.set_y(inner_panel.get_position(Anchor::TOP_CENTER).y + 40);
  slider2.set_x(inner_panel.get_position(Anchor::CENTER_CENTER).x);
  slider2.set_y(inner_panel.get_position(Anchor::TOP_CENTER).y + 70);
}

void SettingsMenu::draw() {
}