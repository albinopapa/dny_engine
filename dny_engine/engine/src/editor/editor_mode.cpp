#include "editor/editor_mode.hpp"
#include "editor/save_mode.hpp"
#include "editor/load_mode.hpp"
#include "editor/save_before_exit_mode.hpp"
#include "editor/resize_mode.hpp"
#include "editor/texture_select_mode.hpp"
#include "editor/tile_palette_mode.hpp"


namespace dny{
	LevelEditor::EditorMode::EditorMode( LevelEditor& parent, Rect<std::int32_t> workspace )
		:m_parent( parent ), m_workspace( workspace ){}

	void LevelEditor::EditorMode::update( Mouse const& mouse, Keyboard& keyboard ){
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::EditorMode::render( renderer2d& renderer, Font const& font ) const{}

	void LevelEditor::EditorMode::handle_mouse( Mouse const& mouse ){
		// Editor area: paint tiles and camera control
		if( contains( m_workspace, m_parent.m_mouse_position ) ){
			// First, handle zoom via mouse wheel (consumes wheel events)
			handle_mouse_wheel( mouse );

			// Middle-mouse panning
			if( mouse.is_pressed( MouseButton::Middle ) ){
				const auto mouse_delta = vector2{
					static_cast< float >( mouse.delta().x ),
					static_cast< float >( mouse.delta().y )
				};
				// Move opposite so dragging feels natural
				m_parent.m_camera.pan( mouse_delta );
			}
			else if( mouse.is_pressed( MouseButton::Left ) ){
				place_tile( mouse );
			}
		}
	}

	void LevelEditor::EditorMode::handle_keyboard( Keyboard& keyboard ){
		const auto workspace_center = m_workspace.center();
		const auto dialog_rect = Rect<std::int32_t>{
			workspace_center.x - 200,
			workspace_center.y - 100,
			workspace_center.x + 200,
			workspace_center.y + 100
		};

		if( keyboard.is_pressed( Key::F1 ) ){
			m_parent.transition_mode( std::make_unique<SaveMode>( m_parent, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::F2 ) ){
			m_parent.transition_mode( std::make_unique<LoadMode>( m_parent, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::F3 ) ){
			m_parent.transition_mode( std::make_unique<ResizeMode>( m_parent, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::F4 ) ){
			m_parent.transition_mode(
				std::make_unique<TextureSelectMode>( m_parent, m_parent.m_active_tile_index, dialog_rect )
			);
		}
		else if( keyboard.is_pressed( Key::T ) ){
			// Open tile palette
			m_parent.transition_mode( std::make_unique<TilePaletteMode>( m_parent, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<SaveBeforeExitMode>( m_parent, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::C ) ){
			m_parent.m_camera.position = vector3<float>{};
			m_parent.m_camera.ortho_scale = 1.f;
		}
	}

	void LevelEditor::EditorMode::handle_mouse_wheel( Mouse const& mouse ){
		if( auto wd = mouse.wheel_delta(); wd != 0 ){
			const auto mouse_pos = vector2<float>{
				static_cast< float >( mouse.position().x ),
				static_cast< float >( mouse.position().y )
			};
			if( wd > 0 ){
				m_parent.m_camera.zoom_in( mouse_pos );
			}
			else{
				m_parent.m_camera.zoom_out( mouse_pos );
			}
		}

		m_parent.clamp_camera();
	}

	void LevelEditor::EditorMode::place_tile( Mouse const& mouse ){
		const auto tile_index = m_parent.m_tilemap_view.screen_to_tile_index( 
			mouse.position(), m_parent.m_camera 
		);
		if( !tile_index.has_value() )
			return;

		// Create tile with active tile index
		Tile tile;
		tile.definition_id = m_parent.m_active_tile_index;
		const auto size = m_parent.m_document.tilemap.size();

		if( ( *tile_index ).x < 0 || ( *tile_index ).y < 0 ||
			( *tile_index ).x >= size.width ||
			( *tile_index ).y >= size.height ){
			return;
		}

		m_parent.m_document.tilemap.get_tile( *tile_index ) = tile;
	}

}
