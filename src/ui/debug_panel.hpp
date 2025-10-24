
#pragma once
#include <sstream>
#include "label.hpp"
#include "sprite.hpp"
#include "ui.hpp"
#include "widget.hpp"

class DebugPanel : public Widget {
public:
  DebugPanel(UI& ui, std::stringstream& seed) : Widget::Widget(ui),
                                                label_frametime{add_child<Label>()},
                                                label_frametime_shadow{label_frametime.add_child<Label>()},
                                                label_seed{add_child<Label>()},
                                                label_seed_shadow{label_seed.add_child<Label>()},
                                                label_player_pos{add_child<Label>()},
                                                label_player_pos_shadow{label_player_pos.add_child<Label>()},
                                                label_player_chunk_pos{add_child<Label>()},
                                                label_player_chunk_pos_shadow{label_player_chunk_pos.add_child<Label>()},
                                                label_biome_name{add_child<Label>()},
                                                label_biome_name_shadow{label_biome_name.add_child<Label>()},
                                                label_lightmap{add_child<Label>()},
                                                label_lightmap_shadow{label_lightmap.add_child<Label>()},
                                                label_heightmap{add_child<Label>()},
                                                label_heightmap_shadow{label_heightmap.add_child<Label>()} {

    get_layout().enabled = true;
    get_layout().direction = LayoutDirection::TOP_TO_BOTTOM;
    get_layout().positioning = LayoutPositioning::LEFT_OR_TOP;
    // get_layout().margin = 16;
    get_layout().spacing = -64 + 20;
    set_x(4);
    set_y(4);

    for (auto* label : {&label_frametime, &label_seed, &label_player_pos, &label_player_chunk_pos, &label_biome_name, &label_lightmap, &label_heightmap}) {
      label->set_process_children_first(true);
      label->set_label_anchor(Anchor::TOP_LEFT);
    }

    label_seed_shadow.set_text(seed.str());
    label_seed.set_text(seed.str());

    for (auto* shadow : {&label_frametime_shadow, &label_seed_shadow, &label_player_pos_shadow, &label_player_chunk_pos_shadow, &label_biome_name_shadow, &label_lightmap_shadow, &label_heightmap_shadow}) {
      shadow->set_x(1);
      shadow->set_y(1);
      shadow->set_text_color({0.1f, 0.1f, 0.1f});
      shadow->set_label_anchor(Anchor::TOP_LEFT);
    }
  }

  void update() override {
    Widget::update();
  }

  void draw() override {
  }

public:
  Label& label_frametime;
  Label& label_frametime_shadow;
  Label& label_seed;
  Label& label_seed_shadow;
  Label& label_player_pos;
  Label& label_player_pos_shadow;
  Label& label_player_chunk_pos;
  Label& label_player_chunk_pos_shadow;
  Label& label_biome_name;
  Label& label_biome_name_shadow;
  Label& label_lightmap;
  Label& label_lightmap_shadow;
  Label& label_heightmap;
  Label& label_heightmap_shadow;
};