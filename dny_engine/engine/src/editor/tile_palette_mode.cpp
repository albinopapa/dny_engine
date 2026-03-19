#include "editor/tile_palette_mode.hpp"

#include "editor/tile_map.hpp"
#include "graphics/colors.hpp"
#include "input/input.hpp"
#include "graphics/font.hpp"

namespace dny{
	LevelEditor::TilePaletteMode::TilePaletteMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect )
		: 
		basic_mode{ "tile_palette_mode", "Tile Palette", dialog_rect },
		m_parent{ parent } {
		const auto panel_rect = m_panel.bounds();

		// For resolution scaling later: ( 0.0375, 0.0222 ) 
		// are the ratios of scrollbar offset to viewport size in 
		// the original 640x360 layout
		// To convert to absolute pixels, we can multiply these ratios by the viewport dimensions.
		// Example: For a 1280x720 viewport, the offsets would be ( 0.0375 * 1280, 0.0222 * 720 ) = ( 48, 16 )
		const auto scrollbar_offset = vector2<std::int32_t>{ -24, 8 };

		// Size factor: ( 0.025, 0.953 ) are the ratios of scrollbar
		// size to viewport size in the original layout
		const auto scrollbar_size = dims2<std::int32_t>{ 16, panel_rect.height() - 16 };
		m_scrollbar = std::make_shared< ui::VScrollBar >(
			"tile_palette_scrollbar",
			panel_rect.top_right() + scrollbar_offset,
			scrollbar_size
		);
		m_scrollbar->set_page_size( m_visible_count );
		m_panel.add_child( m_scrollbar );
		sync_scrollbar();
	}

	void LevelEditor::TilePaletteMode::update( Mouse const& mouse, Keyboard& keyboard ){
		sync_scrollbar();
		m_panel.update( mouse, keyboard );
		if( m_scrollbar->value_changed() ){
			m_scroll_offset = static_cast< std::size_t >( m_scrollbar->value() );
		}
		handle_mouse( mouse );
		handle_keyboard( keyboard );
		sync_scrollbar();
	}

	void LevelEditor::TilePaletteMode::render( renderer2d& renderer_ ) const{
		m_panel.draw( renderer_, m_parent.m_font );

		const auto panel_rect = m_panel.bounds();
		const auto content_right = m_scrollbar->bounds().left - 8;
		const auto padding = ( ( content_right - panel_rect.left ) - m_tile_size ) / 2;
		const auto cell_height = m_tile_spacing + m_tile_size;
		auto y_offset = panel_rect.top + 40;

		for( std::size_t i = m_scroll_offset; i < m_tileset.size(); ++i ){
			if( y_offset + m_tile_size > panel_rect.bottom - 10 ){
				break;
			}

			const Rect<std::int32_t> tile_rect{
				panel_rect.left + padding,
				y_offset,
				panel_rect.left + padding + m_tile_size,
				y_offset + m_tile_size
			};

			const auto tile_id = m_tileset[ i ];
			const auto& tile_def = g_tile_defs[ static_cast< std::size_t >( tile_id ) ];

			const auto fallback_fill = [ &tile_def ](){
				switch( tile_def.category ){
					case TileCategory::Empty:
						return Color32{ 35, 35, 35, 255 };
					case TileCategory::Solid:
						return Color32{ 125, 88, 55, 255 };
					case TileCategory::Platform:
						return Color32{ 140, 140, 140, 255 };
					case TileCategory::Liquid:
						return ( tile_def.name == "Lava" )
							? Color32{ 220, 90, 25, 255 }
						: Color32{ 35, 115, 220, 255 };
					case TileCategory::Spawner:
						return Color32{ 90, 180, 90, 255 };
					case TileCategory::Trigger:
						return Color32{ 180, 70, 180, 255 };
					case TileCategory::Decoration:
						return Color32{ 180, 180, 80, 255 };
					default:
						return to_color32( Colors::gray );
				}
			}( );

			renderer_.fill_rect( tile_rect, fallback_fill );
			if( !tile_def.texture_name.empty() ){
				if( const auto texture_it = m_parent.m_textures.find( std::string{ tile_def.name } ); texture_it != m_parent.m_textures.end() ){
					renderer_.draw_sprite( tile_rect, texture_it->second );
				}
			}
			renderer_.draw_rect( tile_rect, Color32{ 25, 25, 25, 255 }, 1.f );

			if( m_has_hover && i == m_hovered_index ){
				renderer_.draw_rect( tile_rect, to_color32( Colors::yellow ), 2.f );
			}

			if( i == static_cast< std::size_t >( m_parent.m_active_tile_index ) ){
				const auto selected_rect = Rect<std::int32_t>{
					tile_rect.left - 2,
					tile_rect.top - 2,
					tile_rect.right + 2,
					tile_rect.bottom + 2
				};
				renderer_.draw_rect( selected_rect, to_color32( Colors::green ), 2.f );
			}

			y_offset += cell_height;
		}
	}

	void LevelEditor::TilePaletteMode::handle_mouse( Mouse const& mouse ){
		const auto mouse_pos = mouse.position();
		m_scroll_offset = static_cast< std::size_t >( m_scrollbar->value() );

		if( !contains( m_panel.bounds(), mouse_pos ) ){
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_state = State::Done;
			}
			m_has_hover = false;
			return;
		}

		if( contains( m_scrollbar->bounds(), mouse_pos ) ){
			m_has_hover = false;
			return;
		}

		if( const auto wheel_delta = mouse.wheel_delta(); wheel_delta != 0 ){
			m_scrollbar->set_value( m_scrollbar->value() + ( wheel_delta > 0 ? -1 : 1 ) );
			m_scroll_offset = static_cast< std::size_t >( m_scrollbar->value() );
		}

		const auto hit_index = hit_test( mouse_pos );
		if( hit_index.has_value() ){
			m_has_hover = true;
			m_hovered_index = *hit_index;
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_parent.m_active_tile_index = static_cast< std::int32_t >( *hit_index );
				m_state = State::Done;
			}
		}
		else{
			m_has_hover = false;
		}
	}

	void LevelEditor::TilePaletteMode::handle_keyboard( Keyboard& keyboard ){
		if( keyboard.is_pressed( Key::Escape ) ){
			m_state = State::Done;
		}

		if( keyboard.is_pressed( Key::Up ) ){
			if( m_parent.m_active_tile_index > 0 ){
				--m_parent.m_active_tile_index;
				if( static_cast< std::size_t >( m_parent.m_active_tile_index ) < m_scroll_offset ){
					m_scroll_offset = static_cast< std::size_t >( m_parent.m_active_tile_index );
				}
			}
		}
		else if( keyboard.is_pressed( Key::Down ) ){
			if( m_parent.m_active_tile_index < static_cast< std::int32_t >( m_tileset.size() ) - 1 ){
				++m_parent.m_active_tile_index;
				if( static_cast< std::size_t >( m_parent.m_active_tile_index ) >= m_scroll_offset + m_visible_count ){
					m_scroll_offset = static_cast< std::size_t >( m_parent.m_active_tile_index - m_visible_count + 1 );
				}
			}
		}

		m_scrollbar->set_value( static_cast< std::int32_t >( m_scroll_offset ) );
		m_scroll_offset = static_cast< std::size_t >( m_scrollbar->value() );

		if( keyboard.is_pressed( Key::Enter ) ){
			m_state = State::Done;
		}
	}

	std::optional<std::size_t> LevelEditor::TilePaletteMode::hit_test( vector2<std::int32_t> const& screen_pos ) const noexcept{
		if( !contains( m_panel.bounds(), screen_pos ) ){
			return std::nullopt;
		}

		const auto panel_rect = m_panel.bounds();
		const auto content_right = m_scrollbar->bounds().left - 8;
		const auto padding = ( ( content_right - panel_rect.left ) - m_tile_size ) / 2;
		const auto cell_height = m_tile_spacing + m_tile_size;
		const auto start_y = panel_rect.top + 40;

		const auto local_x = screen_pos.x - ( panel_rect.left + padding );
		const auto local_y = screen_pos.y - start_y;

		if( local_x < 0 || local_x >= m_tile_size ){
			return std::nullopt;
		}
		if( local_y < 0 ){
			return std::nullopt;
		}

		const auto index = m_scroll_offset + static_cast< std::size_t >( local_y / cell_height );
		if( index >= m_tileset.size() ){
			return std::nullopt;
		}

		const auto cell_local_y = local_y % cell_height;
		if( cell_local_y >= m_tile_size ){
			return std::nullopt;
		}

		return index;
	}

	std::int32_t LevelEditor::TilePaletteMode::max_scroll_offset() const noexcept{
		return std::max< std::int32_t >( 0, static_cast< std::int32_t >( m_tileset.size() ) - m_visible_count );
	}

	void LevelEditor::TilePaletteMode::sync_scrollbar() noexcept{
		m_scrollbar->set_range( 0, max_scroll_offset() );
		m_scrollbar->set_value( static_cast< std::int32_t >( m_scroll_offset ) );
		m_scroll_offset = static_cast< std::size_t >( m_scrollbar->value() );
	}
}
