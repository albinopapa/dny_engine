#pragma once

#include "dny_IMode.hpp"
#include "dny_LevelEditor.hpp"
#include "../../dny_framework/include/dny_UITools.hpp"
#include "../../dny_framework/include/dny_VectorMath.hpp"

namespace dny
{
	class LevelEditor::SaveBeforeExitMode : public IMode{
	public:
		SaveBeforeExitMode( LevelEditor& parent );
		void render()const override;

	private:
		void handle_mouse()override;
		void handle_keyboard()override;

	private:
		static constexpr Point center = m_editor_area.center();
		static constexpr RectF m_dialog_rect = RectF{
			Vec2f{ center - Point{ 200, 125 } },
			Vec2f{ center + Point{ 200, 125 } }
		};

		ui::Button m_save_and_exit_button;
		ui::Button m_exit_without_saving_button;
		ui::Button m_cancel_button;
		LevelEditor* m_parent = nullptr;
	};
}
