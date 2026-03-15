#include "SaveMode.hpp"
#include "EditorMode.hpp"
#include "Input.hpp"

namespace dny
{
	// ---------- LevelEditor::SaveMode ----------
	LevelEditor::SaveMode::SaveMode( LevelEditor& parent )
		:
		m_parent{ &parent }{
		m_filename_input_box = UIInputBox{
			Graphics::calculate_rect( "Filename: " )
			+ m_dialog_rect.top_left() + Vec2f{ 10.f, 50.f },
			"Filename: ",
			""
		};
		m_okay_button = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 50.f, 100.f },
			" Save "
		};
		m_cancel_button = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 225.f, 100.f },
			" Cancel "
		};
	}

	void LevelEditor::SaveMode::handle_mouse(){
		auto mouse = Mouse::instance();
		const auto mouse_pos = Vec2f{ mouse.position() };
		while( !mouse.is_event_queue_empty() ){
			const auto event = mouse.read_event();
			if( !Input::is_button_release_event( event, Mouse::Button::Left ) )continue;

			if( m_okay_button.is_hit( mouse_pos ) ){
				if( !m_okay_button.is_enabled() )continue;

				TileMapSerializer::save( m_parent->m_tilemap, m_filename_input_box.get_text() );
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
			else if( m_cancel_button.is_hit( mouse_pos ) ){
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
		}
	}

	void LevelEditor::SaveMode::handle_keyboard(){
		auto& keyboard = Keyboard::instance();
		while( !keyboard.is_char_queue_empty() ){
			const auto ch = keyboard.read_key();

			if( m_filename_input_box.get_text().size() < 32 ){
				m_filename_input_box.on_text_input( ch );
			}
		}
		m_filename_input_box.get_text().empty()
			? m_okay_button.disable() : m_okay_button.enable();

		while( !keyboard.is_event_queue_empty() ){
			const auto event = keyboard.read_event();

			if( Input::is_button_release_event(event, VK_RETURN ) ){
				if( m_filename_input_box.get_text().empty() )continue;

				TileMapSerializer::save( m_parent->m_tilemap, m_filename_input_box.get_text() );
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
			else if( Input::is_button_release_event( event, VK_ESCAPE ) ){
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
		}
	}

	void LevelEditor::SaveMode::render() const{
		// Draw dialog outline
		Graphics::instance().draw_rectangle( m_dialog_rect, Colors::White );
		// Draw filename input label and box
		m_filename_input_box.render();
		// Draw OK button
		m_okay_button.render();
		// Draw Cancel button
		m_cancel_button.render();
	}
}
