#pragma once

#include "imode.hpp"
#include "level_editor.hpp"
#include "tile_map.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"

#include <array>
#include <optional>

namespace dny
{
	class LevelEditor::TilePaletteMode : public IMode{
	public:
		TilePaletteMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect );

		void update( Mouse const& mouse, Keyboard& keyboard )override;
		void render( renderer2d& renderer_, Font const& font_ )const override;

	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		std::optional<std::size_t> hit_test( vector2<std::int32_t> const& screen_pos ) const noexcept;

	private:
		static constexpr std::int32_t m_tile_size = Tile::size;
		static constexpr std::int32_t m_tile_spacing = 10;

		// Available tile IDs in the palette
		std::array<std::int32_t, 8> m_tileset = {
			0, // Empty
			1, // Dirt
			2, // Platform
			3, // Water
			4, // Lava
			5, // PlayerSpawn
			6, // HopperSpawn
			7  // FireBallSpawn
		};

		ui::Panel m_dialog_panel;
		LevelEditor& m_parent;
		std::size_t m_scroll_offset = 0;
		std::size_t m_hovered_index = 0;
		bool m_has_hover = false;
	};
}
