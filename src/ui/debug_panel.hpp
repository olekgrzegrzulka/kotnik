
#pragma once
#include <sstream>
#include "label.hpp"
#include "sprite.hpp"
#include "ui.hpp"
#include "widget.hpp"

class DebugPanel : public Widget {
public:
  DebugPanel(UI& ui, std::stringstream& seed) : Widget::Widget(ui),
                                                label_seed_shadow{add_child<Label>("")},
                                                label_seed{add_child<Label>("")},
                                                label_player_pos_shadow{add_child<Label>("")},
                                                label_player_pos{add_child<Label>("")},
                                                label_player_chunk_pos_shadow{add_child<Label>("")},
                                                label_player_chunk_pos{add_child<Label>("")},
                                                label_biome_name_shadow{add_child<Label>("")},
                                                label_biome_name{add_child<Label>("")} {
    label_seed_shadow.set_x(6 + 1);
    label_seed_shadow.set_y(6 + 1 + 28 * 0);
    label_seed_shadow.set_text_color({0.1f, 0.1f, 0.1f});
    label_seed_shadow.set_text(seed.str());

    label_seed.set_x(6);
    label_seed.set_y(6 + 28 * 0);
    label_seed.set_text(seed.str());

    label_player_pos_shadow.set_x(6 + 1);
    label_player_pos_shadow.set_y(6 + 1 + 28 * 1);
    label_player_pos_shadow.set_text_color({0.1f, 0.1f, 0.1f});

    label_player_pos.set_x(6);
    label_player_pos.set_y(6 + 28 * 1);

    label_player_chunk_pos_shadow.set_x(6 + 1);
    label_player_chunk_pos_shadow.set_y(6 + 1 + 28 * 2);
    label_player_chunk_pos_shadow.set_text_color({0.1f, 0.1f, 0.1f});

    label_player_chunk_pos.set_x(6);
    label_player_chunk_pos.set_y(6 + 28 * 2);

    label_biome_name_shadow.set_x(6 + 1);
    label_biome_name_shadow.set_y(6 + 1 + 28 * 3);
    label_biome_name_shadow.set_text_color({0.1f, 0.1f, 0.1f});

    label_biome_name.set_x(6);
    label_biome_name.set_y(6 + 28 * 3);
  }

  void update() override {
  }

  void draw() override {
  }

public:
  Label& label_seed_shadow;
  Label& label_seed;
  Label& label_player_pos_shadow;
  Label& label_player_pos;
  Label& label_player_chunk_pos_shadow;
  Label& label_player_chunk_pos;
  Label& label_biome_name_shadow;
  Label& label_biome_name;
};