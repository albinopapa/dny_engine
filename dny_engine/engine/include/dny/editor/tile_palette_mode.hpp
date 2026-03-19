#pragma once

#include "imode.hpp"
#include "level_editor.hpp"
#include "tile_map.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"
#include "ui/panel.hpp"
#include "ui/vscrollbar.hpp"

#include <array>
#include <memory>
#include <optional>

namespace dny
{
	class LevelEditor::TilePaletteMode : public basic_mode{
	public:
		TilePaletteMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect );

		void update( Mouse const& mouse, Keyboard& keyboard )override;
		void render( renderer2d& renderer_ )const override;

	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		std::optional<std::size_t> hit_test( vector2<std::int32_t> const& screen_pos ) const noexcept;
		std::int32_t max_scroll_offset() const noexcept;
		void sync_scrollbar() noexcept;

	private:
		static constexpr std::int32_t m_tile_size = Tile::size;
		static constexpr std::int32_t m_tile_spacing = 10;
		static constexpr std::int32_t m_visible_count = 5;

		std::array<std::int32_t, 8> m_tileset = {
			0,
			1,
			2,
			3,
			4,
			5,
			6,
			7
		};

		LevelEditor& m_parent;
	
		std::shared_ptr<ui::VScrollBar> m_scrollbar;
		std::size_t m_scroll_offset = 0;
		std::size_t m_hovered_index = 0;
		bool m_has_hover = false;
	};
}
