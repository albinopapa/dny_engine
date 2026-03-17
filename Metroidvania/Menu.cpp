#include "Menu.hpp"
#include "math/vector2.hpp"
#include "physics/physics.hpp"

namespace dny
{
	void Menu::update( Input& input_, float dt ){
		handle_keyboard( input_.keyboard() );
		handle_mouse( input_.mouse() );
	}

	void Menu::render( renderer2d& renderer_, Font const& font_ ) const{
		for( std::size_t i = 0; i < m_options.size(); ++i ){
			const auto y = m_start_pos.y + static_cast< float >( i ) * m_spacing;
			const auto selected = ( static_cast< int >( i ) == m_selected_index );

			const auto color = selected ? m_selected_color : m_unselected_color;

			const auto position = vector2<float>{ m_start_pos.x, y };
			renderer_.draw_text( m_options[ i ], position, font_, color );
		}
	}

	void Menu::move_selection_up(){
		m_selected_index = std::clamp( m_selected_index - 1, 0, static_cast< std::int32_t >( m_options.size() ) - 1 );
	}

	void Menu::move_selection_down(){
		m_selected_index = std::clamp( m_selected_index + 1, 0, static_cast< std::int32_t >( m_options.size() ) - 1 );
	}

	void Menu::select_current(){
		switch( m_selected_index ){
			case 0: m_request  = app_state_request::Editor; break;
			case 1: m_request  = app_state_request::Game;   break;
			case 2: m_request  = app_state_request::Exit;   break;
			default: m_request = app_state_request::None;  break;
		}
	}

	void Menu::handle_keyboard( Keyboard& keyboard ){
		if(keyboard.is_pressed(Key::Enter)){
			select_current();
		}
		else if(keyboard.is_pressed(Key::Up)){
			move_selection_up();
		}
		else if(keyboard.is_pressed(Key::Down)){
			move_selection_down();
		}
	}

	void Menu::handle_mouse( Mouse const& mouse ){
		const auto mouse_pos = mouse.position();
		const auto mouse_pos_f = vector2<float>{
			static_cast< float >( mouse_pos.x ),
			static_cast< float >( mouse_pos.y )
		};

		for( std::size_t i = 0; i < m_options.size(); ++i ){
			const auto y = m_start_pos.y + static_cast< float >( i ) * m_spacing;
			const auto button_translated = m_button_rect + vector2<float>{ m_start_pos.x, y };

			if( !contains( button_translated, mouse_pos_f ) ){
				continue;
			}

			m_selected_index = static_cast< std::int32_t >( i );
			if( mouse.is_pressed( MouseButton::Left ) ){
				select_current();
			}
		}
	}
}
