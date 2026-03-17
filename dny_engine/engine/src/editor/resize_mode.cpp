#include "editor/resize_mode.hpp"
#include "editor/editor_mode.hpp"
#include "editor/save_before_exit_mode.hpp"

#include "input/input.hpp"

namespace dny
{
	// ---------- LevelEditor::ResizeMode ----------
	LevelEditor::ResizeMode::ResizeMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect )
		: m_parent{ parent },
		m_dialog_panel{ "dialog_panel", "Resize Tilemap", dialog_rect.top_left(), dialog_rect.size() }
	{
		m_width_input_box = std::make_shared<ui::InputTextBox>( 
			"width_input", "Width Input",
			vector2<std::int32_t>{ 10, 50 },
			dims2<std::int32_t>{ 100, 30 }
		);
		m_height_input_box = std::make_shared<ui::InputTextBox>( 
			"height_input", "Height Input",
			vector2<std::int32_t>{ 10, 100 },
			dims2<std::int32_t>{ 100, 30 } 
		);
		m_okay = std::make_shared<ui::Button>( 
			"ok_button", 
			" Ok ", 
			vector2<std::int32_t>{ 50, -50 }, 
			dims2<std::int32_t>{ 75, 30 } 
		);
		m_cancel = std::make_shared<ui::Button>( 
			"cancel_button", 
			" Cancel ", 
			vector2<std::int32_t>{ 200, -50 }, 
			dims2<std::int32_t>{ 75, 30 }
		);

		m_okay->set_enabled( false );
		m_dialog_panel.add_child( m_width_input_box );
		m_dialog_panel.add_child( m_height_input_box );
		m_dialog_panel.add_child( m_okay );
		m_dialog_panel.add_child( m_cancel );
	}

	void LevelEditor::ResizeMode::update( Mouse const& mouse, Keyboard& keyboard ){
		m_dialog_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::ResizeMode::render( renderer2d& renderer_, Font const& font_ ) const{
		m_dialog_panel.draw( renderer_, font_ );
	}

	void LevelEditor::ResizeMode::handle_mouse( Mouse const& mouse ){
		if( mouse.is_pressed( MouseButton::Left ) ){
			if( m_okay->contains( mouse.position() ) ){
				accept_and_resize();
			}
			else if( m_cancel->contains( mouse.position() ) ){
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
		}
	}

	void LevelEditor::ResizeMode::handle_keyboard( Keyboard& keyboard ){
		// Handle user text input
		auto& field = ( m_focus == Focus::WidthField ) ? m_width_input_box : m_height_input_box;

		m_okay->set_enabled( 
			!m_width_input_box->text().empty() && 
			!m_height_input_box->text().empty() 
		);


		// Handle user navigation input
		if( keyboard.is_pressed( Key::Enter ) ){
			accept_and_resize();
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<SaveBeforeExitMode>( m_parent, m_dialog_panel.bounds() ) );
		}
		else if( keyboard.is_pressed( Key::Tab ) ){
			if( m_focus == Focus::WidthField ){
				m_focus = Focus::HeightField;
			}
			else{
				m_focus = Focus::WidthField;
			}
		}
	}

	void LevelEditor::ResizeMode::accept_and_resize(){
		auto string_to_int = []( std::string_view str ) -> int{
			char* end_iter = nullptr;
			const auto value = std::strtol( str.data(), std::addressof( end_iter ), 10 );
			if( end_iter == str.data() ){
				return 0; // conversion failed
			}
			return static_cast< std::int32_t >( value );
		};

		if( m_width_input_box->text().empty() ) return;
		if( m_height_input_box->text().empty() ) return;

		const auto width = string_to_int( m_width_input_box->text() );
		if( width < 10 || width > 1000 )return;

		const auto height = string_to_int( m_height_input_box->text() );
		if( height < 10 || height > 1000 )return;

		m_parent.resize_tilemap( { width, height } );
		m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
	}
}
