
#pragma once
#include <sstream>
#include "../biome.hpp"
#include "../player.hpp"
#include "../world.hpp"
#include "../world_gen.hpp"
#include "label.hpp"
#include "sprite.hpp"
#include "ui.hpp"
#include "widget.hpp"

#ifdef linux
#include <fstream>
#include <iostream>
#include <unistd.h>

inline std::string get_memory_usage() {
  i64 page_size = sysconf(_SC_PAGESIZE);
  std::ifstream f("/proc/self/statm");
  i64 size_pages, resident_pages;
  if (!(f >> size_pages >> resident_pages)) return "?";
  i64 resident_mb = (resident_pages * page_size) / 1024 / 1024;

  std::stringstream ss_memory_text;
  ss_memory_text << std::fixed << std::setprecision(2);
  ss_memory_text << resident_mb << "  MB";
  std::string memory_text = ss_memory_text.str();
  return memory_text;
}
#else
std::string get_memory_usage() { return "?"; }
#endif

class DebugPanel : public Widget {
public:
  DebugPanel(UI& ui, std::stringstream& seed, World& world_) : Widget::Widget(ui), world{world_},
                                                               label_frametime{add_child<Label>()},
                                                               label_frametime_shadow{label_frametime.add_child<Label>()},
                                                               label_memory{add_child<Label>()},
                                                               label_memory_shadow{label_memory.add_child<Label>()},
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

    for (auto* label : {&label_frametime, &label_memory, &label_seed, &label_player_pos, &label_player_chunk_pos, &label_biome_name, &label_lightmap, &label_heightmap}) {
      label->set_process_children_first(true);
      label->set_label_anchor(Anchor::TOP_LEFT);
    }

    label_seed_shadow.set_text(seed.str());
    label_seed.set_text(seed.str());

    for (auto* shadow : {&label_frametime_shadow, &label_memory_shadow, &label_seed_shadow, &label_player_pos_shadow, &label_player_chunk_pos_shadow, &label_biome_name_shadow, &label_lightmap_shadow, &label_heightmap_shadow}) {
      shadow->set_x(1);
      shadow->set_y(1);
      shadow->set_text_color({0.1f, 0.1f, 0.1f});
      shadow->set_label_anchor(Anchor::TOP_LEFT);
    }
  }

  void update() override {
    Widget::update();

    auto* player = world.get_player();

    auto [player_chunk_pos, player_local_pos] = cube_to_local(player->world_pos);
    Chunk* player_chunk = world.get_chunk(player_chunk_pos);

    std::stringstream ss_frametime_text;
    ss_frametime_text << std::fixed << std::setprecision(2);
    ss_frametime_text << "Frametime: " << delta_ms << " ms";
    std::string frametime_text = ss_frametime_text.str();
    label_frametime.set_text(frametime_text);
    label_frametime_shadow.set_text(frametime_text);

    static i32 query_memory_clock = 0;
    if (query_memory_clock <= 0) {
      std::string mem = "Memory: " + get_memory_usage();
      label_memory.set_text(mem);
      label_memory_shadow.set_text(mem);

      query_memory_clock = 40;
    } else {
      query_memory_clock -= 1;
    }

    std::stringstream ss_player_pos_text;
    ss_player_pos_text << std::fixed << std::setprecision(2);
    ss_player_pos_text << "World: [" << player->world_pos.x << ", " << player->world_pos.y << ", " << player->world_pos.z << "]";
    std::string player_pos_text = ss_player_pos_text.str();

    label_player_pos.set_text(player_pos_text);
    label_player_pos_shadow.set_text(player_pos_text);

    std::stringstream ss_player_chunk_pos_text;
    ss_player_chunk_pos_text << std::fixed << std::setprecision(2);
    ss_player_chunk_pos_text << "Chunk: [" << player_chunk_pos.x << ", " << player_chunk_pos.y << ", " << player_chunk_pos.z << "]";
    std::string player_chunk_pos_text = ss_player_chunk_pos_text.str();

    label_player_chunk_pos.set_text(player_chunk_pos_text);
    label_player_chunk_pos_shadow.set_text(player_chunk_pos_text);
    static i32 get_biome_counter = 0;
    static auto get_biome_last_player_pos = player->world_pos;
    if (get_biome_counter-- <= 0) {
      if ((get_biome_last_player_pos - player->world_pos).length() > 0.5) {
        get_biome_last_player_pos = player->world_pos;
        auto biome_name = "Biome: " + world.get_world_gen().get_blended_biome(player->world_pos).name;
        label_biome_name.set_text(biome_name);
        label_biome_name_shadow.set_text(biome_name);
      }
      get_biome_counter = 10;
    }

    if (player_chunk) {
      std::stringstream ss_lightmap_text;
      ss_lightmap_text << "Lightmap: " << (i32)player_chunk->get_lightmap(player_local_pos);
      label_lightmap.set_text(ss_lightmap_text.str());
      label_lightmap_shadow.set_text(ss_lightmap_text.str());
    } else {
      label_lightmap.set_text("Lightmap: ?");
      label_lightmap_shadow.set_text("Lightmap: ?");
    }

    auto heightmap = player_chunk ? player_chunk->get_heightmap(player_local_pos.x, player_local_pos.z) : std::nullopt;
    if (heightmap.has_value()) {
      std::stringstream ss_heightmap_text;
      ss_heightmap_text << "Heightmap: " << (i32)heightmap.value() + player_chunk_pos.y * Chunk::chunk_size;
      label_heightmap.set_text(ss_heightmap_text.str());
      label_heightmap_shadow.set_text(ss_heightmap_text.str());
    } else {
      label_heightmap.set_text("Heightmap: ?");
      label_heightmap_shadow.set_text("Heightmap: ?");
    }
  }

  void draw() override {
  }

public:
  World& world;
  float delta_ms{};
  Label& label_frametime;
  Label& label_frametime_shadow;
  Label& label_memory;
  Label& label_memory_shadow;
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