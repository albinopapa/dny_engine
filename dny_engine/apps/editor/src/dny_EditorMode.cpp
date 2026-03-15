#include "../include/dny_EditorMode.hpp"
#include "../include/dny_SaveMode.hpp"
#include "../include/dny_LoadMode.hpp"
#include "../include/dny_SaveBeforeExitMode.hpp"
#include "../include/dny_ResizeMode.hpp"
#include "../include/dny_TextureSelectMode.hpp"


namespace dny
{
	LevelEditor::EditorMode::EditorMode( LevelEditor& parent )
		:m_parent( &parent ){}

	void LevelEditor::EditorMode::render() const{}

	void LevelEditor::EditorMode::handle_mouse( Input const& input_ ){
		const auto mouse_ipos = input_.mouse().position();
		const auto mouse_pos = vector2<float>{
			static_cast< float >( mouse_ipos.x ),
			static_cast< float >( mouse_ipos.y )
		};

		// Tileset area: select active tile
		//const auto tileset_area = m_parent->m_layout.cell_rect( 1, 1 );
		if( m_parent->m_layout.cell_rect( 1, 1 ).contains( mouse_pos ) ){
			if( !input_.mouse().is_pressed( MouseButton::Left ) )
				return;

			const auto idx = m_parent->m_palette.hit_test( mouse_ipos );
			if( !idx.has_value() )
				return;

			m_parent->m_palette.set_active_index( *idx );
		}
		// Editor area: paint tiles
		else if( m_parent->m_layout.cell_rect( 1, 0 ).contains( mouse_pos ) ){
			// First, handle zoom via mouse wheel (consumes wheel events)
			handle_mouse_wheel( input_ );

			// Middle-mouse panning
			if( input_.mouse().is_pressed( MouseButton::Middle ) ){
				// Move opposite so dragging feels natural
				m_parent->m_camera.pan( vector2{ input_.mouse().delta() } );
			}
			else if( input_.mouse().is_pressed( MouseButton::Left ) ){
				m_parent->place_tile();
			}
		}
	}

	void LevelEditor::EditorMode::handle_keyboard( Input const& input_ ){
		if( input_.keyboard().is_pressed( Key::F1 ) ){
			m_parent->transition_mode( std::make_unique<SaveMode>( *m_parent ) );
		}
		else if( input_.keyboard().is_pressed( Key::F2 ) ){
			m_parent->transition_mode( std::make_unique<LoadMode>( *m_parent ) );
		}
		else if( input_.keyboard().is_pressed( Key::F3 ) ){
			m_parent->transition_mode( std::make_unique<ResizeMode>( *m_parent ) );
		}
		else if( input_.keyboard().is_pressed( Key::F4 ) ){
			m_parent->transition_mode(
				std::make_unique<TextureSelectMode>( *m_parent, m_parent->m_palette.get_active_index() )
			);
		}
		else if( input_.keyboard().is_pressed( Key::Escape ) ){
			m_parent->transition_mode( std::make_unique<SaveBeforeExitMode>( *m_parent ) );
		}
		else if( input_.keyboard().is_pressed( Key::C ) ){
			m_parent->m_camera.position = vector2<float>{};
			m_parent->m_camera.zoom = 1.f;
		}
	}

	void LevelEditor::EditorMode::handle_mouse_wheel( Input const& input_ ){
		auto const& mouse = input_.mouse();
		if(auto wd = mouse.wheel_delta(); wd != 0 ){
			if( wd > 0 ){
				m_parent->m_camera.zoom_in();
			}
			else{
				m_parent->m_camera.zoom_out();
			}
		}

		m_parent->clamp_camera();
	}
}
