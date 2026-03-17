#include "editor/save_mode.hpp"
#include "editor/editor_mode.hpp"

#include "input/input.hpp"

namespace dny{
	// ---------- LevelEditor::SaveMode ----------
	LevelEditor::SaveMode::SaveMode( LevelEditor& parent_, Rect<std::int32_t> const& dialog_rect_ )
		:
		m_parent{ parent_ },
		m_dialog_panel{
			std::string{ "save_panel" },
			std::string{ "Save Level" },
			dialog_rect_.top_left(),
			dialog_rect_.size() }{

		const auto button_dims = dims2<std::int32_t>{ 50, 30 };
		m_save = std::make_shared<ui::Button>(
			"save_button",
			"Save Level",
			vector2<std::int32_t>{dialog_rect_.left + 10, dialog_rect_.top + 10 },
			button_dims
		);

		m_dialog_panel.add_child( m_save );
		m_filename_input_box = std::make_shared<ui::InputTextBox>(
			"filename_input",
			"Filename: ",
			vector2<std::int32_t>{ dialog_rect_.left + 10, dialog_rect_.top + 50 },
			dims2<std::int32_t>{ 75, 30 }
		);
		m_cancel = std::make_shared<ui::Button>(
			"cancel_button",
			" Cancel ",
			vector2<std::int32_t>{dialog_rect_.left + 225, dialog_rect_.top + 100},
			button_dims
		);
	}

	void LevelEditor::SaveMode::update( Mouse const& mouse, Keyboard& keyboard ){
		m_dialog_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::SaveMode::handle_mouse( Mouse const& mouse ){
		m_save->set_enabled( !m_filename_input_box->text().empty() );
		if( !mouse.is_pressed( MouseButton::Left ) ) return;
		if( m_save->contains( m_parent.m_mouse_position ) && m_save->enabled() ){
			// TODO: Implement TileMapSerializer
			// TileMapSerializer::save( m_parent->m_document.tilemap, m_filename_input_box.get_text() );
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
		else if( m_cancel->contains( m_parent.m_mouse_position ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
	}

	void LevelEditor::SaveMode::handle_keyboard( Keyboard& keyboard ){
		m_save->set_enabled(!m_filename_input_box->text().empty());
		
		if( keyboard.is_pressed( Key::Enter ) && m_save->enabled() ){			
			// TODO: Implement TileMapSerializer
			// TileMapSerializer::save( m_parent->m_document.tilemap, m_filename_input_box->text() );
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
	}

	void LevelEditor::SaveMode::render( renderer2d& renderer_, Font const& font_ ) const{
		m_dialog_panel.draw( renderer_, font_ );
	}
}
