# Input Subsystem API Design

## Public API

### Enums
- `dny::Key`
  - `Space`, `A`, `D`, `E`, `Q`, `Escape`, arrows.
- `dny::MouseButton`
  - `Left`, `Right`, `Middle`.
- `dny::GamepadButton`
  - `A`, `B`, `X`, `Y`, shoulders, dpad, `Back`, `Start`.

### `dny::Keyboard`
- `void begin_frame() noexcept`
- `bool handle_message(UINT msg, WPARAM wparam) noexcept`
- `void clear() noexcept`
- `bool is_pressed(Key key) const noexcept`
- `bool is_held(Key key) const noexcept`
- `bool is_released(Key key) const noexcept`

Implementation model:
- stores `current[256]` and `previous[256]`
- `begin_frame()` copies `current` to `previous`
- Win32 key messages mutate `current`
- transitions are derived from `current/previous`

### `dny::Mouse`
- `void begin_frame() noexcept`
- `bool handle_message(UINT msg, WPARAM wparam, LPARAM lparam) noexcept`
- `void clear() noexcept`
- `vector2<int32_t> position() const noexcept`
- `vector2<int32_t> delta() const noexcept`
- `int32_t wheel_delta() const noexcept`
- `bool is_pressed(MouseButton button) const noexcept`
- `bool is_held(MouseButton button) const noexcept`
- `bool is_released(MouseButton button) const noexcept`

Implementation model:
- button states use current/previous arrays
- movement delta and wheel reset in `begin_frame()` and accumulate from Win32 messages

### `dny::Gamepad`
- `explicit Gamepad(uint32_t user_index = 0) noexcept`
- `void begin_frame() noexcept`
- `void poll() noexcept`
- `bool is_connected() const noexcept`
- `bool is_pressed(GamepadButton) const noexcept`
- `bool is_held(GamepadButton) const noexcept`
- `bool is_released(GamepadButton) const noexcept`
- analog accessors for triggers and sticks (normalized to `[-1, 1]` / `[0,1]`)

Implementation model:
- polls `XInputGetState` once per frame
- stores `current` and `previous` `XINPUT_STATE`

### `dny::Input` (high-level binding layer)
- ctor references devices (does not own device state)
- bind methods:
  - `bind(std::string action, Key)`
  - `bind(std::string action, MouseButton)`
  - `bind(std::string action, GamepadButton)`
- query methods:
  - `is_pressed(std::string_view action) const`
  - `is_held(std::string_view action) const`
  - `is_released(std::string_view action) const`

Internal binding representation:
- `unordered_map<string, variant<Key, MouseButton, GamepadButton>>`

## Platform Integration
- `dny::platform` owns:
  - `Keyboard m_keyboard`
  - `Mouse m_mouse`
  - `Gamepad m_gamepad`
  - `Input m_input`
- frame flow in `process_message_pump()`:
  1. `begin_frame()` for keyboard/mouse/gamepad
  2. process Win32 messages
  3. `gamepad.poll()`
- `message_proc` routes messages:
  - keyboard messages -> `m_keyboard.handle_message`
  - mouse messages -> `m_mouse.handle_message`

## Example Usage
```cpp
auto& input = platform.get_input();
input.bind("jump", dny::Key::Space);
input.bind("fire", dny::MouseButton::Left);
input.bind("dash", dny::GamepadButton::A);

if (input.is_pressed("jump")) { /* ... */ }
if (input.is_held("fire")) { /* ... */ }
if (input.is_released("dash")) { /* ... */ }
```

## Implementation Task List
1. Define enums for key/mouse/gamepad controls.
2. Implement `Keyboard` (Win32 key messages + transition states).
3. Implement `Mouse` (position, delta, buttons, wheel, transitions).
4. Implement `Gamepad` (XInput poll + digital/analog queries).
5. Implement `Input` binding layer using action -> variant binding map.
6. Integrate devices and frame lifecycle into `dny::platform`.
7. Refactor game usage to bind/query actions through `dny::Input`.
8. Update project files to compile new input source files.
