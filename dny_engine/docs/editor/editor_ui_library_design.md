# dny::ui API Design and Implementation Plan

## Design goals
- Editor-only, lightweight, and deterministic.
- Internally retained state on each element.
- No event bus, callback tree, or global input routing.
- Editor code polls element state (`was_clicked`, `checked`, `text`, etc.).
- Renders directly with `dny_graphics.hpp` primitives.

## Namespace
All types live under `dny::ui`.

## Shared concepts
### `dny::ui::Element` (base class)
Common retained properties:
- `position`
- `size`
- `visible`
- `enabled`
- string `id` for lookup

Common API:
- `set_position`, `position`
- `set_size`, `size`
- `set_visible`, `visible`
- `set_enabled`, `enabled`
- `bounds`, `contains`
- `update(Mouse const&, Keyboard&)`
- `draw(surface<Color32>&, Font const&)`

## Required controls
### `Button`
- Label text
- Click state per frame via `was_clicked()`

### `TextBox`
- Read-only text display

### `InputTextBox`
- Editable text buffer
- Focus state
- Pulls characters from keyboard queue
- Access via `text()`

### `ListBox`
- Retained item list (`set_items`, `add_item`, `clear_items`)
- Selection with `selected_index`, `selected_item`
- `selection_changed()` for editor-side reactions

### `Panel`
- Visual/layout container
- Fixed position and fixed size (no floating, no resize)
- Stores child elements
- Draws background region
- Child lookup via `find_child(id)`
- Resolves radio-group exclusivity among children

### `RadioButton`
- Group name (`group()`)
- Selected state (`selected()`)
- `was_selected_this_frame()` for panel-level exclusivity resolution

### `CheckBox`
- Toggle on/off state (`checked()`)
- Frame state query `was_toggled()`

## Panel layout model
A panel stores `std::shared_ptr<Element>` children and iterates sequentially for update/draw.
The editor positions children explicitly (simple deterministic layout).

## State query model
Editor loop pattern:
```cpp
if( save_button->was_clicked() ){
    save_level();
}

if( snap_checkbox->checked() ){
    enable_grid_snap();
}
```

## Example usage in editor
```cpp
using namespace dny::ui;

Panel tools_panel{ "tools_panel", "Editor Tools", { 8, 8 }, { 240, 344 } };
auto save_button = tools_panel.emplace_child<Button>( "save", "Save Level", dny::vector2<std::int32_t>{ 16, 40 }, dny::dims2<std::int32_t>{ 120, 24 } );
auto name_input  = tools_panel.emplace_child<InputTextBox>( "name", "level_01", dny::vector2<std::int32_t>{ 16, 72 }, dny::dims2<std::int32_t>{ 200, 24 } );
auto snap_toggle = tools_panel.emplace_child<CheckBox>( "snap", "Grid Snap", dny::vector2<std::int32_t>{ 16, 104 }, dny::dims2<std::int32_t>{ 140, 18 } );

// Frame update
mouse.begin_frame();
keyboard.begin_frame();
tools_panel.update( mouse, keyboard );

if( save_button->was_clicked() ){
    save_level();
}
if( snap_toggle->was_toggled() ){
    set_grid_snap( snap_toggle->checked() );
}

// Frame draw
tools_panel.draw( canvas, font );
```

## Implementation plan
### Files to create
- `ui/element.hpp`, `ui/element.cpp`
- `ui/button.hpp`, `ui/button.cpp`
- `ui/textbox.hpp`, `ui/textbox.cpp`
- `ui/input_textbox.hpp`, `ui/input_textbox.cpp`
- `ui/listbox.hpp`, `ui/listbox.cpp`
- `ui/panel.hpp`, `ui/panel.cpp`
- `ui/checkbox.hpp`, `ui/checkbox.cpp`
- `ui/radiobutton.hpp`, `ui/radiobutton.cpp`
- `ui/dny_ui.hpp` (single include convenience header)

### Supporting utilities
- No new render layer.
- Use `draw_line`, `draw_rect`, `fill_rect`, `draw` (text), and `fill_circle` directly.
- No global input dispatcher; call each element/panel `update` directly in editor code.
