#pragma once

#include <cassert>
#include <cstdint>

namespace dny{
	enum class TileCategory : std::int32_t{
		Empty = 0,
		Platform,
		Trigger,
		Spawner,
		Liquid,
		Decoration,
		// Solid tiles imply there's a collider, but no special behavior like 
		// moving platforms or triggers
		Solid,			
	};

	[[nodiscard]] constexpr TileCategory category_from_int( std::int32_t num )noexcept{
		assert( num >= 0 && num <= static_cast< std::int32_t >( TileCategory::Solid ) );
		switch( num ){
			case 0:return  TileCategory::Empty;
			case 1:return  TileCategory::Platform;
			case 2:return  TileCategory::Trigger;
			case 3:return  TileCategory::Spawner;
			case 4:return  TileCategory::Liquid;
			case 5:return  TileCategory::Decoration;
			case 6:return  TileCategory::Solid;
			default:return TileCategory::Empty;
		}
	}
	[[nodiscard]] constexpr std::int32_t category_to_int( TileCategory cat )noexcept{
		switch( cat ){
			case TileCategory::Empty:		return 0;
			case TileCategory::Platform:	return 1;
			case TileCategory::Trigger:		return 2;
			case TileCategory::Spawner:		return 3;
			case TileCategory::Liquid:		return 4;
			case TileCategory::Decoration:	return 5;
			case TileCategory::Solid:		return 6;
			default:return -1;
		}
	}
}
