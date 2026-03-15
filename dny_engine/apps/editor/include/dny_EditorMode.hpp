#pragma once

#include "dny_LevelEditor.hpp"
#include "dny_IMode.hpp"

namespace dny
{
	class LevelEditor::EditorMode :public IMode{
	public:
		EditorMode( LevelEditor& parent );
		void render()const override;

	private:
		void handle_mouse( Input const& input_ );
		void handle_keyboard( Input const& input_ );
		void handle_mouse_wheel( Input const& input_ );

	private:
		LevelEditor* m_parent = nullptr;
	};

}
