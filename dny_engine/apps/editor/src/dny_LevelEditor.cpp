
#include "../include/dny_LevelEditor.hpp"
#include "graphics/image_loader.hpp"
#include "../include/dny_EditorMode.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <format>
#include <fstream>
#include <string>
#include <filesystem>

// ---------- Internal helpers ----------
namespace{

	// Same coloring logic you had before, now shared.
	//dny::ColorF choose_liquid_tile_color( dny::Tile const& tile ){
	//	if( tile.name() == "water" )
	//		return dny::ColorF{ 0.1f, 0.4f, 1.0f, 0.7f };
	//	else if( tile.name() == "lava" )
	//		return dny::ColorF{ 1.0f, 0.3f, 0.0f, 0.7f };
	//	else
	//		return dny::ColorF{};
	//}

} // anonymous namespace

namespace dny
{
	// ---------- TileMapView ----------
	TileMapView::TileMapView( LevelEditor& editor, Rect<std::int32_t> const& area ) noexcept
		: m_editor{ &editor }
		, m_area{ area }{
		m_editor->m_camera.viewport = Rect<float>{ 
			static_cast<float>(area.left), 
			static_cast<float>(area.top), 
			static_cast<float>(area.right), 
			static_cast<float>(area.bottom) 
		};
	}

	void TileMapView::render( EditorCamera const& cam ) const{
		auto clipper = Graphics::set_render_region( Rect<float>{ m_area } );

		const auto size = dims2<std::int32_t>{ m_editor->m_tilemap.size() };
		for( std::int32_t y = 0; y < size.height; ++y ){
			for( std::int32_t x = 0; x < size.width; ++x ){
				const auto world_pos = vector2<float>{
					static_cast< float >( x * m_tile_size ),
					static_cast< float >( y * m_tile_size )
				};

				const auto screen_pos =
					( ( world_pos - cam.position ) * cam.zoom ) + vector2<float>{ m_area.top_left() };

				const auto tile_rect = Rect<float>{
					screen_pos.x,
					screen_pos.y,
					screen_pos.x + static_cast< float >( m_tile_size ) * cam.zoom,
					screen_pos.y + static_cast< float >( m_tile_size ) * cam.zoom
				};

				if( !m_area.intersects( Rect<std::int32_t>{ tile_rect } ) ){
					continue;
				}

				const auto tile_index = vector2<std::int32_t>{ x, y };
				const auto& tile = m_editor->m_tilemap.get_tile( tile_index );

				//switch( tile.category() ){
				//	// Don't draw anything for empty tiles
				//	case TileCategory::Empty: continue;
				//	case TileCategory::Liquid:
				//	{
				//		// Draw a semi-transparent rectangle for water/lava
				//		//const auto color = choose_liquid_tile_color( tile );
				//		//Graphics::fill_rectangle( tile_rect, color );
				//		break;
				//	}
				//	default:
				//	{
				//		// Draw the tile sprite scaled to the zoom level
				//		const auto old_xform = Graphics::instance().set_transform(
				//			D2D1::Matrix3x2F::Scale( cam.zoom, cam.zoom, D2D1::Point2F( screen_pos.x, screen_pos.y ) )
				//		);
				//		Graphics::instance().draw_sprite(
				//			tile_rect.top_left(),
				//			m_editor->m_tileset_sprites[ tile.get_id() ]
				//		);
				//		Graphics::instance().set_transform( old_xform );
				//		break;
				//	}
				//}
			}
		}
	}

	std::optional<vector2<std::int32_t>> TileMapView::tile_at( vector2<std::int32_t> const& screen_pos, EditorCamera const& cam ) const noexcept{
		if( !contains( m_area, screen_pos ) )
			return std::nullopt;

		auto world = cam.screen_to_world( screen_pos, m_area );

		vector2<std::int32_t> tile_index{
			int( world.x / m_tile_size ),
			int( world.y / m_tile_size )
		};

		const auto size = m_editor->m_tilemap.size();

		if( tile_index.x < 0 || tile_index.y < 0 ||
			tile_index.x >= static_cast< std::int32_t >( size.width ) ||
			tile_index.y >= static_cast< std::int32_t >( size.height ) ){
			return std::nullopt;
		}

		return tile_index;
	}

	void TileMapView::set_tile( vector2<std::int32_t> const& tile_index, Tile const& tile ){
		const auto size = m_editor->m_tilemap.size();

		if( tile_index.x < 0 || tile_index.y < 0 ||
			tile_index.x >= static_cast< std::int32_t >( size.width ) ||
			tile_index.y >= static_cast< std::int32_t >( size.height ) ){
			return;
		}

		m_editor->m_tilemap.get_tile( tile_index ) = tile;
	}

	// ---------- TilePalette ----------

	TilePalette::TilePalette( LevelEditor& editor, Rect<std::int32_t> const& area ) noexcept
		: m_area{ area },
		m_editor{ &editor }{}

	void TilePalette::render() const{
		// Active tile preview
		{
			auto preview_rect = m_editor->m_layout.cell_rect( 0, 1 );
			auto clipper = Graphics::set_render_region( Rect<float>{ preview_rect } );

			const auto& active = m_tileset[ m_active_index ];

			//if( active.category() == TileCategory::Empty ){
			//	Graphics::fill_rectangle( preview_rect, Colors::Transparent );
			//}
			//else if( active.category() == TileCategory::Liquid ){
			//	Graphics::fill_rectangle( preview_rect, choose_liquid_tile_color( active ) );
			//}
			//else{
			//	const auto xform = D2D1::Matrix3x2F::Scale(
			//		D2D1::SizeF( 2.f, 2.f ),
			//		d2d_cast< D2D1_POINT_2F >::convert( preview_rect.top_left() )
			//	);
			//	const auto old_xform = graphics.set_transform( xform );
			//	Graphics::draw_sprite(
			//		preview_rect.top_left(),
			//		m_editor->m_tileset_sprites[ active.get_id() ]
			//	);
			//	Graphics::set_transform( old_xform );
			//}
		}

		// Palette list below preview
		auto clipper = Graphics::set_render_region( Rect<float>{ m_area } );
		const auto cell_height = m_tile_spacing + m_tile_size;
		const auto x = m_area.left;
		auto y =
			static_cast< std::int32_t >( m_editor->m_layout.cell_rect( 0, 1 ).height() );

		const auto padding = ( m_area.width() - m_tile_size ) / 2;

		for( std::size_t i = 0; i < m_tileset.size(); ++i ){
			const Rect<float> tile_rect{
				static_cast< float >( padding + x ),
				static_cast< float >( padding + y ),
				static_cast< float >( padding + x + m_tile_size ),
				static_cast< float >( padding + y + m_tile_size )
			};

			const auto current_tile = m_tileset[ i ];
			//const auto color = choose_liquid_tile_color( current_tile );

			const auto& sprites = m_editor->m_tileset_sprites;			
			//if( const auto sprite = sprites.find( current_tile.get_id() ); sprite != sprites.end() ) {
			//	Graphics::draw_sprite(
			//		tile_rect.top_left(),
			//		sprite->second
			//	);
			//}
			//else{
			//	Graphics::fill_rectangle( tile_rect, color );
			//}

			y += cell_height;
		}
	}

	std::optional<std::size_t> TilePalette::hit_test( vector2<std::int32_t> const& screen_pos ) const noexcept{
		if( !m_editor->m_layout.cell_rect( 1, 1 ).contains( vector2<float>{ screen_pos } ) )
			return std::nullopt;

		// Same layout as render(): tiles start below preview
		const int local_x = screen_pos.x - m_area.left;
		const int local_y = screen_pos.y - m_area.top;

		const int start_y = 0;
		if( local_y < start_y )
			return std::nullopt;

		const int index = ( local_y - start_y ) / ( m_tile_size + m_tile_spacing );

		if( index < 0 || index >= static_cast< int >( m_tileset.size() ) )
			return std::nullopt;

		return static_cast< std::size_t >( index );
	}

	void TilePalette::set_active_index( std::size_t index ) noexcept{
		if( index < m_tileset.size() ){
			m_active_index = index;
		}
	}

	Tile const& TilePalette::active_tile() const noexcept{
		return m_tileset[ m_active_index ];
	}


	// ---------- LevelEditor ----------
	LevelEditor::LevelEditor()
		: m_layout{ "editor_panel","", Rect<float>{ 0.f, 0.f, 640.f, 360.f }, ""}{

		// [ tool bar                 | preview    ]
		// [ editor area              | palette    ]
		// [ status bar               | unassigned ]

		m_view = TileMapView{ *this, Rect<std::int32_t>{ m_layout.cell_rect( 1, 0 ) } };
		m_palette = TilePalette{ *this, Rect<std::int32_t>{ m_layout.cell_rect( 1, 1 ) } };

		m_mode = std::make_unique<EditorMode>( *this );
		load_tileset_sprites();
	}

	void LevelEditor::update( float dt ){
		if( m_next_mode ){
			m_mode = std::move( m_next_mode );
			Mouse::update();
		}
		m_mode->update();
	}

	void LevelEditor::render() const{
		m_layout.render();
		m_view.render( m_camera );
		m_palette.render();
		m_mode->render();

		const auto mouse_pos = Mouse::position();
		const auto pos_string =
			std::format( "X: {}, Y: {}", mouse_pos.x, mouse_pos.y );

		const auto str_pos = m_layout.cell_rect( 2, 0 ).top_left();
		Graphics::draw_text( str_pos, pos_string, Colors::Yellow );
	}

	void LevelEditor::clamp_camera() noexcept{
		const auto size = m_tilemap.size();
		const float world_width  = static_cast< float >( size.width  * Tile::size ) - m_camera.viewport.width();
		const float world_height = static_cast< float >( size.height * Tile::size ) - m_camera.viewport.height();

		m_camera.position.x = std::clamp( m_camera.position.x, 0.0f, world_width );
		m_camera.position.y = std::clamp( m_camera.position.y, 0.0f, world_height );
	}

	void LevelEditor::resize_tilemap( dims2<std::int32_t> const& new_size ){
		m_tilemap.resize( new_size );
	}

	void LevelEditor::transition_mode( std::unique_ptr<IMode> next_mode ){
		m_next_mode = std::move( next_mode );
	}

	void LevelEditor::load_tileset_sprites(){
		namespace fs = std::filesystem;

		auto create_image_lookup = []( fs::path const& images_dir ){
			std::unordered_map<std::string, fs::path> image_lookup;

			for( const auto& entry : fs::directory_iterator( images_dir ) ){
				if( entry.is_regular_file() ){
					auto ext = entry.path().extension().string();
					if( ext == ".png" || ext == ".PNG" ){
						std::string key = entry.path().stem().string();  // filename without .png
						image_lookup[ key ] = entry.path();
					}
				}
			}

			return image_lookup;
		};

		auto fill_sprite_map = [this]( std::unordered_map<std::string, fs::path> const& image_lookup ){
			std::unordered_map<std::int32_t, Sprite> tileset_sprites;

			for( std::size_t i = 0; i < m_palette.tile_count(); ++i ){
				auto type = m_palette.tile_at( i ).id;

				//// Skip types you intentionally draw without textures
				//if( type == Tile::Type::Empty ||
				//	type == Tile::Type::Water ||
				//	type == Tile::Type::Lava ){
				//	continue;
				//}

				// Expected filename: lowercase enum name WITHOUT extension
				// Example: Solid → "solid.png"
				//std::string type_name = []( Tile::Type type ){
				//
				//	switch( type ){
				//		case Tile::Type::Solid:          return "dirt";
				//		case Tile::Type::Platform:       return "platform";
				//		case Tile::Type::Door:           return "door";
				//		case Tile::Type::PlayerSpawn:    return "player";
				//		case Tile::Type::HopperSpawn:    return "hopper";
				//		case Tile::Type::FireBallSpawn:  return "fireball";
				//		default: return "";
				//	}
				//}( type );
				// Look up corresponding PNG file
				//auto it = image_lookup.find( type_name );
				//if( it != image_lookup.end() ){
				//	// Load sprite
				//	tileset_sprites[ type ] =
				//		Graphics::instance().make_sprite_from_file( it->second.string() );
				//}
			}

			return tileset_sprites;
		};

		// Path to Images directory (same working directory convention you already use)
		const auto images_dir = fs::current_path() / "Images";

		if( !fs::exists( images_dir ) || !fs::is_directory( images_dir ) ){
			// No images folder, nothing to load
			return;
		}

		// Build a map: filename_without_extension → full path
		const auto image_lookup = create_image_lookup( images_dir );

		// Loop through every tile type in the palette
		m_tileset_sprites = fill_sprite_map( image_lookup );

	}

	void LevelEditor::place_tile(){
		const auto mouse_pos = vector2<float>{ Mouse::position() };
		const auto tile_index = m_view.tile_at( vector2<std::int32_t>{ mouse_pos }, m_camera );
		if( !tile_index.has_value() )
			return;

		//if( m_tilemap.get_tile( *tile_index ).type() == m_palette.active_tile().type() )
		//	return;
		//
		//if( m_palette.active_tile().type() != Tile::Type::PlayerSpawn ){
		//	if( m_tilemap.get_tile( *tile_index ).type() == Tile::Type::PlayerSpawn ){
		//		m_player_spawn_used = false;
		//	}
		//	m_view.set_tile( *tile_index, m_palette.active_tile() );
		//}
		//else{
		//	// Limit player spawn to single cell
		//	if( !m_player_spawn_used ){
		//		m_view.set_tile( *tile_index, m_palette.active_tile() );
		//		m_player_spawn_used = true;
		//	}
		//}
	}
}
