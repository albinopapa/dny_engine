#include "editor/resize_mode.hpp"
#include "editor/save_before_exit_mode.hpp"

#include "input/input.hpp"

namespace dny
{
	// ---------- LevelEditor::ResizeMode ----------
	LevelEditor::ResizeMode::ResizeMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect )
		: 
		basic_mode{ "resize_mode", "Resize Tilemap", dialog_rect },
		m_parent{ parent }
	{
		const auto& font = m_parent.m_font;
		const auto box_dims = Font::measure_text( "000 ", font );
		const auto bounds = m_panel.bounds();
		const auto padding = 10;
		auto offset = vector2<std::int32_t>{ 10, font.char_height() + padding };
		m_width_input_box = std::make_shared<ui::InputTextBox>( 
			"width_input", "000 ",
			bounds.top_left() + offset,
			box_dims
		);

		offset.y += box_dims.height + padding;
		m_height_input_box = std::make_shared<ui::InputTextBox>( 
			"height_input", "000 ",
			bounds.top_left() + offset,
			box_dims
		);

		const auto button_dims = Font::measure_text( " Cancel ", font );
		offset.y += button_dims.height + padding;
		m_okay = std::make_shared<ui::Button>(
			"ok_button",
			" Ok ",
			bounds.top_left() + offset,
			button_dims
		);

		offset.x += 110;
		m_cancel = std::make_shared<ui::Button>(
			"cancel_button",
			" Cancel ",
			bounds.top_left() + offset,
			button_dims
		);

		m_okay->set_enabled( false );
		m_panel.add_child( m_width_input_box );
		m_panel.add_child( m_height_input_box );
		m_panel.add_child( m_okay );
		m_panel.add_child( m_cancel );
	}

	void LevelEditor::ResizeMode::update( Mouse const& mouse, Keyboard& keyboard ){
		m_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::ResizeMode::render( renderer2d& renderer_ ) const{
		m_panel.draw( renderer_, m_parent.m_font );
	}

	void LevelEditor::ResizeMode::handle_mouse( Mouse const& mouse ){
		if(m_okay->was_clicked() ){
			accept_and_resize();
		}
		else if(m_cancel->was_clicked() ){
			m_state = State::Done;
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
			m_state = State::Done;
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

		m_parent.m_document.tilemap.resize( { width, height } );
		m_state = State::Done;
	}
}
