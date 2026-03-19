#pragma once

#include "imode.hpp"
#include "level_editor.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"
#include "ui/button.hpp"
#include "ui/listbox.hpp"

namespace dny
{
	class LevelEditor::TextureSelectMode : public basic_mode{
	public:
		TextureSelectMode( LevelEditor& editor, std::size_t active_idx, Rect<std::int32_t> const& dialog_rect );

		void update( Mouse const& mouse, Keyboard& keyboard )override;
		void render( renderer2d& renderer )const override;

	protected:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );

	protected:		
		LevelEditor& m_parent;
		std::size_t m_active_index = {};

		std::string m_selected;
		std::shared_ptr<ui::ListBox> m_list_box;
		std::shared_ptr<ui::Button> m_accept;
		std::shared_ptr<ui::Button> m_cancel;
	};
}
