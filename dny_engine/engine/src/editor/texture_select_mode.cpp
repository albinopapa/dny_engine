#include "editor/texture_select_mode.hpp"
#include "editor/level_editor.hpp"
#include "editor/editor_mode.hpp"

#include "input/input.hpp"

#include <filesystem>
#include <memory>

namespace dny
{
	LevelEditor::TextureSelectMode::TextureSelectMode( LevelEditor& editor, std::size_t active_idx, Rect<std::int32_t> const& dialog_rect )
		: m_parent{ editor }
		, m_active_index( active_idx )
		, m_dialog_panel( "", "Texture Select", dialog_rect.top_left(), dialog_rect.size() )
	{
		m_list_box = std::make_shared<ui::ListBox>( "textures", dialog_rect.top_left() + vector2{ 0, 0 }, dims2<std::int32_t>{ 400, 400 } );
		m_accept = std::make_shared<ui::Button>( "accept", "Accept", dialog_rect.top_left() + vector2{ 50, -50 }, dims2<std::int32_t>{ 75, 30 } );
		m_cancel = std::make_shared<ui::Button>( "cancel", "Cancel", dialog_rect.top_left() + vector2{ 200, -50 }, dims2<std::int32_t>{ 75, 30 } );
		m_dialog_panel.add_child( m_accept );
		m_dialog_panel.add_child( m_cancel );
		m_dialog_panel.add_child( m_list_box );

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
		m_dialog_panel.update( mouse, keyboard );
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}
	void LevelEditor::TextureSelectMode::render( renderer2d& renderer, dny::Font const& font ) const{
		m_dialog_panel.draw( renderer, font );
	}

	void LevelEditor::TextureSelectMode::handle_mouse( Mouse const& mouse ){
		if( m_list_box->contains( mouse.position() ) ){
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_selected = m_list_box->selected_item();
			}
		}
		if( m_accept->contains( mouse.position() ) ){
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_parent.m_selected_texture_name = m_selected;
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
		}
		if( m_cancel->contains( mouse.position() ) ){
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
		}
	}

	void LevelEditor::TextureSelectMode::handle_keyboard( Keyboard& keyboard ){
		if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
	}
}
