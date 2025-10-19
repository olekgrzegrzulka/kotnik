#pragma once
#include "button.hpp"
#include "slider.hpp"
#include "widget.hpp"

class Sprite;

class UI;
class Sprite;

class SettingsMenu : public Widget {
public:
  SettingsMenu(const UI& ui);

  void update() override;

  void draw() override;

private:
  Sprite& panel;
  Sprite& inner_panel;
  Sprite& tab_panel;

  Button& tab1;
  Button& tab2;
  Button& tab3;

  Label& tab1_label_shadow;
  Label& tab2_label_shadow;
  Label& tab3_label_shadow;
  Label& tab1_label;
  Label& tab2_label;
  Label& tab3_label;

  Slider& slider1;
  Slider& slider2;

  std::array<Button*, 3> tabs = {&tab1, &tab2, &tab3};
  std::array<Label*, 3> tab_labels = {&tab1_label, &tab2_label, &tab3_label};
  std::array<Label*, 3> tab_label_shadows = {&tab1_label_shadow, &tab2_label_shadow, &tab3_label_shadow};

  bool is_dragged = false;
};