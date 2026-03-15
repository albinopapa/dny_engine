#pragma once

#include "core/rectangle.hpp"
#include "core/dims2.hpp"
#include "math/math.hpp"

#include <cstdint>
#include <vector>

namespace dny
{
	class Tile{
	public:
		static constexpr Rect<float> get_rect()noexcept{
			return Rect<float>{
				0.f,
				0.f,
				static_cast< float >( size ),
				static_cast< float >( size )
			};
		}
		static constexpr std::int32_t size = 32;
		std::int32_t id = -1;
		std::int32_t rotation_index = 0; // 0, 1, 2, 3
	};

	inline const std::vector<TileDef> g_tile_defs = {
		{ "Empty", "", TileCategory::Empty, 0.f, 0.f, 0 },
		{ "Dirt", "dirt.png", TileCategory::Solid, 0.8f, 0.f, 1 },
		{ "Platform", "platform.png", TileCategory::Platform, 0.5f, 0.f, 2 },
		{ "Water", "", TileCategory::Liquid, 0.f, 0.f, 3 },
		{ "Lava", "", TileCategory::Liquid, 0.f, 10.f, 4 },
		{ "PlayerSpawn", "player.png", TileCategory::Spawner, 0.f, 0.f, 5 },
		{ "HopperSpawn", "hopper.png", TileCategory::Spawner, 0.f, 0.f, 6 },
		{ "FireBallSpawn", "fireball.png", TileCategory::Spawner, 0.f, 0.f, 7 }
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
