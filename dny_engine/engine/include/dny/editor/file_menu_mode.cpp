#include "file_menu_mode.hpp"
#include "editor/save_mode.hpp"
#include "editor/load_mode.hpp"
#include "editor/save_before_exit_mode.hpp"

dny::LevelEditor::FileMenuMode::FileMenuMode( LevelEditor& parent_, Rect<std::int32_t> const& dialog_rect_ )
	:
	basic_mode{ "file_menu_mode", "File Menu", dialog_rect_ },
	m_parent{ parent_ }
{
	const auto button_dims = dims2<std::int32_t>{ 82, 30 };
	auto offset = vector2<std::int32_t>{ 10, 30 };
	m_save = std::make_shared<ui::Button>(
		"save_button",
		"  Save  ",
		dialog_rect_.top_left() + offset,
		button_dims
	);
	offset.y += 40;
	m_load = std::make_shared<ui::Button>(
		"load_button",
		"  Load  ",
		dialog_rect_.top_left() + offset,
		button_dims
	);
	offset.y += 40;
	m_exit = std::make_shared<ui::Button>(
		"exit_button",
		"  Exit  ",
		dialog_rect_.top_left() + offset,
		button_dims
	);
	m_panel.add_child( m_save );
	m_panel.add_child( m_load );
	m_panel.add_child( m_exit );
}

void dny::LevelEditor::FileMenuMode::update( Mouse const& mouse, Keyboard& keyboard ){
	if( !m_parent.m_dirty ){
		m_save->set_enabled( false );
	}
	else{
		m_save->set_enabled( true );
	}
	m_panel.update( mouse, keyboard );
	handle_mouse( mouse );
	handle_keyboard( keyboard );
}

void dny::LevelEditor::FileMenuMode::render( renderer2d& renderer_ ) const{
	m_panel.draw( renderer_, m_parent.m_font );
}

void dny::LevelEditor::FileMenuMode::handle_mouse( Mouse const& mouse ){
	if( m_load->was_clicked() ){
		on_load();
	}
	else if( m_save->was_clicked() ){
		on_save();
	}
	else if( m_exit->was_clicked() ){
		on_exit();
	}
}

void dny::LevelEditor::FileMenuMode::handle_keyboard( Keyboard& keyboard ){
	if( keyboard.is_pressed( Key::Escape ) ){
		m_state = State::Done;
	}
	else if( keyboard.is_pressed( Key::Enter ) ){
		switch( m_focus_index ){
			case 0:
				on_load();
				break;
			case 1:
				on_save();
				break;
			case 2:
				on_exit();
				break;
			default:
				break;
		}
	}
	else if( keyboard.is_pressed( Key::Down ) ){
		m_focus_index = ( m_focus_index + 1 ) % 3;
	}
	else if( keyboard.is_pressed( Key::Up ) ){
		m_focus_index = ( m_focus_index - 1 + 3 ) % 3;
	}
}

void dny::LevelEditor::FileMenuMode::on_exit(){
	if( m_parent.m_dirty ){
		m_parent.transition_mode( std::make_unique<SaveBeforeExitMode>( m_parent, m_panel.bounds() ) );
	}
	else{
		m_state = State::Done;
	}
}

void dny::LevelEditor::FileMenuMode::on_save(){
	if( m_save->enabled() )
		m_parent.transition_mode( std::make_unique<SaveMode>( m_parent, m_panel.bounds() ) );
}

void dny::LevelEditor::FileMenuMode::on_load(){
	m_parent.transition_mode( std::make_unique<LoadMode>( m_parent, m_panel.bounds() ) );
}
