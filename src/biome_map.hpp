#pragma once

namespace biomes {
struct Biome;
};

namespace biomemap {
const biomes::Biome& get_biome(float humidity, float temperature);
};