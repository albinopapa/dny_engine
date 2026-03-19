#pragma once

#include "imode.hpp"
#include "level_editor.hpp"

#include "utilities/rectangle.hpp"
#include "math/math.hpp"

namespace dny
{
	class LevelEditor::ResizeMode : public basic_mode{
	public:
		ResizeMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect );

		void update( Mouse const& mouse, Keyboard& keyboard )override;
		void render( renderer2d& renderer_ )const override;
	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		void accept_and_resize();

	private:
		enum class Focus{ None, WidthField, HeightField };
		std::shared_ptr<ui::InputTextBox> m_width_input_box;
		std::shared_ptr<ui::InputTextBox> m_height_input_box;
		std::shared_ptr<ui::Button> m_okay;
		std::shared_ptr<ui::Button> m_cancel;
		LevelEditor& m_parent;
		Focus m_focus = Focus::None;
	};
}
