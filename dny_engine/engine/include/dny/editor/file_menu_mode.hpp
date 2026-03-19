#pragma once

#include "imode.hpp"
#include "level_editor.hpp"

namespace dny{
	class LevelEditor::FileMenuMode : public basic_mode{
	public:
		FileMenuMode( LevelEditor& parent_, Rect<std::int32_t> const& dialog_rect_ );
		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void render( renderer2d& renderer_ )const override;
	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		void on_exit();
		void on_save();
		void on_load();
	private:
		std::shared_ptr<ui::Button> m_load;
		std::shared_ptr<ui::Button> m_save;
		std::shared_ptr<ui::Button> m_exit;
		LevelEditor& m_parent;

		std::int32_t m_focus_index = 0;
	};
}