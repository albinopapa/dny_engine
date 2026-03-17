#include "editor/load_mode.hpp"
#include "editor/editor_mode.hpp"

#include "input/input.hpp"

#include <filesystem>
#include <memory>

namespace dny{
	// ---------- LevelEditor::LoadMode ----------
	LevelEditor::LoadMode::LoadMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect_ )
		:
		m_parent{ parent }, m_dialog_panel{ "dialog_panel", "Load Level", dialog_rect_.top_left(), dialog_rect_.size() }{
		const auto offset = vector2<std::int32_t>{ 10, 10 };
		const auto lb_width = dialog_rect_.width() - 20;
		const auto lb_height = 30;
		m_list_box = std::make_shared<ui::ListBox>(
			"file_list",
			dialog_rect_.top_left() + offset,
			dims2<std::int32_t>{ lb_width, lb_height }
		);
		m_filename_input_box = std::make_shared<ui::InputTextBox>(
			"filename_input",
			"Filename: ",
			dialog_rect_.top_left() + offset + vector2{ 0, lb_height + 5 },
			dims2<std::int32_t>{ lb_width, lb_height }
		);
		m_load = std::make_shared<ui::Button>(
			"load_button",
			" Load ",
			dialog_rect_.top_left() + offset + vector2{ 0, lb_height * 2 + 10 },
			dims2<std::int32_t>{ 75, 30 }
		);
		m_cancel = std::make_shared<ui::Button>(
			"cancel_button",
			" Cancel ",
			dialog_rect_.top_left() + offset + vector2{ 150, lb_height * 2 + 10 },
			dims2<std::int32_t>{ 75, 30 }
		);

		m_dialog_panel.add_child( m_list_box );
		m_dialog_panel.add_child( m_filename_input_box );
		m_dialog_panel.add_child( m_load );
		m_dialog_panel.add_child( m_cancel );

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

				if( ext == ".lvl" ){
					m_list_box->add_item( entry.path().filename().string() );
				}
			}
		}
	}

	void LevelEditor::LoadMode::update( Mouse const& mouse, Keyboard& keyboard ){
		m_dialog_panel.update( mouse, keyboard );
	}

	void LevelEditor::LoadMode::render( renderer2d& renderer_, Font const& font_ ) const{
		m_dialog_panel.draw( renderer_, font_ );
	}

	void LevelEditor::LoadMode::handle_mouse( Mouse const& mouse_ ){
		if( mouse_.is_pressed( MouseButton::Left ) ){
			handle_listbox( mouse_ );
			if( m_load->contains( m_parent.m_mouse_position ) ){
				// Load the file
				// TODO: Implement TileMapSerializer
				//LevelSerializer::load( m_parent.m_document.tilemap, m_filename_input_box.get_text() );
				// m_parent.m_current_filename = m_filename_input_box.get_text();
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
			else if( m_cancel->contains( m_parent.m_mouse_position ) ){
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
		}

	}

	void LevelEditor::LoadMode::handle_keyboard( Keyboard& keyboard ){
		if( keyboard.is_pressed( Key::Enter ) ){
			// Load the file
			// TODO: Implement TileMapSerializer
			// TileMapSerializer::load( m_parent->m_document.tilemap, m_filename_input_box.get_text() );
			// m_parent->m_current_filename = m_filename_input_box.get_text();
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
	}

	void LevelEditor::LoadMode::handle_listbox( Mouse const& mouse_ ){
		if( !m_list_box->contains( mouse_.position() ) ){
			return;
		}

		// TODO: Handle mouse wheel for scrolling the listbox
		if( mouse_.wheel_delta() > 0 ){
			//m_list_box.scroll_up();
			return;
		}
		else if( mouse_.wheel_delta() < 0 ){
			//m_list_box.scroll_down();
			return;
		}

		auto str = m_list_box->selected_item();
		if( str.empty() ) return;

		m_filename_input_box->set_text( std::string{ str } );
	}
}
