#pragma once

#include "editor/imode.hpp"
#include "editor/level_editor.hpp"

#include "ui/button.hpp"
#include "ui/panel.hpp"

#include <vector>

namespace dny{
	class LevelEditor::TileAttributesMode : public basic_mode{
	public:
		TileAttributesMode( LevelEditor& parent, Rect<std::int32_t> const& area_ ) noexcept;
		void update( Mouse const& mouse_, Keyboard& keyboard_ ) override;
		void render( renderer2d& renderer_ ) const override;

	private:
		static constexpr std::int32_t m_visible_height = 200;
		LevelEditor& m_parent;

		std::int32_t m_scroll_offset = 0;
		std::int32_t m_content_height = 0;
		std::vector<ui::Panel> m_attribute_panels;
	};
}