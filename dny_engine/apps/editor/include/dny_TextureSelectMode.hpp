#pragma once

#include "dny_IMode.hpp"
#include "dny_LevelEditor.hpp"
#include "../../dny_framework/include/dny_Input.hpp"
#include "../../dny_framework/include/dny_UITools.hpp"
#include "../../dny_framework/include/dny_VectorMath.hpp"


namespace dny
{
	class LevelEditor::TextureSelectMode : public IMode{
	public:
		TextureSelectMode( LevelEditor& editor, std::size_t active_idx );
		void render()const override;

	protected:
		void handle_mouse()override;
		void handle_keyboard()override;
		void handle_listbox( Mouse::Event const& event );
	protected:
		static constexpr Point center = m_editor_area.center();
		static constexpr RectF m_dialog_rect = RectF{
			Vec2f{ center - Point{ 200, 200 } },
			Vec2f{ center + Point{ 200, 200 } }
		};

		ui::ListBox<std::string> m_list_box;
		ui::DropDownBox m_dropdown_box;
		ui::Button m_accept;
		ui::Button m_cancel;
		LevelEditor* m_parent = nullptr;
		std::size_t m_active_index = {};
	};
}
