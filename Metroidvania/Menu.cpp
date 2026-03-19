#include "Menu.hpp"
#include "math/vector2.hpp"
#include "physics/physics.hpp"

namespace dny
{
	void Menu::update( Input& input_, float dt ){
		if(m_panel.children().empty()){
			for( auto& option : m_options ){
				m_panel.add_child( option );
			}
		}
		m_panel.update( input_.mouse(), input_.keyboard() );
		handle_keyboard( input_.keyboard() );
		handle_mouse( input_.mouse() );
	}

	void Menu::render( renderer2d& renderer_ ) const{
		m_panel.draw( renderer_, m_font );
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
		for( std::int32_t i = 0; auto& option : m_options ){
			if( !option->was_clicked() ){
				++i;
				continue;
			}

			m_selected_index = i;
			select_current();
			break;
		}
	}
}
