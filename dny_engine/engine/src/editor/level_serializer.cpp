#include "editor/level_serializer.hpp"
#include "editor/file_system.hpp"
#include "editor/utilities.hpp"

#include <concepts>
#include <format>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream> // for std::stringstream
#include <type_traits>
#include <unordered_map>

namespace dny{
	static constexpr std::string_view map_version = "0.00.001";
	static constexpr std::string_view pfm_version = "0.00.001";
	static constexpr std::string_view def_version = "0.00.001";

	// ---------- PropertyParser ----------
	class PropertyParser{
	public:
		[[nodiscard]] static auto get_property( std::string const& line ){
			std::string key, value;
			for( auto x = 0; x < line.length(); ++x ){
				char ch = std::tolower( line[ x ] );

				if( ch == '.' ){
					++x;
					key = parse_key( line, x );
					x += key.length();
				}
				else if( ch == '=' ){
					++x;
					if( line[ x ] == '\"' ){
						++x;
					}

					value = parse_value( line, x );
					x += value.length();
				}
			}


			return std::pair{std::move( key ), std::move( value )};
		}

		template<Number NumberT>
		[[nodiscard]] static std::optional<NumberT> string_to_number( std::string const& str ) {
			char* end_iter = nullptr;
			const auto value = [ & ]()->NumberT{
				if constexpr( sizeof( NumberT ) == 64 )
					if constexpr( std::is_integral_v<NumberT> )
						return std::strtoll( str.c_str(), std::addressof( end_iter ), 10 );
					else
						return std::strtod( str.c_str(), std::addressof( end_iter ) );
				else{
					if constexpr( std::is_integral_v<NumberT> )
						return std::strtol( str.c_str(), std::addressof( end_iter ), 10 );
					else
						return std::strtof( str.c_str(), std::addressof( end_iter ) );
				}
			}();

			if( end_iter == str.c_str() ){
				return std::nullopt; // conversion failed
			}

			return value;
		};

	private:

		[[nodiscard]] static auto is_alpha( char ch )noexcept{
			return ch >= 'a' && ch <= 'z';
		};

		[[nodiscard]] static auto is_num( char ch )noexcept{
			return ch >= '0' && ch <= '9';
		};

		[[nodiscard]] static auto parse_key( std::string const& line, std::int32_t& x )noexcept->std::string{
			++x;
			const auto start = x;
			while( is_alpha( line[ x ] ) ){
				++x;
			}

			return line.substr( start, x - start );
		};

		[[nodiscard]] static auto parse_value( std::string const& line, std::int32_t& x )noexcept->std::string{
			++x;
			if( line[ x ] == '\"' ){
				++x;
			}

			const auto start = x;
			while( line[ x ] != '\n' && line[ x ] != '\"' ){
				++x;
			}

			return line.substr( start, x - start );
		};

	};

	// ---------- TileMapSerializer ----------

	void LevelSerializer::save( LevelDocument const& level ){
		if( !is_valid_win_filename( std::string{ level.basename } ) )
			return;

		auto level_file = std::ofstream{ level.basename + ".lvl" };
		level_file << "LevelFileVersion = 0.00.001\n";
		level_file << std::format( "TilemapFile = {}.map\n", level.basename );
		level_file << std::format( "TileDefFile = {}.def\n", level.basename );
		level_file << std::format( "PlatformDefFile = {}.pfm\n", level.basename );
		level_file << std::format( "TriggerDefFile = {}.trg\n", level.basename );
		level_file << std::format( "EntityDefFile = {}.ety\n", level.basename );
		level_file << std::format( "PolylineFile = {}.pln\n", level.basename );

		save_map_file( level );
		save_def_file( level );
		save_pfm_file( level );
		save_trg_file( level );
		save_ety_file( level );
		save_pln_file( level );
	}

	void LevelSerializer::save_map_file( LevelDocument const& level ){
		std::ofstream map_file{ level.basename + ".map" };
		if( !map_file ) return;

		const auto size = level.tilemap.size();

		// Version line
		map_file << map_version << '\n';

		// Size line
		map_file << std::format( "({}, {})\n", size.width, size.height );

		for( std::int32_t y = 0; y < size.height; ++y ){
			for( std::int32_t x = 0; x < size.width; ++x ){
				const auto& tile = level.tilemap.get_tile( { x, y } );

				map_file << std::format( "Tile{{\n" );
				map_file << std::format( "\t.id = {}\n", tile.definition_id );
				map_file << std::format( "\t.orientatio_index = {}\n", tile.orientation_index );
				map_file << std::format( "\t.platform_id = {}\n", tile.platform_id );
				map_file << std::format( "\t.entity_id = {}\n", tile.entity_id );
				map_file << std::format( "\t.trigger_id = {}\n", tile.trigger_id );
				map_file << std::format( "}}\n" );
			}
		}
	}

	void LevelSerializer::save_def_file( LevelDocument const& level ){
		std::ofstream def_file{ std::string{ level.basename } + ".def" };
		if( !def_file )return;

		for( const auto& def : level.tile_defs ){
			def_file << std::format( "TileDef{{\n" );
			def_file << std::format( "\t.name = {}\n", def.name );
			def_file << std::format( "\t.category = {}\n", static_cast< std::size_t >( def.category ) );
			def_file << std::format( "\t.friction = {}\n", def.friction );
			def_file << std::format( "\t.damage = {}\n", def.damage );
			def_file << std::format( "\t.texture_name = {}\n", def.texture_name );
			def_file << std::format( "}}\n" );
		}
	}

	void LevelSerializer::save_pfm_file( LevelDocument const& level ){
		std::ofstream pfm_file{ std::string{ level.basename } + ".pfm" };
		if( !pfm_file )return;

		for( const auto& pfm : level.platform_defs ){
			pfm_file << std::format( "PlatformDef{{\n" );
			pfm_file << std::format( "\t.start = ({}, {})\n",	pfm.start.x, pfm.start.y );
			pfm_file << std::format( "\t.end = ({}, {})\n",		pfm.end.x, pfm.end.y );
			pfm_file << std::format( "\t.size = ({}, {})\n",	pfm.size.width, pfm.size.height );
			pfm_file << std::format( "\t.speed = {}\n",			pfm.speed );
			pfm_file << std::format( "\t.texture_name = {}\n",	pfm.texture_name );
			pfm_file << std::format( "\t.is_moveable = {}\n",	pfm.is_moveable ? "true" : "false" );
			pfm_file << std::format( "}}\n" );
		}
	}

	void LevelSerializer::save_trg_file( LevelDocument const& level ){
		std::ofstream file{ level.basename + ".trg" };
		if( !file )return;

		for( auto const& trg : level.trigger_defs ){
			const auto& region = trg.condition.region;

			file << std::format( "Trigger{{\n" );
			file << std::format( "\tcondition_type = {}\n", static_cast< std::int32_t >( trg.condition.type ) );
			file << std::format( "\tcondition_region = ({},{},{},{})\n", region.left, region.top, region.width(), region.height() );
			file << std::format( "\tcondition_id = {}\n", trg.condition.id );
			file << std::format( "\taction_type = {}\n", static_cast< std::int32_t >( trg.action.type ) );
			file << std::format( "\taction_target_type = {}", static_cast< std::int32_t >( trg.action.target.type ) );
			file << std::format( "\taction_target_id = {}\n", trg.action.target.id );
			file << std::format( "\taction_id = {}\n", trg.action.id );
			file << std::format( "\tfire_once = {}\n", trg.fire_once ? "true" : "false" );
			file << std::format( "\tid = {}\n", trg.id );
			file << std::format( "}}\n" );
		}
	}

	void LevelSerializer::save_ety_file( LevelDocument const& level ){
		std::ofstream file{ level.basename + ".ety" };
		if( !file ) return;

		for( auto const& ety : level.entity_defs ){
			file << "Entity{\n";
			file << std::format( "\tname = {}\n", ety.name );
			file << std::format( "\tspawn_pos = ({},{})\n",
				ety.spawn_pos.x, ety.spawn_pos.y );
			file << std::format( "\tsize = ({},{})\n",
				ety.size.width, ety.size.height );
			file << std::format( "\tmax_speed = {}\n", ety.max_speed );
			file << std::format( "\tdamage = {}\n", ety.damage );
			file << std::format( "\thealth = {}\n", ety.health );
			file << std::format( "\tid = {}\n", ety.id );
			file << "}\n";
		}
	}

	void LevelSerializer::save_pln_file( LevelDocument const& level ){
		std::ofstream file{ level.basename + ".pln" };
		if( !file ) return;

		for( auto const& pl : level.collisions ){
			file << "Polyline{\n";
			// TODO: Renable if needed
			//file << std::format( "\tis_closed = {}\n", pl.is_closed ? "true" : "false" );

			for( auto const& pt : pl.points ){
				file << std::format( "\tpoint = ({},{})\n", pt.x, pt.y );
			}

			file << "}\n";
		}
	}

	template<template<typename> typename Vec, Number Scalar>
	void parse_2d( Vec<Scalar>& result, std::string const& value ){
		std::stringstream ss{ value };
		char junk = 0;

		Scalar x = {};
		Scalar y = {};

		// Expected format: (x, y)
		ss >> junk >> x >> junk >> y >> junk;
		result = Vec<Scalar>{ x, y };
	}

	template<template<typename> typename Rec, Number Scalar>
	void parse_rect( Rec<Scalar>& rect, std::string const& value ){
		std::stringstream ss{ value };
		char junk = 0;

		Scalar x = {};
		Scalar y = {};
		Scalar w = {};
		Scalar h = {};

		// Expected format: (x, y, w, h)
		ss >> junk >> x >> junk >> y >> junk >> w >> junk >> h >> junk;
		
		rect = Rect<Scalar>{ x, y, x + w, y + h };
	}

	void fill_tile(std::string const& key, std::string const& value, Tile& tile ){
		if( key == "id" ){
			const auto result = PropertyParser::string_to_number<std::int32_t>( value );
			if( result )
				tile.definition_id = *result;
		}
		else if( key == "rotation" ){
			const auto result = PropertyParser::string_to_number<std::int32_t>( value );
			if( result )
				tile.orientation_index = *result;
		}
		else if( key == "platform_id" ){
			const auto result = PropertyParser::string_to_number<std::int32_t>( value );
			if( result )
				tile.platform_id = *result;
		}
		else if( key == "entity_id" ){
			const auto result = PropertyParser::string_to_number<std::int32_t>( value );
			if( result )
				tile.entity_id = *result;
		}
		else if( key == "trigger_id" ){
			const auto result = PropertyParser::string_to_number<std::int32_t>( value );
			if( result )
				tile.trigger_id = *result;
		}
	}

	void fill_tile_def( std::string const& key, std::string const& value, TileDef& def ){
		if( key == "name" ){
			def.name = value;
		}
		else if( key == "category" ){
			const auto result = PropertyParser::string_to_number<std::size_t>( value );
			if( result )
				def.category = static_cast< TileCategory >( *result );
		}
		else if( key == "friction" ){
			const auto result = PropertyParser::string_to_number<float>( value );
			if( result )
				def.friction = *result;
		}
		else if( key == "damage" ){
			const auto result = PropertyParser::string_to_number<float>( value );
			if( result )
				def.damage = *result;
		}
		else if( key == "texture_name" ){
			def.texture_name = value;
		}
	}

	void fill_platform_def( std::string const& key, std::string const& value, PlatformDef& def ){
		
		if( key == "start" ){
			parse_2d( def.start, value );
		}
		else if( key == "end" ){
			parse_2d( def.end, value );
		}
		else if( key == "size" ){
			parse_2d( def.size, value );
		}
		else if( key == "speed" ){
			const auto result = PropertyParser::string_to_number<float>( value );
			if( result )
				def.speed = *result;
		}
		else if( key == "id" ){
			const auto result = PropertyParser::string_to_number<std::int32_t>( value );
			if( result )
				def.id = *result;
		}
		else if( key == "texture_index" ){
			def.texture_name = value;
		}
		else if( key == "is_moveable" ){
			def.is_moveable = value == "true" ? true : false;
		}
	}

	std::string trim( std::string const& s ){
		auto begin = s.find_first_not_of( " \t\r\n" );
		auto end = s.find_last_not_of( " \t\r\n" );

		if( begin == std::string::npos ) return "";
		return s.substr( begin, end - begin + 1 );
	}

	void fill_trigger_def( std::string const& key, std::string const& value, TriggerDef& def ){
		// Example of Trigger file layout
		//Trigger{
		//	condition_type = 1
		//	condition_region = ( x, y, w, h )
		//	condition_id = 0
		//	action_type = 2
		//	action_target_type = 1
		//	action_target_id = 10
		//	action_id = 5
		//	fire_once = true
		//	id = 3
		//}
		if( key == "condition_type" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.condition.type = to_trigger_condition_type( *result );
		}
		else if( key == "condition_region" ){
			parse_rect( def.condition.region, value );
		}
		else if( key == "condition_id" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.condition.id = *result;
		}
		else if( key == "action_type" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.action.type = to_trigger_action_type( *result );
		}
		else if( key == "action_target_type" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.action.target.type = to_trigger_target_type( *result );
		}
		else if( key == "action_target_id" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.action.target.id = *result;
		}
		else if( key == "action_id" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.action.id = *result;
		}
		else if( key == "fire_once" ){
			def.fire_once = value == "true";
		}
		else if( key == "id" ){
			if( const auto result = PropertyParser::string_to_number<std::int32_t>( value ) )
				def.id = *result;
		}
	}

	void fill_entity_def( std::string const& key, std::string const& value, EntityDef& def ){
		if( key == "name" ){
			def.name = value;
		}
		else if( key == "spawn_pos" ){
			parse_2d( def.spawn_pos, value );     // (<x>,<y>)
		}
		else if( key == "size" ){
			parse_2d( def.size, value );          // (<w>,<h>)
		}
		else if( key == "max_speed" ){
			if( auto v = PropertyParser::string_to_number<float>( value ) )
				def.max_speed = *v;
		}
		else if( key == "damage" ){
			if( auto v = PropertyParser::string_to_number<float>( value ) )
				def.damage = *v;
		}
		else if( key == "health" ){
			if( auto v = PropertyParser::string_to_number<float>( value ) )
				def.health = *v;
		}
		else if( key == "id" ){
			if( auto v = PropertyParser::string_to_number<int32_t>( value ) )
				def.id = *v;
		}
	}

	void LevelSerializer::load_map_file( LevelDocument& level ){
		std::ifstream file{ level.basename + ".map" };
		if( !file ){
			return;
		}

		std::string line;

		// Version
		if( !std::getline( file, line ) ) return;
		if( line != map_version ){
			return;
		}

		// Size line
		if( !std::getline( file, line ) ) return;
		auto size = dims2<std::int32_t>{};
		parse_2d( size, line );
		level.tilemap.resize( size );

		// Tile data lines
		for( std::int32_t y = 0; y < size.height; ++y ){
			if( !std::getline( file, line ) ) break;

			std::stringstream ss( line );
			for( std::int32_t x = 0; x < size.width; ++x ){
				if( line == "Tile{" ){
					Tile tile{};

					while( std::getline( file, line ) && line != "}" ){
						auto [key, value] = PropertyParser::get_property( line );
						if( !key.empty() )
							fill_tile( key, value, tile );
					}

					level.tilemap.get_tile( { x, y } ) = tile;
				}
			}
		}
	}

	void LevelSerializer::load_def_file( LevelDocument& level ){
		std::ifstream file{ level.basename + ".def" };
		if( !file ) return;

		std::string line;
		while( std::getline( file, line ) ){
			if( line == "TileDef{" ){
				TileDef def{};

				while( std::getline( file, line ) && line != "}" ){
					auto [key, value] = PropertyParser::get_property( line );
					if( !key.empty() )
						fill_tile_def( key, value, def );
				}

				level.tile_defs.push_back( def );
			}
		}
	}

	void LevelSerializer::load_pfm_file( LevelDocument& level ){
		std::ifstream file{ level.basename + ".pfm" };
		if( !file ) return;

		std::string line;
		while( std::getline( file, line ) ){
			if( line == "PlatformDef{" ){
				PlatformDef def{};

				while( std::getline( file, line ) && line != "}" ){
					auto [key, value] = PropertyParser::get_property( line );
					if( !key.empty() )
						fill_platform_def( key, value, def );
				}

				level.platform_defs.push_back( def );
			}
		}
	}

	void LevelSerializer::load_trg_file( LevelDocument& level ){
		std::ifstream file( level.basename + ".trg" );
		if( !file ) return;

		std::string line;

		while( std::getline( file, line ) ){
			auto trimmed = trim( line );

			if( trimmed == "Trigger{" ){
				TriggerDef def;

				while( std::getline( file, line ) ){
					trimmed = trim( line );
					if( trimmed == "}" ) break;

					auto [key, value] = PropertyParser::get_property( trimmed );
					if( !key.empty() )
						fill_trigger_def( key, value, def );
				}

				level.trigger_defs.push_back( def );
			}
		}
	}

	void LevelSerializer::load_ety_file( LevelDocument& level ){
		std::ifstream file{ level.basename + ".ety" };
		if( !file ) return;

		std::string line;

		while( std::getline( file, line ) ){
			auto stripped = trim( line );

			if( stripped == "Entity{" ){
				EntityDef def;

				while( std::getline( file, line ) ){
					stripped = trim( line );
					if( stripped == "}" )
						break;

					auto [key, value] = PropertyParser::get_property( stripped );
					if( key.empty() ) continue;
					fill_entity_def( key, value, def );
				}

				level.entity_defs.push_back( def );
			}
		}
	}

	void LevelSerializer::load_pln_file( LevelDocument& level ){
		std::ifstream file{ level.basename + ".pln" };
		if( !file ) return;

		std::string line;

		while( std::getline( file, line ) ){
			auto stripped = trim( line );

			if( stripped == "Polyline{" ){
				polyline_collider<float> pl;

				while( std::getline( file, line ) ){
					stripped = trim( line );
					if( stripped == "}" )
						break;

					auto [key, value] = PropertyParser::get_property( stripped );
					if( key.empty() ) continue;

					/*if( key == "is_closed" ){
						pl.is_closed = ( value == "true" );
					}
					else */if( key == "point" ){
						vector2<float> pt;
						parse_2d( pt, value );   // uses your template-based parser
						pl.points.push_back( pt );
					}
				}

				level.collisions.push_back( pl );
			}
		}
	}

	void LevelSerializer::load( LevelDocument& level ){
		if( !is_valid_win_filename( std::string{ level.basename } ) )
			return;

		load_pfm_file( level );
		load_def_file( level );
		load_map_file( level );
		load_trg_file( level );
		load_ety_file( level );
		load_pln_file( level );
	}
}
