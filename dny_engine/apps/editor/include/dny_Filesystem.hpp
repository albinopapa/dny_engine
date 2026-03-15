#pragma once

#include <filesystem>

namespace dny{
	constexpr bool is_valid_win_filename_char( unsigned char c )noexcept{
		// Forbidden characters
		switch( c ){
			case '"': case '<': case '>': case '|':
			case ':': case '?': case '*':
			case '/': case '\\':
				return false;
		}
		// No control characters or DEL
		if( c <= 31 || c == 127 )
			return false;

		return true;
	}

	// List of reserved device names (case-insensitive)
	constexpr bool is_reserved_device_name( const std::string& name ){
		static constexpr const char* reserved[] = {
			"CON", "PRN", "AUX", "NUL",
			"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
			"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
		};

		std::string upper;
		upper.reserve( name.size() );
		for( char c : name )
			upper.push_back( std::toupper( static_cast< unsigned char >( c ) ) );

		for( const char* r : reserved )
			if( upper == r ) return true;

		return false;
	}

	constexpr bool is_valid_win_filename( const std::string& name ){
		if( name.empty() )
			return false;

		// Windows disallows names ending with space or period
		char last = name.back();
		if( last == ' ' || last == '.' )
			return false;

		// Cannot consist entirely of periods or spaces
		bool has_real_char = false;

		for( unsigned char c : name ){
			if( !is_valid_win_filename_char( c ) )
				return false;

			if( c != ' ' && c != '.' )
				has_real_char = true;
		}

		if( !has_real_char )
			return false;

		// Check for reserved device names (with optional extension)
		auto dotpos = name.find( '.' );
		std::string basename = ( dotpos == std::string::npos )
			? name
			: name.substr( 0, dotpos );

		if( is_reserved_device_name( basename ) )
			return false;

		return true;
	}

	auto get_files_with_extension_of( std::filesystem::path directory, std::string_view file_extension )
		-> std::vector<std::filesystem::path>;
}
