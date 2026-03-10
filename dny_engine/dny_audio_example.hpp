// Example usage of the dny audio API
//
// Basic integration example:
//
// 1. Create an audio_engine instance (typically in your main game class)
//    dny::audio_engine audio;
//
// 2. Load audio files during initialization
//    auto jump_sound = audio.load_audio( "sounds/jump.wav" );
//    auto bgm = audio.load_audio( "music/background.mp3" );
//
// 3. Play sounds on events
//    if( player_jumped ){
//        jump_sound.play();
//    }
//
// 4. Loop background music
//    bgm.set_loop( true );
//    bgm.play();
//
// 5. Control volume
//    audio.set_master_volume( 0.7f );
//    jump_sound.set_volume( 0.5f );
//
// 6. Check playback state
//    if( bgm.is_playing() ){
//        // Music is currently playing
//    }
//
// 7. Stop/pause audio
//    bgm.pause();
//    bgm.resume();
//    bgm.stop();
//
// Supported formats: WAV, MP3, WMA, AAC (any format supported by Windows Media Foundation)
//
// Performance notes:
// - Load audio files once during initialization
// - Reuse audio_clip objects for sound effects that play multiple times
// - Call audio.update() once per frame (currently optional, reserved for future features)
// - audio_clip uses move semantics only, store them as members or in containers

#pragma once

#include "dny_audio.hpp"

// Example game class integration:
// Note: If storing audio_clip members, you MUST define destructor in cpp file
// where dny_audio.cpp is compiled, or use unique_ptr/optional wrappers.
//
// Example of proper usage:
// class game_with_audio{
// public:
//     game_with_audio();
//     ~game_with_audio(); // Must be defined in .cpp file
//     
//     void update( float dt );
//     
// private:
//     dny::audio_engine m_audio;
//     dny::audio_clip m_jump_sound;
//     dny::audio_clip m_coin_sound;
//     dny::audio_clip m_background_music;
// };
//
// In the .cpp file:
// #include "your_header.hpp"
//
// game_with_audio::game_with_audio(){
//     m_jump_sound = m_audio.load_audio( "assets/sounds/jump.wav" );
//     m_coin_sound = m_audio.load_audio( "assets/sounds/coin.wav" );
//     m_background_music = m_audio.load_audio( "assets/music/theme.mp3" );
//     
//     m_background_music.set_loop( true );
//     m_background_music.set_volume( 0.6f );
//     m_background_music.play();
//     
//     m_audio.set_master_volume( 0.8f );
// }
//
// game_with_audio::~game_with_audio() = default;
//
// void game_with_audio::update( float dt ){
//     m_audio.update();
// }
