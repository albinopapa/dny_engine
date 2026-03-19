#pragma once

#include "tile_category.hpp"
#include "definitions.hpp"

#include "utilities/rectangle.hpp"
#include "utilities/dims2.hpp"
#include "math/math.hpp"

#include <cstdint>
#include <vector>

namespace dny{
	struct Tile{
		bool operator==( Tile const& other )const noexcept{
			return definition_id == other.definition_id &&
				orientation_index == other.orientation_index &&
				platform_id == other.platform_id &&
				entity_id == other.entity_id &&
				trigger_id == other.trigger_id;
		}
		bool operator!=( Tile const& other )const noexcept{
			return !( *this == other );
		}

		std::int32_t definition_id = 0;
		std::int32_t orientation_index = 0;

		std::int32_t platform_id = 0;
		std::int32_t entity_id = 0;
		std::int32_t trigger_id = 0;
		static constexpr auto size = 32;
	};

	// ------------------------------------------------------------------//
	// TILE DEFINITIONS REGISTRY (TEMPORARY STATIC VERSION)              //
	//                                                                   //
	// IMPORTANT INVARIANT:                                              //
	//  - Tile::definition_id is the index into this array.              //
	//  - TileMap stores Tile objects, not pointers.                     //
	//  - Serialized tilemaps depend on these indices being stable.      //
	//  - DO NOT reorder existing entries.                               //
	//  - DO NOT remove entries.                                         //
	//  - New entries must only be appended.                             //
	//                                                                   //
	// This will later be replaced by a runtime TileDefRegistry, but the //
	// index-based ID contract must remain the same.                     //
	// ------------------------------------------------------------------//

	// Name, texture, category, friction, damage
	constexpr std::array<TileDef, 8> g_tile_defs = {
		TileDef{ "Empty", "", TileCategory::Empty, 0.f, 0.f },
		TileDef{ "Dirt", "dirt.png", TileCategory::Solid, 0.8f, 0.f },
		TileDef{ "Platform", "platform.png", TileCategory::Platform, 0.5f, 0.f },
		TileDef{ "Water", "", TileCategory::Liquid, 0.f, 0.f },
		TileDef{ "Lava", "", TileCategory::Liquid, 0.f, 10.f },
		TileDef{ "PlayerSpawn", "player.png", TileCategory::Spawner, 0.f, 0.f },
		TileDef{ "HopperSpawn", "hopper.png", TileCategory::Spawner, 0.f, 0.f },
		TileDef{ "FireBallSpawn", "fireball.png", TileCategory::Spawner, 0.f, 0.f }
	};


	class TileMap{
	public:
		dims2<std::int32_t> size() const noexcept;
		void resize( dims2<std::int32_t> const& new_size );

		Tile& operator[]( vector2<std::int32_t> const& idx )&;
		Tile const& operator[]( vector2<std::int32_t> const& idx )const&;

		Tile& get_tile( vector2<std::int32_t> const& pt ) noexcept;
		Tile const& get_tile( vector2<std::int32_t> const& pt )const noexcept;
		Rect<std::int32_t> tile_rect( vector2<std::int32_t> const& tile_index ) const noexcept;

	private:
		Tile& access_tile( std::int32_t x, std::int32_t y ) noexcept;
		Tile const& access_tile( std::int32_t x, std::int32_t y )const noexcept;

	private:
		dims2<std::int32_t> m_size;
		std::vector<Tile> m_tiles;
	};
}
