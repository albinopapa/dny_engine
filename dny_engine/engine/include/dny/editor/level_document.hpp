#pragma once

#include "tile_map.hpp"
#include "definitions.hpp"

#include "physics/physics.hpp"

namespace dny
{
	struct LevelDocument{
		// Used as the level filename and definition files.
		std::string basename;
		TileMap tilemap;
		std::vector<polyline_collider<float>> collisions;
		std::vector<TileDef> tile_defs;
		std::vector<PlatformDef> platform_defs;
		std::vector<EntityDef> entity_defs;
		std::vector<TriggerDef> trigger_defs;
	};
}