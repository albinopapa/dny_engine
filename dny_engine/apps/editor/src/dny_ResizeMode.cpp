#include "ResizeMode.hpp"
#include "EditorMode.hpp"
#include "Input.hpp"
#include "SaveBeforeExitMode.hpp"

namespace dny
{
	// ---------- LevelEditor::ResizeMode ----------
	LevelEditor::ResizeMode::ResizeMode( LevelEditor& parent )
		: m_parent{ &parent }{
		auto make_input_box = [ & ]( Vec2f const& offset, std::string label, std::string in_string ){
			return UIInputBox{
				Graphics::calculate_rect( label )
				+ m_dialog_rect.top_left() + offset,
				label,
				std::move( in_string )
			};
		};
		auto make_button = [ & ]( Vec2f const& offset, std::string str ){
			return UIButton{
				Graphics::calculate_rect( "00000000" )
				+ m_dialog_rect.bottom_left() + offset,
				str
			};
		};

		m_width_input_box = make_input_box( Vec2f{ 10.f, 50.f }, "Width: ", "" );
		m_height_input_box = make_input_box( Vec2f{ 10.f, 100.f }, "Height: ", "" );
		m_okay_button = make_button( Vec2f{ 50.f, -50.f }, " Ok " );
		m_cancel_button = make_button( Vec2f{ 200.f, -50.f }, " Cancel " );

		m_okay_button.disable();
	}

	void LevelEditor::ResizeMode::render() const{
		// Draw dialog outline
		Graphics::instance().draw_rectangle( m_dialog_rect, Colors::White );

		// Draw width input label and box
		m_width_input_box.render();
		// Draw height input label and box
		m_height_input_box.render();

		// Draw OK button
		m_okay_button.render();
		// Draw Cancel button
		m_cancel_button.render();
	}

	void LevelEditor::ResizeMode::handle_mouse(){
		const auto mouse_pos = Vec2f{ Mouse::instance().position() };

		while( !Mouse::instance().is_event_queue_empty() ){
			const auto event = Mouse::instance().read_event();
			if( !Input::is_button_release_event( event, Mouse::Button::Left ) )continue;

			if( m_okay_button.is_hit( mouse_pos ) ){
				accept_and_resize();
			}
			else if( m_cancel_button.is_hit( mouse_pos ) ){
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
		}
	}

	void LevelEditor::ResizeMode::handle_keyboard(){
		// Handle user text input
		auto& field = ( m_focus == Focus::WidthField ) ? m_width_input_box : m_height_input_box;
		auto& keyboard = Keyboard::instance();

		while( !keyboard.is_char_queue_empty() ){
			const auto ch = keyboard.read_key();

			if( field.get_text().size() < 4 ){
				field.on_text_input( ch );
			}
		}

		const auto disable_okay = 
			m_width_input_box.get_text().empty() || m_height_input_box.get_text().empty();

		if( disable_okay ){
			m_okay_button.disable();
		}
		else{
			m_okay_button.enable();
		}

		// Handle user navigation input
		while( !keyboard.is_event_queue_empty() ){
			const auto event = keyboard.read_event();
			if( Input::is_button_release_event( event, VK_RETURN ) ){
				accept_and_resize();
			}
			else if( Input::is_button_release_event( event, VK_ESCAPE ) ){
				m_parent->transition_mode( std::make_unique<SaveBeforeExitMode>( *m_parent ) );
			}
			else if( Input::is_button_release_event( event, VK_TAB ) ){
				if( m_focus == Focus::WidthField ){
					m_focus = Focus::HeightField;
				}
				else{
					m_focus = Focus::WidthField;
				}
			}
		}
	}

	void LevelEditor::ResizeMode::accept_and_resize(){
		auto string_to_int = []( std::string const& str ) -> int{
			char* end_iter = nullptr;
			const auto value = std::strtol( str.c_str(), std::addressof( end_iter ), 10 );
			if( end_iter == str.c_str() ){
				return 0; // conversion failed
			}
			return static_cast< std::int32_t >( value );
		};

		if( m_width_input_box.get_text().empty() ) return;
		if( m_height_input_box.get_text().empty() ) return;

		const auto width = string_to_int( m_width_input_box.get_text() );
		if( width < 10 || width > 1000 )return;

		const auto height = string_to_int( m_height_input_box.get_text() );
		if( height < 10 || height > 1000 )return;

		m_parent->resize_tilemap( { width, height } );
		m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
	}
}
