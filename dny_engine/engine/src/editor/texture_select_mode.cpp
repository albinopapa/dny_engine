#include "editor/texture_select_mode.hpp"
#include "editor/level_editor.hpp"

#include "input/input.hpp"

#include <filesystem>
#include <memory>

namespace dny
{
	LevelEditor::TextureSelectMode::TextureSelectMode( LevelEditor& editor, std::size_t active_idx, Rect<std::int32_t> const& dialog_rect )
		: 
		basic_mode{ "texture_select_mode", "Select Texture", dialog_rect },
		m_parent{ editor },
		m_active_index( active_idx ){
		m_list_box = std::make_shared<ui::ListBox>( "textures", dialog_rect.top_left() + vector2{ 0, 0 }, dims2<std::int32_t>{ 400, 400 } );
		m_accept = std::make_shared<ui::Button>( "accept", "Accept", dialog_rect.top_left() + vector2{ 50, -50 }, dims2<std::int32_t>{ 75, 30 } );
		m_cancel = std::make_shared<ui::Button>( "cancel", "Cancel", dialog_rect.top_left() + vector2{ 200, -50 }, dims2<std::int32_t>{ 75, 30 } );
		
		m_panel.add_child( m_accept );
		m_panel.add_child( m_cancel );
		m_panel.add_child( m_list_box );

		for(auto const& [name, _] : m_parent.m_textures ){
			m_list_box->add_item( name );
		}

		if(!m_parent.m_selected_texture_name.empty() ){
			m_selected = m_parent.m_selected_texture_name;
			m_list_box->set_selected_item( m_selected );
		}

		m_accept->set_enabled( !m_selected.empty() );
	}

	void LevelEditor::TextureSelectMode::update( Mouse const& mouse, Keyboard& keyboard ){
		m_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::TextureSelectMode::render( renderer2d& renderer ) const{
		m_panel.draw( renderer, m_parent.m_font );
	}

	void LevelEditor::TextureSelectMode::handle_mouse( Mouse const& mouse ){
		if( m_list_box->contains( mouse.position() ) ){
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_selected = m_list_box->selected_item();
			}
		}
		if( m_accept->was_clicked() ){
			m_parent.m_selected_texture_name = m_selected;
			m_state = State::Done;
		}
		if( m_cancel->was_clicked() ){
			m_state = State::Done;
		}
	}

	void LevelEditor::TextureSelectMode::handle_keyboard( Keyboard& keyboard ){
		if( keyboard.is_pressed( Key::Escape ) && !m_selected.empty() ){
			m_state = State::Done;
		}
	}
}
