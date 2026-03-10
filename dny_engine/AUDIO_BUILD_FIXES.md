# Audio Module Build Fixes

## Issues Fixed

### 1. Incomplete Type Error with audio_clip::impl

**Problem:** The `audio_clip` class uses the PIMPL idiom with `std::unique_ptr<impl>`, but the compiler needs to see the complete definition of `impl` when generating destructors, move constructors, and move assignment operators.

**Solution:** 
- Explicitly declared destructor, move constructor, and move assignment operator in `dny_audio.hpp`
- Defined them in `dny_audio.cpp` after the complete definition of `impl`

Changes made to `dny_audio.hpp`:
```cpp
class audio_clip{
public:
    audio_clip() = default;
    ~audio_clip();  // Explicitly declared

    audio_clip( audio_clip const& ) = delete;
    audio_clip& operator=( audio_clip const& ) = delete;

    audio_clip( audio_clip&& ) noexcept;  // Explicitly declared
    audio_clip& operator=( audio_clip&& ) noexcept;  // Explicitly declared
    // ...
};
```

Changes made to `dny_audio.cpp`:
```cpp
struct audio_clip::impl{
    // ... complete definition ...
};

// Definitions AFTER impl is complete
audio_clip::~audio_clip() = default;
audio_clip::audio_clip( audio_clip&& other ) noexcept
    : m_impl( std::move( other.m_impl ) ) {}
audio_clip& audio_clip::operator=( audio_clip&& other ) noexcept{
    if( this != &other ){
        m_impl = std::move( other.m_impl );
    }
    return *this;
}
```

### 2. Game Class Destructor Issue - **This Was The Key Fix!**

**Problem:** The `Game` class in `game.hpp` had `audio_clip` members, which means its implicitly-generated destructor needed the complete type definition. When the destructor was generated inline in the header, it couldn't see the complete `audio_clip::impl` definition.

**Solution:**
- Moved `Game` constructor and destructor out-of-line by declaring them in the header
- Created `game.cpp` to define them
- Now the destructor is only instantiated in `game.cpp`, which can access the complete type definitions

This is the proper way to handle classes that contain PIMPL members!

Changes made to `game.hpp`:
```cpp
class Game{
public:
    Game( dny::platform& platform_ );  // Declared, not defined inline
    ~Game();  // Explicitly declared
    // ...
};
```

Created `game.cpp`:
```cpp
#include "game.hpp"

Game::Game( dny::platform& platform_ )
    : platform( platform_ ){
    // ... initialization ...
}

Game::~Game() = default;
```

### 3. Multiple Definition Linker Error

**Problem:** `clip_rect` function in `dny_graphics.hpp` was not marked as `inline`, causing multiply defined symbols when included in multiple translation units.

**Solution:** Added `inline` keyword:
```cpp
inline Rect<std::int32_t> clip_rect( Rect<std::int32_t>const& src_, Rect<std::int32_t> const& bounds_ ){
    // ...
}
```

## How to Use Audio in Game Class

To properly use audio members in the `Game` class:

1. Uncomment the audio members in `game.hpp`:
```cpp
dny::audio_engine audio;
dny::audio_clip jump_sound;
dny::audio_clip background_music;
```

2. The Game destructor is already defined in `game.cpp`, which will properly destroy the audio members.

3. Initialize audio in the Game constructor (in `game.cpp`):
```cpp
Game::Game( dny::platform& platform_ )
    : platform( platform_ ){
    // ... existing initialization ...
    
    // Load audio files
    jump_sound = audio.load_audio( "assets/sounds/jump.wav" );
    background_music = audio.load_audio( "assets/music/theme.mp3" );
    background_music.set_loop( true );
    background_music.play();
}
```

## Key Takeaway

When using the PIMPL idiom with `std::unique_ptr`, always:
1. Declare special member functions (destructor, move constructor, move assignment) in the header
2. Define them in the .cpp file where the `impl` struct is fully defined
3. **Any class that has members using PIMPL must also follow this pattern** - declare destructor in header, define in .cpp

This ensures the compiler sees the complete type when generating code that needs to destroy/move the objects.

## Project Notes

- Project uses **C++23** (stdcpp23)
- Splitting Game into header/source was essential, not optional
