#include "paint_tool.hpp"

dny::LevelEditor::PaintTool::PaintTool( LevelEditor& parent_ ) noexcept
	:
	m_parent( parent_ ),
	m_start_tile( std::nullopt )
{}

void dny::LevelEditor::PaintTool::update( Mouse const& mouse_, Keyboard & keyboard_ ){
	if( m_parent.clamp_camera(); mouse_.is_pressed( MouseButton::Left ) ){
		m_start_tile = m_parent.m_tilemap_view.screen_to_tile_index(
			mouse_.position(), m_parent.m_camera
		);
	}
	else if( mouse_.is_held( MouseButton::Left ) && m_start_tile.has_value() ){
		const auto current_tile = m_parent.m_tilemap_view.screen_to_tile_index(
			mouse_.position(), m_parent.m_camera
		);
		if( current_tile != m_start_tile ){
			m_parent.place_tile( mouse_ );
			m_start_tile = current_tile;
		}
	}
	else if( mouse_.is_released( MouseButton::Left ) ){
		m_start_tile = std::nullopt;
	}
}
