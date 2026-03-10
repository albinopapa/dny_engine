// Minimal test to verify dny_audio compiles and links correctly
// This file can be compiled separately or integrated into your existing Source.cpp

#include "dny_audio.hpp"
#include <iostream>

void test_audio_api(){
	try{
		// Create audio engine
		dny::audio_engine audio;
		
		// Test loading an audio file
		// Note: Update path to an actual audio file for real testing
		auto clip = audio.load_audio( "test.wav" );
		
		// Test API methods
		clip.set_loop( true );
		clip.set_volume( 0.5f );
		
		// These will work once an actual file is loaded
		// clip.play();
		// bool playing = clip.is_playing();
		// clip.stop();
		
		audio.set_master_volume( 0.8f );
		float volume = audio.get_master_volume();
		
		audio.update();
		
		std::cout << "Audio API test passed!\n";
		std::cout << "Master volume: " << volume << "\n";
		std::cout << "Clip is looping: " << ( clip.is_looping() ? "yes" : "no" ) << "\n";
	}
	catch( ... ){
		std::cerr << "Audio API test failed!\n";
	}
}

// Uncomment to run test
// int main(){
//     test_audio_api();
//     return 0;
// }
