#pragma once

#include "level_editor.hpp"
#include "imode.hpp"

namespace dny{
	class LevelEditor::EditorMode :public IMode{
	public:
		EditorMode( LevelEditor& parent, Rect<std::int32_t> workspace );

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void render( renderer2d& renderer, Font const& font )const override;

	private:
		void handle_mouse( Mouse const& mouse );
		void handle_keyboard( Keyboard& keyboard );
		void handle_mouse_wheel( Mouse const& mouse );
		void place_tile( Mouse const& mouse );
	private:
		LevelEditor& m_parent;
		Rect<std::int32_t> m_workspace;
	};
}
