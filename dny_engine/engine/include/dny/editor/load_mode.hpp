#pragma once

#include "imode.hpp"
#include "level_editor.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"
#include "ui/input_textbox.hpp"
#include "ui/button.hpp"
#include "ui/listbox.hpp"

#include <string_view>

namespace dny
{
	class LevelEditor::LoadMode : public IMode{
	public:
		LoadMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect_ );
		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void render( renderer2d& renderer_, Font const& font_ )const override;
	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		void handle_listbox( Mouse const& mouse_ );
	private:		
		std::shared_ptr<ui::Button> m_load;
		std::shared_ptr<ui::Button> m_cancel;
		std::shared_ptr<ui::InputTextBox> m_filename_input_box;
		std::shared_ptr<ui::ListBox> m_list_box;
		ui::Panel m_dialog_panel;
		LevelEditor& m_parent;
	};
}
