#include "../include/dny_Filesystem.hpp"

auto dny::get_files_with_extension_of( std::filesystem::path directory, std::string_view file_extension )-> std::vector<std::filesystem::path>{
	namespace fs = std::filesystem;

	auto filenames = std::vector<fs::path>();
	if( file_extension.length() > 3 || file_extension.length() == 0 ){
		return filenames;
	}

	// Find the Images folder relative to the executable / working directory
	auto dir = directory.is_absolute()
		? directory
		: fs::current_path() / directory;

	if( !fs::exists( dir ) || !fs::is_directory( dir ) )
		return filenames;

	for( const auto& entry : fs::directory_iterator( dir ) ){
		if( entry.is_regular_file() ){
			auto ext = entry.path().extension().string();
			
			if( ext.front() == '.' )
				ext.erase( ext.begin() );

			std::transform(
				ext.begin(),
				ext.end(),
				ext.begin(),
				[]( char ch ){return std::tolower( ch ); }
			);

			if( ext == file_extension ){
				filenames.push_back( entry.path() );
			}
		}
	}

	return filenames;
}
