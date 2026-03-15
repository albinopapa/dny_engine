#pragma once

#include "dny_IMode.hpp"
#include "dny_LevelEditor.hpp"
#include "../../dny_framework/include/dny_UITools.hpp"
#include "../../dny_framework/include/dny_VectorMath.hpp"

namespace dny
{
	class LevelEditor::ResizeMode : public IMode{
	public:
		ResizeMode( LevelEditor& parent );
		void render()const override;

	private:
		void handle_mouse()override;
		void handle_keyboard()override;
		void accept_and_resize();

	private:
		static constexpr auto center = m_editor_area.center();
		static constexpr auto m_dialog_rect = RectF{
			Vec2f{ center - Point{ 200, 200 } },
			Vec2f{ center + Point{ 200, 200 } }
		};

		enum class Focus{ None, WidthField, HeightField };
		ui::InputBox m_width_input_box;
		ui::InputBox m_height_input_box;
		ui::Button m_okay_button;
		ui::Button m_cancel_button;
		LevelEditor* m_parent = nullptr;
		Focus m_focus = Focus::None;
	};
}
