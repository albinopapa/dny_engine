#include "editor/load_mode.hpp"

#include "input/input.hpp"

#include <filesystem>
#include <memory>

namespace dny{
	// ---------- LevelEditor::LoadMode ----------
	LevelEditor::LoadMode::LoadMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect_ )
		:
		basic_mode{ "load_mode", "Load Level", dialog_rect_ },
		m_parent{ parent }{
		const auto& font = m_parent.m_font;
		auto offset = vector2<std::int32_t>{ 10, 10 };
		const auto lb_width = dialog_rect_.width() - 20;
		const auto lb_height = font.char_height();
		const auto padding = 5;
		const auto str_dims = Font::measure_text( " Cancel ", font );

		m_dropdown = std::make_shared<ui::Dropdown>(
			"file_list",
			dialog_rect_.top_left() + offset,
			dims2<std::int32_t>{ lb_width, lb_height }
		);

		offset.y += lb_height + padding;
		m_filename_input_box = std::make_shared<ui::InputTextBox>(
			"filename_input",
			"Filename: ",
			dialog_rect_.top_left() + offset,
			dims2<std::int32_t>{ lb_width, lb_height }
		);

		offset.y += lb_height + padding;
		m_load = std::make_shared<ui::Button>(
			"load_button",
			" Load ",
			dialog_rect_.top_left() + offset,
			str_dims
		);

		offset.x += str_dims.width + padding;
		m_cancel = std::make_shared<ui::Button>(
			"cancel_button",
			" Cancel ",
			dialog_rect_.top_left() + offset,
			str_dims
		);

		m_panel.add_child( m_filename_input_box );
		m_panel.add_child( m_load );
		m_panel.add_child( m_cancel );
		m_panel.add_child( m_dropdown );

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
					m_dropdown->add_item( entry.path().filename().string() );
				}
			}
		}
	}

	void LevelEditor::LoadMode::update( Mouse const& mouse, Keyboard& keyboard ){
		if( m_dropdown->items().empty() ){
			m_filename_input_box->set_text( "No .lvl files found" );
			m_filename_input_box->set_enabled( false );
			m_load->set_enabled( false );
		}
		else{
			m_filename_input_box->set_enabled( true );
			m_load->set_enabled( true );
		}
		m_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::LoadMode::render( renderer2d& renderer_ )const{
		m_panel.draw( renderer_, m_parent.m_font );

		// Draw the list box items with scrolling
		//const auto& items = m_dropdown->items();
		//const auto scroll_value = m_dropdown->scroll_value();
		//const auto char_height = m_parent.m_font.char_height();
		//const auto padding = 2;
		//for( std::size_t i = 0; i < items.size(); ++i ){
		//	const auto record_pos = static_cast< std::int32_t >( i ) * char_height;
		//	const auto item_pos = m_dropdown->position() + vector2{ padding, padding + record_pos - scroll_value * char_height };
		//	if( item_pos.y + char_height < m_dropdown->bounds().top || item_pos.y > m_dropdown->bounds().bottom ){
		//		continue; // Skip items outside the visible area
		//	}
		//	renderer_.draw_text(items[ i ], item_pos, m_parent.m_font, Color32{ 255, 255, 255, 255 } );
		//}
	}


	void LevelEditor::LoadMode::handle_mouse( Mouse const& mouse_ ){
		if( mouse_.is_pressed( MouseButton::Left ) ){
			handle_listbox( mouse_ );
			if( m_load->was_clicked() ){
				if( !m_load->enabled() ) return;

				// Load the file
				m_parent.m_document.basename = m_filename_input_box->text();
				LevelSerializer::load( m_parent.m_document );
				m_state = State::Done;
			}
			else if( m_cancel->was_clicked() ){
				m_state = State::Done;
			}
		}

	}

	void LevelEditor::LoadMode::handle_keyboard( Keyboard& keyboard ){
		if( keyboard.is_pressed( Key::Tab ) ){
			if( keyboard.is_pressed( Key::Shift ) ){
				--m_focus_index;
			}
			else{
				++m_focus_index;
			}
			wrap_focus();
			m_filename_input_box->set_focused( m_focus_index == 1 );
		}

		if( keyboard.is_pressed( Key::Enter ) ){
			if( m_focus_index == 0 ){
				if( m_dropdown->open() ){
					if( m_dropdown->selected_index() >= 0 ){
						m_filename_input_box->set_text(
							std::string{ m_dropdown->selected_item() }
						);
						m_dropdown->set_open( false );
					}
				}
			}
			else if( m_focus_index == 1 || m_focus_index == 2 ){
				if( !m_load->enabled() ) return;
				if( m_filename_input_box->text().empty() ) return;

				// Load the file
				m_parent.m_document.basename = m_filename_input_box->text();
				LevelSerializer::load( m_parent.m_document );
				m_state = State::Done;
			}
			else if( m_focus_index == 3 ){
				m_state = State::Done;
			}
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			m_state = State::Done;
		}
	}

	void LevelEditor::LoadMode::handle_listbox( Mouse const& mouse_ ){
		auto str = m_dropdown->selected_item();
		if( str.empty() ) return;

		m_filename_input_box->set_text( std::string{ str } );
	}

	void LevelEditor::LoadMode::wrap_focus() noexcept{
		if( m_focus_index < 0 ){
			m_focus_index = 3;
		}
		m_focus_index %= 4;
	}
}
