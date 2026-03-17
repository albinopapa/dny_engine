# Type Migration and Include Path Updates

## Summary
Migrated editor code from legacy framework types to new dny_engine types and updated include paths after moving `rectangle.hpp` from `core/` to `utilities/`.

## Include Path Changes

### Updated Paths
- `core/rectangle.hpp` ? `utilities/rectangle.hpp`
- Added `math/math.hpp` where needed for vector types

### Files Updated
- `dny_LevelEditor.hpp`
- `dny_TileMap.hpp`
- `dny_TriggerCategory.hpp`
- `dny_SaveMode.hpp`
- `dny_ResizeMode.hpp`
- `dny_LoadMode.hpp`
- `dny_SaveBeforeExitMode.hpp`
- `dny_TextureSelectMode.hpp`

## Type Conversions

### Type Mapping
- `Point` ? `vector2<std::int32_t>`
- `Vec2f` ? `vector2<float>`
- `RectF` ? `Rect<float>`
- `UIButton` ? `ui::Button`
- `UIInputBox` / `UITextBox` ? `ui::InputTextBox`
- `UIListBox<T>` ? `ui::ListBox<T>`
- `UIDropDownBox` ? `ui::DropDownBox`

### Static Center Points Converted
All mode classes previously used `m_editor_area.center()` with legacy types. Now use explicit `vector2<std::int32_t>{ 640, 360 }` for constexpr center calculation.

## Implementation File Updates

### Modal Class Constructors
Updated all mode constructors to:
- Use `Font::measure_text()` instead of `Graphics::calculate_rect()`
- Construct UI elements with explicit `Rect<std::int32_t>` bounds
- Use new type conversions throughout

### Event Handling
Updated all `handle_mouse()` and `handle_keyboard()` methods to:
- Accept `Input const& input_` parameter (matching `IMode` interface)
- Use `input_.mouse()` / `input_.keyboard()` instead of singletons
- Use enum values like `MouseButton::Left`, `Key::Enter` instead of raw VK codes

### Files Modified
- `dny_ResizeMode.cpp`
- `dny_LoadMode.cpp`
- `dny_SaveMode.cpp`
- `dny_SaveBeforeExitMode.cpp`
- `dny_TextureSelectMode.cpp`

## Notes and TODOs

### Temporarily Disabled
Some functionality was commented out pending full UI/renderer integration:
- `Graphics::instance().draw_rectangle()` calls (need renderer2d replacement)
- Character input handling (keyboard character queue API needs update)
- `TileMapSerializer` calls (serializer not yet implemented)
- Current filename tracking in `LevelEditor` (member was commented out)

### Known Issues
- `TextureSelectMode::handle_listbox()` still uses old `Mouse::Event` type
- Mouse wheel handling needs `Input` API wrapper
- UI element construction is verbose with explicit rect construction

## Next Steps
1. Implement renderer2d drawing for dialog backgrounds
2. Add keyboard character input API to new Input system
3. Restore TileMapSerializer functionality
4. Add current filename tracking back to LevelEditor
5. Consider helper functions for UI element rect construction
6. Convert handle_listbox to use Input instead of Mouse::Event
