#pragma once

#include "dny_Definitions.hpp"
#include "dny_TileMap.hpp"
#include "dny_TriggerCategory.hpp"
#include "physics/physics.hpp"

#include <string>
#include <vector>

namespace dny{
	struct Level{
		// Used as the level filename and definition files.
		std::string basename;
		TileMap tilemap;
		std::vector<polyline_collider<float>> collisions;
		std::vector<TileDef> tile_defs;
		std::vector<PlatformDef> platform_defs;
		std::vector<EntityDef> entity_defs;
		std::vector<TriggerDef> trigger_defs;
	};

	// Save/load helpers for TileMap
	struct LevelSerializer{
		static void save( Level const& level );
		static void load( Level& level );

	private:
		static void save_map_file( Level const& level );
		static void save_def_file( Level const& level );
		static void save_pfm_file( Level const& level );
		static void save_trg_file( Level const& level );
		static void save_ety_file( Level const& level );
		static void save_pln_file( Level const& level );

		static void load_map_file( Level& level );
		static void load_def_file( Level& level );
		static void load_pfm_file( Level& level );
		static void load_trg_file( Level& level );
		static void load_ety_file( Level& level );
		static void load_pln_file( Level& level );

		static void load_old_map_file( Level& level );
	};
}
