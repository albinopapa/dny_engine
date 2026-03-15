#include "LoadMode.hpp"
#include "EditorMode.hpp"
#include "Input.hpp"

#include <filesystem>
#include <memory>

namespace dny
{
	// ---------- LevelEditor::LoadMode ----------
	LevelEditor::LoadMode::LoadMode( LevelEditor& parent )
		:
		m_parent{ &parent }{
		auto string_box =
			Graphics::calculate_rect( std::string{ "ABCDEFGHIJKLM" } ); // NOPQRSTUVWXYZ012345
		string_box.bottom = string_box.height() * 10.f;
		m_list_box = UIListBox<std::string>{
			string_box + m_dialog_rect.top_left() + Vec2f{ 10.f, 10.f },
			""
		};

		m_filename_input_box = UIInputBox{
			Graphics::calculate_rect( "Filename: " )
			+ m_dialog_rect.top_left() + Vec2f{ 10.f, m_list_box.border().bottom },
			"Filename: ",
			""
		};
		m_okay_button = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 50.f, m_filename_input_box.border().bottom },
			" Load "
		};
		m_cancel_button = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.top_left() + Vec2f{ 225.f, m_filename_input_box.border().bottom },
			" Cancel "
		};

		namespace fs = std::filesystem;

		const auto level_dir = fs::current_path();

		if( !fs::exists( level_dir ) || !fs::is_directory( level_dir ) ){
			return;
		}

		for( const auto& entry : fs::directory_iterator( level_dir ) ){
			if( entry.is_regular_file() ){
				auto ext = entry.path().extension().string();
				std::transform(
					ext.begin(),
					ext.end(),
					ext.begin(),
					[]( unsigned char ch ){return std::tolower( ch ); }
				);

				if( ext == ".txt" || ext == ".lvl" ){
					m_list_box.add_option( entry.path().filename().string() );
				}
			}
		}
	}

	void LevelEditor::LoadMode::render() const{
		// Draw dialog outline
		Graphics::instance().draw_rectangle( m_dialog_rect, Colors::White );
		// Draw filename input label and box
		m_filename_input_box.render();
		// Draw file list
		m_list_box.render();
		// Draw OK button
		m_okay_button.render();
		// Draw Cancel button
		m_cancel_button.render();
	}

	void LevelEditor::LoadMode::handle_mouse(){
		const auto mouse_pos = Vec2f{ Mouse::instance().position() };
		while( !Mouse::instance().is_event_queue_empty() ){
			const auto event = Mouse::instance().read_event();

			if( Input::is_button_release_event( event, Mouse::Button::Left ) ){
				if( m_okay_button.is_hit( mouse_pos ) ){
					// Load the file
					TileMapSerializer::load( m_parent->m_tilemap, m_filename_input_box.get_text() );
					m_parent->m_current_filename = m_filename_input_box.get_text();
					m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
				}
				else if( m_cancel_button.is_hit( mouse_pos ) ){
					m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
				}
			}

			handle_listbox( event );
		}
	}

	void LevelEditor::LoadMode::handle_keyboard(){
		// Handle user text input
		auto& keyboard = Keyboard::instance();
		while( !keyboard.is_char_queue_empty() ){
			const auto ch = keyboard.read_key();

			if( m_filename_input_box.get_text().size() < 32 ){
				m_filename_input_box.on_text_input( ch );
			}
		}
		while( !keyboard.is_event_queue_empty() ){
			const auto event = keyboard.read_event();

			if( Input::is_button_release_event( event, VK_RETURN ) ){
				// Load the file
				TileMapSerializer::load( m_parent->m_tilemap, m_filename_input_box.get_text() );
				m_parent->m_current_filename = m_filename_input_box.get_text();
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
			else if( Input::is_button_release_event( event, VK_ESCAPE ) ) {
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
		}
	}

	void LevelEditor::LoadMode::handle_listbox( Mouse::Event const& event ){
		const auto mouse_pos = Vec2f{ event.position() };
		if( event.type() == Mouse::Event::Type::WheelUp ){
			m_list_box.scroll_up();
			return;
		}
		else if( event.type() == Mouse::Event::Type::WheelDown ){
			m_list_box.scroll_down();
			return;
		}

		if( !Input::is_button_release_event( event, Mouse::Button::Left ) ){
			return;
		}

		const auto row = m_list_box.entry_index( mouse_pos );
		m_list_box.set_hovered( row );
		if( !row ){
			return;
		}

		const auto& str = m_list_box.select_entry( *row );
		m_filename_input_box.get_text() = str;
	}
}
