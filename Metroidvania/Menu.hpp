#pragma once

#include "editor/iapp_state.hpp"
#include "input/input.hpp"
#include "renderer/renderer2d.hpp"

#include <string>
#include <vector>

namespace dny
{
	class Menu : public IAppState{
	public:
		void update( Input& input_, float dt )override;
		void render( renderer2d& renderer_, Font const& font_ ) const override;

	private:
		void move_selection_up();
		void move_selection_down();
		void select_current();

		void handle_keyboard( Keyboard& keyboard );
		void handle_mouse( Mouse const& mouse );

	private:
		static constexpr Rect<float> m_button_rect = { 0.f, 0.f, 100.f, 50.f };
		static constexpr ColorF m_selected_color = ColorF( 1.0f, 1.0f, 0.0f, 1.0f );
		static constexpr ColorF m_unselected_color = ColorF( 0.5f, 0.5f, 0.5f, 1.0f );
		static constexpr vector2<float> m_start_pos = vector2<float>{ 100.0f, 150.0f };
		static constexpr float m_spacing = 50.0f;

		std::vector<std::string> m_options{
			"Editor",
			"Game",
			"Exit"
		};
		ColorF m_color = m_unselected_color;
		std::int32_t m_selected_index = 0;
	};
}
