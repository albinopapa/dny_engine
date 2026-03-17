#pragma once

#include "imode.hpp"
#include "level_editor.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"
#include "ui/button.hpp"
#include "ui/input_textbox.hpp"
#include "ui/panel.hpp"

namespace dny
{
	class LevelEditor::SaveMode : public IMode{
	public:
		SaveMode( LevelEditor& parent_, Rect<std::int32_t> const& dialog_rect_ );

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void render( renderer2d& renderer_, Font const& font_ )const override;

	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );

	private:
		// TODO: this should be a panel with the buttons and input box as children
		// Left in for reminder of UI refactor
		
		ui::Panel m_dialog_panel;
		std::shared_ptr<ui::Button> m_save;
		std::shared_ptr<ui::Button> m_cancel;
		std::shared_ptr<ui::InputTextBox> m_filename_input_box;
		LevelEditor& m_parent;
	};
}
