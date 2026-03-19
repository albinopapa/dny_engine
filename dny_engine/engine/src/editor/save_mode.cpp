#include "editor/save_mode.hpp"
#include "input/input.hpp"

namespace dny{
	// ---------- LevelEditor::SaveMode ----------
	LevelEditor::SaveMode::SaveMode( LevelEditor& parent_, Rect<std::int32_t> const& dialog_rect_ )
		:
		basic_mode{ "save_mode", "Save Level", dialog_rect_ },
		m_parent{ parent_ }{

		const auto button_dims = dims2<std::int32_t>{ 82, 30 };
		auto offset = vector2<std::int32_t>{ 10, 30 };
		m_filename_input_box = std::make_shared<ui::InputTextBox>(
			"filename_input",
			"Enter filename...",
			dialog_rect_.top_left() + offset,
			dims2<std::int32_t>{ 200, 31 }
		);

		offset.y += 40;
		m_save = std::make_shared<ui::Button>(
			"save_button",
			"  Save  ",
			dialog_rect_.top_left() + offset,
			button_dims
		);

		offset.x += 110;
		m_cancel = std::make_shared<ui::Button>(
			"cancel_button",
			" Cancel ",
			dialog_rect_.top_left() + offset,
			button_dims
		);
		m_panel.add_child( m_save );
		m_panel.add_child( m_cancel );
		m_panel.add_child( m_filename_input_box );
	}

	void LevelEditor::SaveMode::update( Mouse const& mouse, Keyboard& keyboard ){
		if( !m_parent.m_document.basename.empty() ){
			m_filename_input_box->set_text( m_parent.m_document.basename );
		}
		m_save->set_enabled( !m_filename_input_box->text().empty() );

		m_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::SaveMode::render( renderer2d& renderer_ ) const{
		m_panel.draw( renderer_, m_parent.m_font );
	}

	void LevelEditor::SaveMode::handle_mouse( Mouse const& mouse ){
		if( m_save->was_clicked() && m_save->enabled() ){
			on_save();
		}
		else if( m_cancel->was_clicked() ){
			on_cancel();
		}
	}

	void LevelEditor::SaveMode::handle_keyboard( Keyboard& keyboard ){

		if( keyboard.is_pressed( Key::Enter ) && m_save->enabled() ){
			on_save();
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			on_cancel();
		}
	}

	void LevelEditor::SaveMode::on_save(){
		m_parent.m_document.basename = m_filename_input_box->text();
		LevelSerializer::save( m_parent.m_document );
		m_state = State::Done;
	}

	void LevelEditor::SaveMode::on_cancel(){
		m_state = State::Done;
	}
}
