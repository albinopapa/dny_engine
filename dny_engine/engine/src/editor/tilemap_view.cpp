#include "editor/tilemap_view.hpp"
#include "editor/level_editor.hpp"
#include "editor/level_document.hpp"

#include "physics/physics.hpp"

#include <algorithm>

namespace dny{
	TileMapView::TileMapView( LevelDocument const& document_ ) noexcept
		: m_document{ document_ }{}

	void TileMapView::render( renderer2d& renderer, EditorCamera const& cam, std::unordered_map<std::string, surface<ColorF>> const& textures_ ) const{
		if( m_document.tilemap.size().width == 0 || m_document.tilemap.size().height == 0 ){
			return;
		}

		const auto area_top_left = vector2<float>{
			static_cast< float >( m_area.left ),
			static_cast< float >( m_area.top )
		};
		const auto area_size = vector2<float>{
			static_cast< float >( m_area.width() ),
			static_cast< float >( m_area.height() )
		};

		const auto world_size = area_size * cam.ortho_scale;
		const auto half_world_size = world_size / 2.f;
		const auto cam_position = vector2<float>{
			cam.position.x,
			cam.position.y
		};
		const auto world_min = cam_position - half_world_size;
		const auto world_max = cam_position + half_world_size;

		auto min_tile_x = static_cast< std::int32_t >( std::floor( world_min.x / m_tile_size ) );
		auto max_tile_x = static_cast< std::int32_t >( std::floor( ( world_max.x - epsilon ) / m_tile_size ) );
		auto min_tile_y = static_cast< std::int32_t >( std::floor( world_min.y / m_tile_size ) );
		auto max_tile_y = static_cast< std::int32_t >( std::floor( ( world_max.y - epsilon ) / m_tile_size ) );

		min_tile_x = std::clamp( min_tile_x, 0, m_document.tilemap.size().width - 1 );
		max_tile_x = std::clamp( max_tile_x, 0, m_document.tilemap.size().width - 1 );
		min_tile_y = std::clamp( min_tile_y, 0, m_document.tilemap.size().height - 1 );
		max_tile_y = std::clamp( max_tile_y, 0, m_document.tilemap.size().height - 1 );

		const auto size = m_document.tilemap.size();
		const float screen_tile_size = ( float )m_tile_size / cam.ortho_scale;

		for( std::int32_t y = min_tile_y; y <= max_tile_y; ++y ){
			for( std::int32_t x = min_tile_x; x <= max_tile_x; ++x ){
				const auto world_pos = vector2<float>{
					static_cast< float >( x * m_tile_size ),
					static_cast< float >( y * m_tile_size )
				};

				const auto half_area = area_size / 2.f;

				const auto screen_pos =
					area_top_left
					+ half_area
					+ ( world_pos - cam_position ) / cam.ortho_scale;


				const auto tile_rect = Rect<std::int32_t>{
					static_cast< std::int32_t >( screen_pos.x ),
					static_cast< std::int32_t >( screen_pos.y ),
					static_cast< std::int32_t >( screen_pos.x + screen_tile_size ),
					static_cast< std::int32_t >( screen_pos.y + screen_tile_size )
				};

				if( !intersects( m_area, tile_rect ) ){
					continue;
				}

				const auto tile_index = vector2<std::int32_t>{ x, y };
				const auto& tile = m_document.tilemap.get_tile( tile_index );
				const auto& tile_def = m_document.tile_defs[ tile.definition_id ];

				if( !tile_def.texture_name.empty() ){
					renderer.draw_sprite( tile_rect, textures_.at( std::string{ tile_def.texture_name } ) );
				}
				else if( tile_def.category != TileCategory::Empty ){
					if( tile_def.category == TileCategory::Liquid && tile_def.name == "Water" ){
						renderer.draw_rect( tile_rect, Colors::cyan );
					}
					else if( tile_def.category == TileCategory::Liquid && tile_def.name == "Lava" ){
						renderer.draw_rect( tile_rect, Colors::red );
					}
					else{
						renderer.draw_rect( tile_rect, Colors::magenta );
					}
				}
				else{
					renderer.fill_rect( tile_rect, Colors::gray );
					renderer.draw_rect( tile_rect, Colors::black );
				}
			}
		}
	}

	std::optional<vector2<std::int32_t>> TileMapView::screen_to_tile_index( vector2<std::int32_t> const& screen_pos, EditorCamera const& cam ) const noexcept{
		if( !contains( m_area, screen_pos ) )
			return std::nullopt;

		// Convert screen to world coordinates
		const auto local_x = screen_pos.x - m_area.left;
		const auto local_y = screen_pos.y - m_area.top;

		const auto world_x = cam.position.x + ( local_x * cam.ortho_scale );
		const auto world_y = cam.position.y + ( local_y * cam.ortho_scale );

		// Convert world coordinates to tile index
		vector2<std::int32_t> tile_index{
			static_cast< std::int32_t >( world_x / m_tile_size ),
			static_cast< std::int32_t >( world_y / m_tile_size )
		};

		const auto size = m_document.tilemap.size();

		if( tile_index.x < 0 || tile_index.y < 0 ||
			tile_index.x >= size.width ||
			tile_index.y >= size.height ){
			return std::nullopt;
		}

		return tile_index;
	}
}