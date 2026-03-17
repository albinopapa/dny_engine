#pragma once

#include "definitions.hpp"
#include "tile_map.hpp"
#include "trigger_category.hpp"
#include "level_document.hpp"
#include "physics/physics.hpp"

#include <string>
#include <vector>

namespace dny{

	// Save/load helpers for TileMap
	struct LevelSerializer{
		static void save( LevelDocument const& level );
		static void load( LevelDocument& level );

	private:
		static void save_map_file( LevelDocument const& level );  // Tilemap data
		static void save_def_file( LevelDocument const& level );  // Tile definitions (properties, textures, etc.)
		static void save_pfm_file( LevelDocument const& level );  // Platform definitions
		static void save_trg_file( LevelDocument const& level );  // Trigger definitions
		static void save_ety_file( LevelDocument const& level );  // Entity definitions
		static void save_pln_file( LevelDocument const& level );  // Polyline collider data
		static void load_map_file( LevelDocument& level );
		static void load_def_file( LevelDocument& level );
		static void load_pfm_file( LevelDocument& level );
		static void load_trg_file( LevelDocument& level );
		static void load_ety_file( LevelDocument& level );
		static void load_pln_file( LevelDocument& level );
	};
}
