#include "editor/level_editor.hpp"
#include "editor/editor_mode.hpp"
#include "editor/texture_select_mode.hpp"

#include "graphics/image_loader.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <format>
#include <fstream>
#include <string>
#include <filesystem>

namespace dny
{
	// ---------- LevelEditor ----------
	LevelEditor::LevelEditor( Rect<std::int32_t> const& viewport, LevelDocument& document_ )
		:
		m_tilemap_view( m_document.tilemap, viewport ),
		m_document( document_ ),
		m_viewport( viewport ) {
		m_mode = std::make_unique<EditorMode>( *this, viewport );
		load_tileset_sprites();
	}

	void LevelEditor::update( Input& input_, float dt ){
		m_mouse_position = input_.mouse().position();
		if( m_next_mode ){
			m_mode = std::move( m_next_mode );
		}

		m_mode->update( input_.mouse(), input_.keyboard() );
	}

	void LevelEditor::render( renderer2d& renderer_, Font const& font_ ) const{
		// TODO: Layout will be handled by EditorView later, 
		// but for now we can just render the tilemap and mode UI directly
		// m_layout.render();
		m_tilemap_view.render( renderer_, m_camera, m_textures );
		m_mode->render(renderer_, font_);

		const auto pos_string = std::format( "X: {}, Y: {}", m_mouse_position.x, m_mouse_position.y );
		const auto char_height = font_.char_height();
		const auto str_pos = 
			m_viewport.bottom_left() + vector2{ 5, -char_height  };
		renderer_.draw_text( 
			pos_string, 
			str_pos, 
			font_, 
			to_color32( Colors::yellow )
		);
	}

	void LevelEditor::clamp_camera() noexcept{
		const auto size = m_document.tilemap.size();
		const float world_width  = static_cast< float >( size.width  * Tile::size ) - m_camera.viewport.width();
		const float world_height = static_cast< float >( size.height * Tile::size ) - m_camera.viewport.height();

		m_camera.position.x = std::clamp( m_camera.position.x, 0.0f, world_width );
		m_camera.position.y = std::clamp( m_camera.position.y, 0.0f, world_height );
	}

	void LevelEditor::resize_tilemap( dims2<std::int32_t> const& new_size ){
		m_document.tilemap.resize( new_size );
	}

	void LevelEditor::transition_mode( std::unique_ptr<IMode> next_mode ){
		m_next_mode = std::move( next_mode );
	}

	void LevelEditor::load_tileset_sprites(){
		namespace fs = std::filesystem;

		// TODO: relative path is baked into the global tile definitions for now, 
		// but eventually this should come from the LevelDocument
		const auto images_dir = fs::current_path();

		if( !fs::exists( images_dir ) || !fs::is_directory( images_dir ) ){
			// No images folder, nothing to load
			return;
		}

		// TODO: Implement sprite loading for tileset
		// Build a map: filename_without_extension → full path
		// Loop through tile definitions and load corresponding sprites
		// We'll use the global g_tile_defs for now, but eventually this should come from the LevelDocument
		for( auto& tile_def : g_tile_defs ){
			if(tile_def.texture_name.empty() ){
				continue; // No texture for this tile
			}
			
			m_textures[ std::string{ tile_def.name } ] = dny::load_surface_from_file<dny::ColorF>( 
				images_dir / tile_def.texture_name 
			);
		}
	}

}
