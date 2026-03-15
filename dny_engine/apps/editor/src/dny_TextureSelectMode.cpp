#include "TextureSelectMode.hpp"
#include "LevelEditor.hpp"
#include "EditorMode.hpp"

#include <filesystem>
#include <memory>

namespace dny
{
	LevelEditor::TextureSelectMode::TextureSelectMode( LevelEditor& editor, std::size_t active_idx )
		: m_parent{ &editor }
		, m_active_index( active_idx ) {
		const auto string_box =
			Graphics::calculate_rect( std::string{ "ABCDEFGHIJKLM" } );  // NOPQRSTUVWXYZ012345
		m_dropdown_box = UIDropDownBox{
			string_box + m_dialog_rect.top_left() + Vec2f{ 10.f, 10.f },
			""
		};

		auto rect = m_dropdown_box.border();
		m_list_box = UIListBox<std::string>{
			RectF{ rect.left, rect.bottom, rect.right, rect.bottom + string_box.height() },
			""
		};

		const auto button_bottom = rect.height() + 40.f;
		m_accept = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.bottom_left() + Vec2f{ 50.f, -button_bottom },
			" Accept "
		};
		m_cancel = UIButton{
			Graphics::calculate_rect( "00000000" )
			+ m_dialog_rect.bottom_left() + Vec2f{ 225.f, -button_bottom },
			" Cancel "
		};
	}

	void LevelEditor::TextureSelectMode::render() const{
		Graphics::instance().draw_rectangle( m_dialog_rect, Colors::White );
		m_dropdown_box.render();
		m_accept.render();
		m_cancel.render();
		if( m_dropdown_box.is_open() ){
			m_list_box.render();
		}
	}

	void LevelEditor::TextureSelectMode::handle_mouse(){
		auto& mouse = Mouse::instance();
		const auto mouse_pos = Vec2f{ mouse.position() };

		while( !mouse.is_event_queue_empty() ){
			const auto event = mouse.read_event();

			if( m_dropdown_box.is_hit( mouse_pos ) ){
				if( Input::is_button_release_event( event, Mouse::Button::Left ) ){
					m_dropdown_box.on_click();
					continue;
				}
			}

			if( m_dropdown_box.is_open() ){
				handle_listbox( event );
			}

			if( m_accept.is_enabled() ){
				if( m_accept.is_hit( mouse_pos ) ){
					m_accept.select();

					if( Input::is_button_release_event( event, Mouse::Button::Left ) ){
						// assign selected texture to active tile
						auto& tile = m_parent->m_palette.tile_at( m_active_index );
						tile.set_sprite_name( m_parent->m_current_filename );
					}
				}
				else{
					m_accept.unselect();
				}
			}
			
			if( m_cancel.is_hit( mouse_pos ) ){
				m_cancel.select();
			}
			else{
				m_cancel.unselect();
			}
			
		}
	}

	void LevelEditor::TextureSelectMode::handle_keyboard(){
		auto& keyboard = Keyboard::instance();
		while( !keyboard.is_event_queue_empty() ){
			const auto event = keyboard.read_event();
			if( Input::is_button_release_event( event, VK_ESCAPE ) ){
				m_parent->transition_mode( std::make_unique<EditorMode>( *m_parent ) );
			}
		}
	}

	void LevelEditor::TextureSelectMode::handle_listbox( Mouse::Event const& event ){
		const auto mouse_pos = Vec2f{ event.position() };
		if( !m_list_box.border().contains( mouse_pos ) ){
			return;
		}

		if( event.type() == Mouse::Event::Type::WheelUp ){
			m_list_box.scroll_up();
		}
		else if( event.type() == Mouse::Event::Type::WheelDown ){
			m_list_box.scroll_down();
		}

		const auto row = m_list_box.entry_index( mouse_pos );
		m_list_box.set_hovered( row );

		if( row.has_value() ){
			return;
		}

		if( !Input::is_button_release_event( event, Mouse::Button::Left ) ){
			return;
		}

		m_dropdown_box.set_string( m_list_box.select_entry( *row ) );
		m_dropdown_box.on_click();
		m_accept.enable();
	}
}
