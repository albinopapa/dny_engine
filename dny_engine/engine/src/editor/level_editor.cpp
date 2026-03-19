#include "editor/level_editor.hpp"
#include "graphics/image_loader.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <memory>
#include <string>

#include "editor/save_mode.hpp"
#include "editor/load_mode.hpp"
#include "editor/save_before_exit_mode.hpp"
#include "editor/resize_mode.hpp"
#include "editor/texture_select_mode.hpp"
#include "editor/tile_palette_mode.hpp"
#include "editor/file_menu_mode.hpp"

namespace dny{
	// ---------- LevelEditor ----------
	LevelEditor::LevelEditor( Rect<std::int32_t> const& viewport, LevelDocument& document_, Font const& font_ )
		:
		m_document( document_ ),
		m_screen_rect( viewport ),
		m_tilemap_view( document_ ),
		m_font( font_ ),
		m_layout{ "editor_layout", "", { 0, 0 }, { 0, 0 } }{
		init_buttons();
		m_layout.add_child( m_file_button );
		m_layout.add_child( m_resize_button );
		m_layout.add_child( m_texture_button );
		m_layout.add_child( m_tile_palette_button );
		m_layout.add_child( m_tools_button );
		m_layout.add_child( m_exit_button );
		load_tileset_sprites();

		const auto view_start_x = static_cast< float >( m_layout.bounds().right );
		const auto cam_rect = Rect<float>{
			view_start_x, 0.f,
			static_cast< float >( viewport.width() ),
			static_cast< float >( viewport.height() )
		};
		m_camera = { .viewport = cam_rect };

		const auto int_view_start_x = m_layout.bounds().right;
		m_viewport = Rect<std::int32_t>{
			static_cast< std::int32_t >( int_view_start_x ),
			viewport.top,
			viewport.right,
			viewport.bottom
		};
		m_tilemap_view.set_area( m_viewport );
	}

	void LevelEditor::update( Input& input_, float dt ){
		++m_frame_count;
		m_time_accumulator += dt;
		if( m_time_accumulator >= 1.f ){
			m_fps = static_cast< float >( m_frame_count ) / m_time_accumulator;
			m_frame_count = 0;
			m_time_accumulator = 0.f;
		}

		m_mouse_position = input_.mouse().position();
		m_layout.update( input_.mouse(), input_.keyboard() );

		while( !m_mode_stack.empty() &&
			m_mode_stack.back()->state() == basic_mode::State::Done ){
			m_mode_stack.pop_back();
		}

		if( !m_mode_stack.empty() ){
			m_mode_stack.back()->update( input_.mouse(), input_.keyboard() );
			return;
		}

		handle_mouse( input_.mouse() );
		handle_keyboard( input_.keyboard() );
	}

	void LevelEditor::render( renderer2d& renderer_ ) const{
		m_tilemap_view.render( renderer_, m_camera, m_textures );
		m_layout.draw( renderer_, m_font );

		if( !m_mode_stack.empty() ){
			m_mode_stack.back()->render( renderer_ );
		}

		const auto pos_string = std::format( "X: {}, Y: {}", m_mouse_position.x, m_mouse_position.y );
		const auto string_pos = m_viewport.bottom_left() + vector2{ 5, -m_font.char_height() };
		renderer_.draw_text( pos_string, string_pos, m_font, to_color32( Colors::yellow ) );

		// FPS display
		const auto fps_string = std::format( "FPS: {:.2f}", m_fps );
		const auto offset = Font::measure_text( pos_string, m_font );
		const auto fps_pos = string_pos + vector2{ offset.width + 10, 0 };
		renderer_.draw_text( fps_string, fps_pos, m_font, to_color32( Colors::yellow ) );
	}

	void LevelEditor::handle_mouse( Mouse const& mouse ){
		const auto workspace_center = m_viewport.center();
		const auto dialog_rect = Rect<std::int32_t>{
			workspace_center.x - 200,
			workspace_center.y - 100,
			workspace_center.x + 200,
			workspace_center.y + 100
		};
		if( m_layout.contains( mouse.position() ) ){
			if( m_file_button->was_clicked() ){
				transition_mode( std::make_unique<LevelEditor::FileMenuMode>( *this, dialog_rect ) );
			}
			else if( m_resize_button->was_clicked() ){
				transition_mode( std::make_unique<LevelEditor::ResizeMode>( *this, dialog_rect ) );
			}
			else if( m_texture_button->was_clicked() ){
				transition_mode(
					std::make_unique<LevelEditor::TextureSelectMode>( *this, m_active_tile_index, dialog_rect )
				);
			}
			else if( m_tile_palette_button->was_clicked() ){
				transition_mode( std::make_unique<LevelEditor::TilePaletteMode>( *this, dialog_rect ) );
			}
			else if( m_exit_button->was_clicked() ){
				if( m_dirty ){
					// Prompt to save changes before exiting
					transition_mode( std::make_unique<LevelEditor::SaveBeforeExitMode>( *this, dialog_rect ) );
				}
				else{
					// No unsaved changes, exit immediately
					m_request = app_state_request::Menu;
				}
			}
			else if( m_tools_button->contains( mouse.position() ) ){
				// TODO: Transition to tools menu mode
			}
		}
		// Editor area: paint tiles and camera control
		else if( contains( m_viewport, mouse.position() ) ){
			// First, handle zoom via mouse wheel (consumes wheel events)
			handle_mouse_wheel( mouse );

			// Middle-mouse panning
			if( mouse.is_held( MouseButton::Middle ) ){
				const auto mouse_delta = vector2{
					static_cast< float >( mouse.delta().x ),
					static_cast< float >( mouse.delta().y )
				};
				if( mouse_delta.x == 0.f && mouse_delta.y == 0.f ){
					return; // No movement, skip
				}
				// Move opposite so dragging feels natural
				m_camera.pan( mouse_delta );
			}
			else if( mouse.is_pressed( MouseButton::Left ) ){
				place_tile( mouse );
			}
		}
	}

	void LevelEditor::handle_keyboard( Keyboard& keyboard ){
		const auto workspace_center = m_viewport.center();
		const auto dialog_rect = Rect<std::int32_t>{
			workspace_center.x - 200,
			workspace_center.y - 100,
			workspace_center.x + 200,
			workspace_center.y + 100
		};

		if( keyboard.is_pressed( Key::F1 ) ){
			transition_mode( std::make_unique<LevelEditor::SaveMode>( *this, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::F2 ) ){
			transition_mode( std::make_unique<LevelEditor::LoadMode>( *this, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::F3 ) ){
			transition_mode( std::make_unique<LevelEditor::ResizeMode>( *this, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::F4 ) ){
			transition_mode(
				std::make_unique<LevelEditor::TextureSelectMode>( *this, m_active_tile_index, dialog_rect )
			);
		}
		else if( keyboard.is_pressed( Key::T ) ){
			// Open tile palette
			transition_mode( std::make_unique<LevelEditor::TilePaletteMode>( *this, dialog_rect ) );
		}
		else if( keyboard.is_pressed( Key::Escape ) ){
			if( m_dirty ){
				// Prompt to save changes before exiting
				transition_mode( std::make_unique<LevelEditor::SaveBeforeExitMode>( *this, dialog_rect ) );
			}
			else{
				// No unsaved changes, exit immediately
				m_request = app_state_request::Menu;
			}
		}
		else if( keyboard.is_pressed( Key::C ) ){
			m_camera.position = vector3<float>{};
			m_camera.ortho_scale = 1.f;
		}
	}

	void LevelEditor::handle_mouse_wheel( Mouse const& mouse ){
		if( auto wd = mouse.wheel_delta(); wd != 0 ){
			const auto mouse_pos = vector2<float>{
				static_cast< float >( mouse.position().x ),
				static_cast< float >( mouse.position().y )
			};
			if( wd > 0 ){
				m_camera.zoom_in( mouse_pos );
			}
			else{
				m_camera.zoom_out( mouse_pos );
			}
		}

		clamp_camera();
	}

	void LevelEditor::clamp_camera() noexcept{
		const auto size = m_document.tilemap.size();

		if( size.width == 0 || size.height == 0 ){
			m_camera.position = { 0.f, 0.f, m_camera.position.z };
			return;
		}

		const float world_width =
			static_cast< float >( size.width * Tile::size );

		const float world_height =
			static_cast< float >( size.height * Tile::size );

		const float visible_width =
			m_camera.viewport.width() * m_camera.ortho_scale;

		const float visible_height =
			m_camera.viewport.height() * m_camera.ortho_scale;

		const float half_visible_w = visible_width * 0.5f;
		const float half_visible_h = visible_height * 0.5f;

		// If viewport larger than world → center
		if( visible_width >= world_width ||
			visible_height >= world_height ){
			m_camera.position.x = world_width * 0.5f;
			m_camera.position.y = world_height * 0.5f;
			return;
		}

		const float min_x = half_visible_w;
		const float max_x = world_width - half_visible_w;

		const float min_y = half_visible_h;
		const float max_y = world_height - half_visible_h;

		m_camera.position.x =
			std::clamp( m_camera.position.x, min_x, max_x );

		m_camera.position.y =
			std::clamp( m_camera.position.y, min_y, max_y );
	}

	void LevelEditor::resize_tilemap( dims2<std::int32_t> const& new_size ){
		m_document.tilemap.resize( new_size );
	}

	void LevelEditor::transition_mode( std::unique_ptr<basic_mode> next_mode ){
		m_mode_stack.push_back( std::move( next_mode ) );
	}

	void LevelEditor::load_tileset_sprites(){
		namespace fs = std::filesystem;

		// TODO: relative path is baked into the global tile definitions for now, 
		// but eventually this should come from the LevelDocument
		const auto images_dir = fs::current_path() / "assets/textures/test";

		if( !fs::exists( images_dir ) || !fs::is_directory( images_dir ) ){
			// No images folder, nothing to load
			return;
		}

		// If no level is loaded, fill document with global tile defs 
		// so we can at least see something in the palette and place tiles with textures
		// during testing. If a level is loaded, it should have its own tile defs 
		// that we will load textures for.

		if( m_document.basename.empty() ){
			m_document.tile_defs = std::vector<TileDef>( g_tile_defs.begin(), g_tile_defs.end() );
		}

		for( auto& tile_def : m_document.tile_defs ){
			if( tile_def.texture_name.empty() ){
				continue; // No texture for this tile
			}

			m_textures[ std::string{ tile_def.name } ] = dny::load_surface_from_file<dny::ColorF>(
				images_dir / tile_def.texture_name
			);
		}
	}

	void LevelEditor::place_tile( Mouse const& mouse ){
		const auto tile_index = m_tilemap_view.screen_to_tile_index(
			mouse.position(), m_camera
		);
		if( !tile_index.has_value() )
			return;

		const auto& index = *tile_index;
		// Create tile with active tile index
		Tile tile;
		tile.definition_id = m_active_definition;
		const auto size = m_document.tilemap.size();

		if( index.x < 0 || index.y < 0 ||
			index.x >= size.width ||
			index.y >= size.height ){
			return;
		}

		m_document.tilemap.get_tile( index ) = tile;
		m_dirty = true;
	}

	void LevelEditor::init_buttons(){
		const auto button_dims = dims2<std::int32_t>{ 80, 30 };
		m_layout.set_size( { button_dims.width + 3, m_screen_rect.height() } );

		auto offset = vector2<std::int32_t>{ 0, 0 };
		auto padding = 10;
		m_file_button = std::make_shared<ui::Button>( "file_button", "File", offset, button_dims );
		offset.y += button_dims.height + padding;
		m_resize_button = std::make_shared<ui::Button>( "resize_button", "Resize", offset, button_dims );
		offset.y += button_dims.height + padding;
		m_texture_button = std::make_shared<ui::Button>( "texture_button", "Texture", offset, button_dims );
		offset.y += button_dims.height + padding;
		m_tile_palette_button = std::make_shared<ui::Button>( "tile_palette_button", "Palette", offset, button_dims );
		offset.y += button_dims.height + padding;
		m_tools_button = std::make_shared<ui::Button>( "tools_button", "Tools", offset, button_dims );
		offset.y += button_dims.height + padding;
		m_exit_button = std::make_shared<ui::Button>( "exit_button", "Exit", offset, button_dims );
	}

}
