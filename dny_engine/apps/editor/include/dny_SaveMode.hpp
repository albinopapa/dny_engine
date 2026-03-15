#pragma once

#include "dny_IMode.hpp"
#include "dny_LevelEditor.hpp"

#include "math/math.hpp"
#include "ui/button.hpp"
#include "ui/input_textbox.hpp"

namespace dny
{
	class LevelEditor::SaveMode : public IMode{
	public:
		SaveMode( LevelEditor& parent );
		void render()const override;

	private:
		void handle_mouse( Input const& input_ )override;
		void handle_keyboard( Input const& input_ )override;

	private:
		static constexpr auto center = m_editor_area.center();
		static constexpr auto m_dialog_rect = Rect<float>{
			vector2<float>{ center - vector2<float>{ 200.f, 100.f } },
			vector2<float>{ center + vector2<float>{ 200.f, 100.f } }
		};
		ui::Button m_okay_button;
		ui::Button m_cancel_button;
		ui::InputTextBox m_filename_input_box;
		LevelEditor* m_parent = nullptr;
	};
}
