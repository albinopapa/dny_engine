#include "editor/tile_map.hpp"

#include <cassert>

namespace dny{
	dims2<std::int32_t> TileMap::size()const noexcept{
		return m_size;
	}

	Tile& TileMap::operator[]( vector2<std::int32_t> const& idx )&{
		return access_tile( idx.x, idx.y );
	}

	Tile const& TileMap::operator[]( vector2<std::int32_t> const& idx ) const&{
		return access_tile( idx.x, idx.y );
	}

	Tile& TileMap::get_tile( vector2<std::int32_t> const& pt ) noexcept{
		return access_tile( pt.x, pt.y );
	}

	Tile const& TileMap::get_tile( vector2<std::int32_t> const& pt ) const noexcept{
		return access_tile( pt.x, pt.y );
	}

	Rect<std::int32_t> TileMap::tile_rect( vector2<std::int32_t> const& tile_index ) const noexcept{
		assert( tile_index.x < static_cast< std::int32_t >( m_size.width ) );
		assert( tile_index.y < static_cast< std::int32_t >( m_size.height ) );
		
		const auto scaled = tile_index * Tile::size;
		return Rect<std::int32_t>{
			scaled.x,
			scaled.y,
			scaled.x + Tile::size,
			scaled.y + Tile::size
		};
	}

	void TileMap::resize( dims2<std::int32_t> const& new_size ){
		assert( new_size.width > 0 );
		assert( new_size.height > 0 );

		const auto mem_size = 
			static_cast< std::size_t >( new_size.width * new_size.height );

		auto new_tiles = std::vector<Tile>{ mem_size };
		const auto min_width = std::min( new_size.width, m_size.width );
		const auto min_height = std::min( new_size.height, m_size.height );

		for( std::int32_t y = 0; y < min_height; ++y ){
			for( std::int32_t x = 0; x < min_width; ++x ){
				const auto src_idx = x + ( y * m_size.width );
				const auto dst_idx = x + ( y * new_size.width );

				new_tiles[ dst_idx ] = std::move( m_tiles[ src_idx ] );
			}
		}

		m_tiles = std::move( new_tiles );
		m_size = new_size;
	}

	Tile& TileMap::access_tile( std::int32_t x, std::int32_t y ) noexcept{
		assert( !m_tiles.empty() && "TileMap is empty." );
		assert( x >= 0 && y >= 0 );
		assert( x < m_size.width && y < m_size.height );
		return m_tiles[ y * m_size.width + x ];
	}

	Tile const& TileMap::access_tile( std::int32_t x, std::int32_t y ) const noexcept{
		assert( !m_tiles.empty() && "TileMap is empty." );
		assert( x >= 0 && y >= 0 );
		assert( x < m_size.width && y < m_size.height );
		return m_tiles[ y * m_size.width + x ];
	}
}
