#pragma once

#include "imode.hpp"
#include "level_editor.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"
#include "ui/button.hpp"

namespace dny
{
	class LevelEditor::SaveBeforeExitMode : public IMode{
	public:
		SaveBeforeExitMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect );

		void update( Mouse const& mouse, Keyboard& keyboard )override;
		void render( renderer2d& renderer_, Font const& font_ )const override;

	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );

	private:
		std::shared_ptr<ui::Button> m_save;
		std::shared_ptr<ui::Button> m_exit;
		std::shared_ptr<ui::Button> m_cancel;
		ui::Panel m_dialog_panel;
		LevelEditor& m_parent;
	};
}
