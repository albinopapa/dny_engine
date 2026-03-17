#include "editor/save_before_exit_mode.hpp"
#include "editor/editor_mode.hpp"

#include "input/input.hpp"

#include <memory>

namespace dny{
	// ---------- LevelEditor::SaveBeforeExitMode ----------
	LevelEditor::SaveBeforeExitMode::SaveBeforeExitMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect )
		:
		m_parent{ parent },
		m_dialog_panel{ "dialog_panel", "Unsaved Changes",
		dialog_rect.top_left(),
		dialog_rect.size() }{

		auto font = Font{ L"Consolas", 16 };
		auto button_dims = Font::measure_text( " Save and Exit ", font );
		button_dims.width += 5;
		button_dims.height += 5;
		auto offset = vector2<std::int32_t>{ 10, 50 };
		m_save = std::make_shared<ui::Button>( ui::Button{
			"save",
			" Save and Exit ",
			dialog_rect.top_left() + offset,
			button_dims,
		});
		
		offset.y += button_dims.height + 10;
		m_exit = std::make_shared<ui::Button>( ui::Button{
			"exit",
			" Exit no Save ",
			dialog_rect.top_left() + offset,
			button_dims,
		});
		
		offset.y += button_dims.height + 10;
		m_cancel = std::make_shared<ui::Button>( ui::Button{
			"cancel",
			" Cancel ",
			dialog_rect.top_left() + offset,
			button_dims,
		});

		m_dialog_panel.add_child( m_save );
		m_dialog_panel.add_child( m_exit );
		m_dialog_panel.add_child( m_cancel );

		if( m_parent.m_current_filename.empty() )
			m_save->set_enabled( false );
	}

    void LevelEditor::SaveBeforeExitMode::update( Mouse const& mouse, Keyboard& keyboard ){
		m_save->set_enabled( !m_parent.m_current_filename.empty() );
		m_dialog_panel.update( mouse, keyboard );

		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::SaveBeforeExitMode::render( renderer2d& renderer_, Font const& font_ ) const{
		m_dialog_panel.draw( renderer_, font_ );
	}

	void LevelEditor::SaveBeforeExitMode::handle_mouse( Mouse const& mouse ){
		const auto mouse_pos = mouse.position();
		
		if( mouse.is_pressed( MouseButton::Left ) ){
			if( m_save->contains( mouse_pos ) ){
				if( !m_save->enabled() ) return;

				// Save the file
				LevelSerializer::save( m_parent.m_document );
				m_parent.m_request = app_state_request::Menu;
			}
			else if( m_exit->contains( mouse_pos ) ){
				// Exit editor without saving
				m_parent.m_request = app_state_request::Menu;
			}
			else if( m_cancel->contains( mouse_pos ) ){
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
		}
	}

	void LevelEditor::SaveBeforeExitMode::handle_keyboard( Keyboard& keyboard ){
		if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
		else if( keyboard.is_pressed( Key::Enter ) ){
			if( !m_save->enabled() ) return;

			// Save the file
			LevelSerializer::save( m_parent.m_document );
			m_parent.m_request = app_state_request::Menu;
		}
	}
}
