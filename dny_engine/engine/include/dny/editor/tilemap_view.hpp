#pragma once

#include "tile_map.hpp"

#include "utilities/rectangle.hpp"
#include "renderer/renderer2d.hpp"

#include <unordered_map>

namespace dny{
	class LevelEditor;
	struct EditorCamera;

	// Simple view over the editable tilemap region
	class TileMapView{
	public:
		TileMapView( TileMap const& tilemap_, Rect<std::int32_t> const& area_ ) noexcept;

		void render( renderer2d& renderer_, EditorCamera const& cam_, std::unordered_map<std::string, surface<ColorF>> const& textures_ ) const;
		std::optional<vector2<std::int32_t>> screen_to_tile_index( vector2<std::int32_t> const& screen_pos, EditorCamera const& cam ) const noexcept;

	private:
		TileMap const& m_tilemap;
		Rect<std::int32_t> m_area{};
		// Layout constants
		static constexpr std::int32_t m_tile_size = Tile::size;
	};

}