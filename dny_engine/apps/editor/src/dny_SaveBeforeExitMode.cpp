#include "SaveBeforeExitMode.hpp"
#include "EditorMode.hpp"
#include "Input.hpp"

#include <memory>

namespace dny{
	// ---------- LevelEditor::SaveBeforeExitMode ----------
	LevelEditor::SaveBeforeExitMode::SaveBeforeExitMode( LevelEditor& parent )
		:
		m_parent{ &parent }{

		m_save_and_exit_button = UIButton{
			Graphics::calculate_rect( "000000000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 50.f, 75.f },
			" Save and Exit "
		};
		m_exit_without_saving_button = UIButton{
			Graphics::calculate_rect( "000000000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 50.f, 125.f },
			" Exit no Save "
		};
		m_cancel_button = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 50.f, 175.f },
			" Cancel "
		};

		if( m_parent->m_current_filename.empty() )
			m_save_and_exit_button.disable();
	}

	void LevelEditor::SaveBeforeExitMode::render() const{
		// Draw dialog outline
		Graphics::instance().draw_rectangle( m_dialog_rect, Colors::White );

		// Draw buttons
		m_save_and_exit_button.render();
		m_exit_without_saving_button.render();
		m_cancel_button.render();
	}

	void LevelEditor::SaveBeforeExitMode::handle_mouse(){
		const auto mouse_pos = Vec2f{ Mouse::instance().position() };
		while( !Mouse::instance().is_event_queue_empty() ){
			const auto event = Mouse::instance().read_event();
			if( !Input::is_button_release_event( event, Mouse::Button::Left ) )continue;

			if( m_save_and_exit_button.is_hit( mouse_pos ) ){
				if( !m_save_and_exit_button.is_enabled() )continue;

				// Save the file
				TileMapSerializer::save( m_parent->m_tilemap, m_parent->m_current_filename );
				m_parent->m_request = IAppState::Request::Menu;
			}
			else if( m_exit_without_saving_button.is_hit( mouse_pos ) ){
				// Exit editor without saving
				m_parent->m_request = IAppState::Request::Menu;
			}
			else if( m_cancel_button.is_hit( mouse_pos ) ){
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
		}
	}

	void LevelEditor::SaveBeforeExitMode::handle_keyboard(){
		auto& keyboard = Keyboard::instance();

		while( !keyboard.is_event_queue_empty() ){
			const auto event = keyboard.read_event();
			if( Input::is_button_release_event( event, VK_ESCAPE ) ){
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
			else if( Input::is_button_release_event( event, VK_RETURN ) ){
				if( m_parent->m_current_filename.empty() )continue;

				// Save the file
				TileMapSerializer::save( m_parent->m_tilemap, m_parent->m_current_filename );
				m_parent->m_request = IAppState::Request::Menu;
			}
		}
	}
}
