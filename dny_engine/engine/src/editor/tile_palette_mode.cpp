#include "editor/tile_palette_mode.hpp"
#include "editor/editor_mode.hpp"

#include "editor/tile_map.hpp"
#include "graphics/colors.hpp"
#include "input/input.hpp"
#include "graphics/font.hpp"

namespace dny{
	LevelEditor::TilePaletteMode::TilePaletteMode( LevelEditor& parent, Rect<std::int32_t> const& dialog_rect )
		: m_parent{ parent },
		m_dialog_panel{ "tile_palette_panel", "Tile Palette", dialog_rect.top_left(), dialog_rect.size() }
	{
	}

	void LevelEditor::TilePaletteMode::update( Mouse const& mouse, Keyboard& keyboard ){
		handle_mouse( mouse );
		handle_keyboard( keyboard );
	}

	void LevelEditor::TilePaletteMode::render( renderer2d& renderer_, Font const& font_ ) const{
		const auto panel_rect = m_dialog_panel.bounds();
		const auto title_pos = panel_rect.top_left() + vector2<std::int32_t>{ 8, 6 };
		const auto visible_count = 5ui64;

		renderer_.fill_rect( panel_rect, ColorF{ 0.10f, 0.10f, 0.12f, 0.94f } );
		renderer_.draw_rect( panel_rect, to_color32( Colors::white ), 2.f );
		renderer_.draw_text( m_dialog_panel.title(), title_pos, font_, to_color32( Colors::white ) );

		const auto padding = ( panel_rect.width() - m_tile_size ) / 2;
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

			Color32 fallback_fill = to_color32( Colors::gray );
			switch( tile_def.category ){
				case TileCategory::Empty:
					fallback_fill = Color32{ 35, 35, 35, 255 };
					break;
				case TileCategory::Solid:
					fallback_fill = Color32{ 125, 88, 55, 255 };
					break;
				case TileCategory::Platform:
					fallback_fill = Color32{ 140, 140, 140, 255 };
					break;
				case TileCategory::Liquid:
					fallback_fill = ( tile_def.name == "Lava" )
						? Color32{ 220, 90, 25, 255 }
						: Color32{ 35, 115, 220, 255 };
					break;
				case TileCategory::Spawner:
					fallback_fill = Color32{ 90, 180, 90, 255 };
					break;
				case TileCategory::Trigger:
					fallback_fill = Color32{ 180, 70, 180, 255 };
					break;
				case TileCategory::Decoration:
					fallback_fill = Color32{ 180, 180, 80, 255 };
					break;
			}

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

		if( m_scroll_offset > 0 ){
			const auto up_center = vector2<std::int32_t>{ panel_rect.right - 16, panel_rect.top + 20 };
			renderer_.fill_circle( up_center, 6, Color32{ 220, 220, 220, 255 } );
			renderer_.draw_line(
				up_center + vector2<std::int32_t>{ -4, 2 },
				up_center + vector2<std::int32_t>{ 0, -3 },
				Color32{ 25, 25, 25, 255 },
				1.f
			);
			renderer_.draw_line(
				up_center + vector2<std::int32_t>{ 0, -3 },
				up_center + vector2<std::int32_t>{ 4, 2 },
				Color32{ 25, 25, 25, 255 },
				1.f
			);
		}

		if( m_scroll_offset + visible_count < m_tileset.size() ){
			const auto down_center = vector2<std::int32_t>{ panel_rect.right - 16, panel_rect.bottom - 16 };
			renderer_.fill_circle( down_center, 6, Color32{ 220, 220, 220, 255 } );
			renderer_.draw_line(
				down_center + vector2<std::int32_t>{ -4, -2 },
				down_center + vector2<std::int32_t>{ 0, 3 },
				Color32{ 25, 25, 25, 255 },
				1.f
			);
			renderer_.draw_line(
				down_center + vector2<std::int32_t>{ 0, 3 },
				down_center + vector2<std::int32_t>{ 4, -2 },
				Color32{ 25, 25, 25, 255 },
				1.f
			);
		}
	}

	void LevelEditor::TilePaletteMode::handle_mouse( Mouse const& mouse ){
		const auto mouse_pos = mouse.position();

		// Check if mouse is over dialog
		if( !contains( m_dialog_panel.bounds(), mouse_pos ) ){
			// Click outside dialog - close it
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
			m_has_hover = false;
			return;
		}

		// Handle scrolling
		const auto wheel_delta = mouse.wheel_delta();
		if( wheel_delta > 0 && m_scroll_offset > 0 ){
			--m_scroll_offset;
		}
		else if( wheel_delta < 0 ){
			const auto max_scroll = m_tileset.size() > 5 ? m_tileset.size() - 5 : 0;
			if( m_scroll_offset < max_scroll ){
				++m_scroll_offset;
			}
		}

		// Hit test for tile selection
		const auto hit_index = hit_test( mouse_pos );
		if( hit_index.has_value() ){
			m_has_hover = true;
			m_hovered_index = *hit_index;

			// Select tile on click
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_parent.m_active_tile_index = static_cast< std::int32_t >( *hit_index );
				m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
			}
		}
		else{
			m_has_hover = false;
		}
	}

	void LevelEditor::TilePaletteMode::handle_keyboard( Keyboard& keyboard ){
		// Close palette on Escape
		if( keyboard.is_pressed( Key::Escape ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}

		// Navigate with arrow keys
		if( keyboard.is_pressed( Key::Up ) ){
			if( m_parent.m_active_tile_index > 0 ){
				--m_parent.m_active_tile_index;

				// Adjust scroll if needed
				if( static_cast< std::size_t >( m_parent.m_active_tile_index ) < m_scroll_offset ){
					m_scroll_offset = m_parent.m_active_tile_index;
				}
			}
		}
		else if( keyboard.is_pressed( Key::Down ) ){
			if( m_parent.m_active_tile_index < static_cast< std::int32_t >( m_tileset.size() ) - 1 ){
				++m_parent.m_active_tile_index;

				// Adjust scroll if needed
				const auto visible_count = 5;
				if( static_cast< std::size_t >( m_parent.m_active_tile_index ) >= m_scroll_offset + visible_count ){
					m_scroll_offset = m_parent.m_active_tile_index - visible_count + 1;
				}
			}
		}

		// Confirm selection with Enter
		if( keyboard.is_pressed( Key::Enter ) ){
			m_parent.transition_mode( std::make_unique<EditorMode>( m_parent, m_parent.m_viewport ) );
		}
	}

	std::optional<std::size_t> LevelEditor::TilePaletteMode::hit_test( vector2<std::int32_t> const& screen_pos ) const noexcept{
		if( !contains( m_dialog_panel.bounds(), screen_pos ) ){
			return std::nullopt;
		}

		const auto padding = ( m_dialog_panel.bounds().width() - m_tile_size ) / 2;
		const auto cell_height = m_tile_spacing + m_tile_size;
		const auto start_y = m_dialog_panel.bounds().top + 40; // Title space

		const auto local_x = screen_pos.x - ( m_dialog_panel.bounds().left + padding );
		const auto local_y = screen_pos.y - start_y;

		// Check if within tile horizontal bounds
		if( local_x < 0 || local_x >= m_tile_size ){
			return std::nullopt;
		}

		// Check if within vertical bounds
		if( local_y < 0 ){
			return std::nullopt;
		}

		const auto index = m_scroll_offset + ( local_y / cell_height );

		if( index >= m_tileset.size() ){
			return std::nullopt;
		}

		// Make sure we're actually clicking on the tile, not the spacing
		const auto cell_local_y = local_y % cell_height;
		if( cell_local_y >= m_tile_size ){
			return std::nullopt; // Clicked in spacing area
		}

		return index;
	}
}
