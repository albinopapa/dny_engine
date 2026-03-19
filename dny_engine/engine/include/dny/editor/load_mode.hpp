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
	class LevelEditor::LoadMode : public basic_mode{
	public:
		LoadMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect_ );
		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void render( renderer2d& renderer_ )const override;

	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		void handle_listbox( Mouse const& mouse_ );
		void wrap_focus()noexcept;
	private:		
		std::shared_ptr<ui::Button> m_load;
		std::shared_ptr<ui::Button> m_cancel;
		std::shared_ptr<ui::InputTextBox> m_filename_input_box;
		std::shared_ptr<ui::ListBox> m_list_box;
		std::shared_ptr<ui::VScrollBar> m_list_scroll_bar;

		LevelEditor& m_parent;

		// Focus order: 0 = list box, 
		//				1 = filename input box, 
		//				2 = load button, 
		//				3 = cancel button
		std::int32_t m_focus_index = 0;
	};
}
