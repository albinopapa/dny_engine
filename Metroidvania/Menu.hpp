#pragma once

#include "editor/iapp_state.hpp"
#include "input/input.hpp"
#include "renderer/renderer2d.hpp"
#include "graphics/font.hpp"
#include "ui/panel.hpp"
#include "ui/button.hpp"

#include <memory>
#include <string>
#include <vector>

namespace dny
{
	class Menu : public IAppState{
	public:
		void update( Input& input_, float dt )override;
		void render( renderer2d& renderer_ ) const override;

	private:
		void move_selection_up();
		void move_selection_down();
		void select_current();

		void handle_keyboard( Keyboard& keyboard );
		void handle_mouse( Mouse const& mouse );

	private:
		static constexpr Rect<std::int32_t> m_panel_rect = { 50, 100, 400, 300 };
		static constexpr Rect<std::int32_t> m_button_rect = { 0, 0, 100, 50 };
		static constexpr ColorF m_selected_color = ColorF( 1.0f, 1.0f, 0.0f, 1.0f );
		static constexpr ColorF m_unselected_color = ColorF( 0.5f, 0.5f, 0.5f, 1.0f );
		static constexpr vector2<float> m_start_pos = vector2<float>{ 100.0f, 150.0f };
		static constexpr std::int32_t m_spacing = 50;

		std::vector<std::shared_ptr<ui::Button>> m_options{
			std::make_shared<ui::Button>( "editor", "Editor", m_panel_rect.top_left(), m_button_rect.size() ),
			std::make_shared<ui::Button>( "game", "Game",     m_panel_rect.top_left() + vector2<std::int32_t>{0, m_spacing }, m_button_rect.size() ),
			std::make_shared<ui::Button>( "exit", "Exit",     m_panel_rect.top_left() + vector2<std::int32_t>{0, m_spacing *2}, m_button_rect.size() )
		};
		ui::Panel m_panel{ "main_menu_panel", "", m_panel_rect.top_left(), m_panel_rect.size()};

		ColorF m_color = m_unselected_color;
		std::int32_t m_selected_index = 0;

		Font m_font = Font{ L"Arial", 24 };
	};
}
