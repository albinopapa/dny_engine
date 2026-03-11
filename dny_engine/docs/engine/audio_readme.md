# DNY Audio Module

A simple audio API for the dny_engine using Windows Media Foundation for loading and XAudio2 for playback.

## Files Created
- `dny_audio.hpp` - Public API header
- `dny_audio.cpp` - Implementation
- `dny_audio_example.hpp` - Usage examples
- `dny_audio_test.cpp` - Basic API test

## Features
- Load audio files (WAV, MP3, WMA, AAC - any format supported by Windows Media Foundation)
- Play, stop, pause, resume audio
- Looping audio for background music
- Per-clip volume control
- Master volume control
- Query playback state

## Quick Start

```cpp
#include "dny_audio.hpp"

// 1. Create an audio engine (typically as a member of your game class)
dny::audio_engine audio;

// 2. Load audio files
auto jump_sound = audio.load_audio( "sounds/jump.wav" );
auto bgm = audio.load_audio( "music/background.mp3" );

// 3. Play sounds
jump_sound.play();

// 4. Loop background music
bgm.set_loop( true );
bgm.set_volume( 0.6f );
bgm.play();

// 5. Control master volume
audio.set_master_volume( 0.8f );

// 6. Update (call once per frame)
audio.update();
```

## API Reference

### `audio_engine`
Main audio system manager. Create one instance per application.

**Methods:**
- `audio_clip load_audio(const std::filesystem::path& filename)` - Load an audio file
- `void set_master_volume(float volume)` - Set master volume (0.0 to 1.0)
- `float get_master_volume() const` - Get current master volume
- `void update()` - Update audio system (call once per frame)

### `audio_clip`
Represents a loaded audio file. Can be played multiple times.

**Methods:**
- `void play()` - Start or restart playback
- `void stop()` - Stop playback and reset position
- `void pause()` - Pause playback
- `void resume()` - Resume from pause
- `void set_loop(bool loop)` - Enable/disable looping
- `void set_volume(float volume)` - Set clip volume (0.0 to 1.0)
- `bool is_playing() const` - Check if currently playing
- `bool is_looping() const` - Check if looping is enabled
- `audio_state get_state() const` - Get current state (stopped/playing/paused)

### `audio_state` enum
- `stopped` - Not playing
- `playing` - Currently playing
- `paused` - Paused

## Integration Example

```cpp
class game{
public:
    game(){
        // Load audio during initialization
        m_jump_sound = m_audio.load_audio( "assets/jump.wav" );
        m_bgm = m_audio.load_audio( "assets/music.mp3" );
        
        m_bgm.set_loop( true );
        m_bgm.play();
    }
    
    void update( float dt ){
        m_audio.update();
        
        // Play sound on events
        if( player_jumped ){
            m_jump_sound.play();
        }
    }
    
private:
    dny::audio_engine m_audio;
    dny::audio_clip m_jump_sound;
    dny::audio_clip m_bgm;
};
```

## Technical Details

### Dependencies
- Windows Media Foundation (mfplat.lib, mfreadwrite.lib, mfuuid.lib)
- XAudio2 (xaudio2.lib)
- Windows Runtime Library (wrl)

### Design Decisions
1. **PIMPL idiom**: Implementation details are hidden to avoid exposing Windows headers
2. **Move-only semantics**: `audio_clip` uses move semantics to prevent accidental copies
3. **Simple API**: Focused on game audio needs - play, stop, loop
4. **Automatic playback**: XAudio2 handles audio processing automatically in background threads

### Performance Considerations
- Load audio files during initialization, not during gameplay
- Reuse `audio_clip` objects - they can be played multiple times
- Audio playback happens on separate threads managed by XAudio2
- Minimal overhead in the `update()` call

### Supported Audio Formats
Any format supported by Windows Media Foundation:
- WAV (PCM, ADPCM)
- MP3
- WMA
- AAC
- FLAC (Windows 10+)

### Limitations
- Windows-only (uses Windows Media Foundation and XAudio2)
- No 3D audio positioning (can be added if needed)
- No real-time effects (can be added using XAudio2 effects)
- No streaming (all audio loaded into memory)

## Troubleshooting

**Audio not playing:**
- Verify file path is correct
- Check file format is supported
- Ensure volume is not set to 0
- Check `is_playing()` returns true after calling `play()`

**Compilation errors:**
- Ensure Windows SDK is installed
- Link against required libraries (mfplat.lib, mfreadwrite.lib, mfuuid.lib, xaudio2.lib)
- Use C++14 or later

**Runtime crashes:**
- Ensure COM is initialized (done automatically in the implementation)
- Don't access audio_clip after it's been moved
- Check audio files exist before loading

## Future Enhancements
The API is designed to be extensible. Possible additions:
- 3D spatial audio
- Audio streaming for large files
- Real-time effects (reverb, echo, etc.)
- Audio mixing/crossfading
- Pitch control
- Audio capture/recording
