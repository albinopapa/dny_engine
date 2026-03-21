#include "tile_attributes_mode.hpp"

dny::LevelEditor::TileAttributesMode::TileAttributesMode( LevelEditor& parent, Rect<std::int32_t> const& area_ ) noexcept
	:
	basic_mode{ "tile_attributes_mode", "Tile Attributes", area_ },
	m_parent( parent ){

	// Attribute panels: ( context driven )
	// Category (dropdown)
	// Empty - no special properties
	// Liquid - 
	//  * Friction (0-1, default 0)
	//  * Damage (0-inf, default 0)
	// Solid - ( implies there's a collider, but no special behavior like moving platforms or triggers )
	// Platform - 
	// * Texture (use TextureSelectMode to select, display thumbnail of selection)
	// * start/end positions (2 X/Y input boxes)
	// * size (width/height input boxes)
	// * speed (input box)
	// Spawner -
	// * Texture (use TextureSelectMode to select, display thumbnail of selection)
	// * spawn position (X/Y input boxes)
}

void dny::LevelEditor::TileAttributesMode::update( Mouse const& mouse_, Keyboard& keyboard_ ){

}
