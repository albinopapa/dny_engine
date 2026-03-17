#include "editor/tile_palette_mode.hpp"
#include "editor/editor_mode.hpp"

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
		// TODO: Use renderer2d to draw dialog background
		// Draw dialog outline
		// Graphics::instance().draw_rectangle( m_dialog_rect, Colors::White );

		// Draw title
		// TODO: Add title rendering

		// Calculate visible area for tiles
		const auto padding = ( m_dialog_panel.bounds().width() - m_tile_size ) / 2;
		const auto cell_height = m_tile_spacing + m_tile_size;
		auto y_offset = m_dialog_panel.bounds().top + 40; // Leave space for title

		// Draw tile grid
		for( std::size_t i = m_scroll_offset; i < m_tileset.size(); ++i ){
			if( y_offset + m_tile_size > m_dialog_panel.bounds().bottom - 10 ){
				break; // Stop if we've reached the bottom
			}

			const Rect<std::int32_t> tile_rect{
				m_dialog_panel.bounds().left + padding,
				y_offset,
				m_dialog_panel.bounds().left + padding + m_tile_size,
				y_offset + m_tile_size
			};

			// Highlight if hovered
			if( m_has_hover && i == m_hovered_index ){
				// TODO: Draw highlight
				// Graphics::fill_rectangle( tile_rect, Colors::Yellow );
			}

			// Highlight if selected
			if( i == static_cast< std::size_t >( m_parent.m_active_tile_index ) ){
				// TODO: Draw selection border
				// Graphics::draw_rectangle( tile_rect, Colors::Green );
			}

			// TODO: Draw tile sprite or colored rectangle based on tile type
			const auto tile_id = m_tileset[ i ];
			// const auto& tile_def = g_tile_defs[ tile_id ];

			// Graphics::draw_sprite or Graphics::fill_rectangle based on tile type

			y_offset += cell_height;
		}

		// TODO: Draw scroll indicator if needed
		if( m_scroll_offset > 0 || m_scroll_offset + 5 < m_tileset.size() ){
			// Draw scroll arrows
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
