#pragma once

#include "dny_IMode.hpp"
#include "dny_LevelEditor.hpp"
#include "../../dny_framework/include/dny_UITools.hpp"
#include "../../dny_framework/include/dny_VectorMath.hpp"

#include <string_view>

namespace dny
{
	class LevelEditor::LoadMode : public IMode{
	public:
		LoadMode( LevelEditor& parent );
		void render()const override;
	private:
		void handle_mouse()override;
		void handle_keyboard()override;
		void handle_listbox(Mouse const& event);
	private:
		static constexpr std::string_view m_settings_filename = "editor_settings.txt";
		static constexpr Point center = m_editor_area.center();
		static constexpr RectF m_dialog_rect = RectF{
			Vec2f{ center - Point{ 200, 300 } },
			Vec2f{ center + Point{ 200, 300 } }
		};

		ui::Button m_okay_button;
		ui::Button m_cancel_button;
		ui::InputBox m_filename_input_box;
		ui::ListBox<std::string> m_list_box;
		LevelEditor* m_parent = nullptr;
	};
}
