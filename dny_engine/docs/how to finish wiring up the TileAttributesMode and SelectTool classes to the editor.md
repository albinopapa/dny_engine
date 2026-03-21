# How to finish wiring up the `TileAttributesMode` and `SelectTool` classes to the editor

## Executive summary

The editor has only a skeleton for `TileAttributesMode` and only a forward declaration for `SelectTool`.

At the moment:

- `TileAttributesMode` has a constructor comment block describing the intended fields, but it does not build any UI, does not track which tile is being edited, and does not read or write any data back into `LevelDocument`.
- `SelectTool` is declared as a nested class in `LevelEditor`, but there is no header, source file, or tool-switching path that instantiates it.
- The current editor interaction path still routes left-clicks in the workspace through `LevelEditor::handle_workspace()` and `place_tile()`, so clicking a tile paints it instead of selecting it.

That means the remaining work is not just “wire up the popup”; it also requires adding the editor state needed to know which tile is selected, mapping tile categories to the correct definition data, and providing a way for the select tool to become the active tool.

## What is already present

### `TileAttributesMode`

The class exists and derives from `basic_mode`, so it already inherits a `ui::Panel` and can participate in the existing mode stack. The constructor comment outlines the intended fields for several categories, and the class has placeholders for scrolling and per-attribute subpanels. However, `update()` and `render()` are effectively empty from a feature perspective.

### Tile/category data model

The editor already has the category model and related definition structures needed to decide what attributes should be shown:

- `TileDef` for base tile attributes such as `name`, `texture_name`, `category`, `friction`, and `damage`.
- `PlatformDef` for platform tiles.
- `EntityDef` for spawner tiles.
- `TriggerDef` for trigger tiles.
- `TileCategory` values for `Empty`, `Platform`, `Trigger`, `Spawner`, `Liquid`, `Decoration`, and `Solid`.

### Existing editor infrastructure you can reuse

- `basic_mode` already provides a `ui::Panel` and lifecycle via `update()` / `render()`.
- The level editor already has a mode stack via `transition_mode()`.
- Existing modal implementations (`LoadMode`, `ResizeMode`, `TextureSelectMode`, `TilePaletteMode`) show the expected pattern for constructing controls, calling `m_panel.update(...)`, and drawing with `m_panel.draw(...)`.
- `ui::Dropdown` and `ui::InputTextBox` already exist and are usable for a category picker and numeric/text fields.

## Main gaps that still need to be closed for `TileAttributesMode`

## 1. Add selected-tile context to the editor

`TileAttributesMode` currently has no way to know which tile it is editing. Before the modal can work, the editor needs a stable selection context.

### What to add

Add editor state for the currently selected tile, for example:

- selected tile coordinate: `std::optional<vector2<std::int32_t>>`
- optionally a cached pointer/reference helper to the selected `Tile`

### Why it is necessary

Without that selection state, the mode cannot:

- load the tile’s current `definition_id`
- discover the tile’s current category from `m_document.tile_defs`
- find related `platform_id`, `entity_id`, or `trigger_id`
- write changes back to the correct tile or definition record

## 2. Decide what the modal edits: tile definition vs tile instance

This is the most important design choice still unresolved in the current code.

The existing data model mixes:

- **definition-level data** in `TileDef` (`category`, `friction`, `damage`, `texture_name`)
- **instance-linked data** in `Tile` (`platform_id`, `entity_id`, `trigger_id`)
- **referenced records** in `PlatformDef`, `EntityDef`, and `TriggerDef`

### Recommended interpretation

The modal should edit:

- the selected tile’s `TileDef` fields for categories and shared tile properties
- the selected tile’s linked record (`PlatformDef`, `EntityDef`, or `TriggerDef`) when the category requires one

### Consequence

Changing category inside the modal cannot be just a dropdown UI action; it must also update the underlying linkage:

- `Platform` must ensure a valid `platform_id`
- `Spawner` must ensure a valid `entity_id`
- `Trigger` must ensure a valid `trigger_id`
- moving away from those categories should clear or normalize the unused IDs

## 3. Build the modal UI dynamically from category

The requested context-aware behavior does not exist yet.

### Recommended implementation shape

Give `TileAttributesMode` a small set of persistent controls:

- category dropdown
- accept/apply button
- cancel button
- optional title or selected-tile label

Then rebuild the category-specific controls whenever the dropdown changes or when the modal opens.

### Suggested internal helpers

Add helpers such as:

- `load_selected_tile_context()`
- `populate_category_dropdown()`
- `rebuild_attribute_controls(TileCategory category)`
- `load_values_into_controls()`
- `apply_changes()`
- `find_or_create_platform_def(...)`
- `find_or_create_entity_def(...)`
- `find_or_create_trigger_def(...)`

### Category-to-control mapping

A practical first pass would be:

- **Empty**
  - no extra controls
- **Solid**
  - friction (`InputTextBox`)
  - damage (`InputTextBox`) if desired because `TileDef` supports it
- **Liquid**
  - friction (`InputTextBox`)
  - damage (`InputTextBox`)
- **Decoration**
  - texture name or texture selector hook
- **Platform**
  - texture name / texture picker entry point
  - start.x / start.y
  - end.x / end.y
  - width / height
  - speed
  - moveable flag (this likely needs a checkbox or dropdown because the current UI set does not include a dedicated boolean editor in the requested plan)
- **Spawner**
  - entity name
  - spawn x / spawn y
  - width / height
  - max speed
  - damage
  - health
- **Trigger**
  - this is the hardest category because `TriggerDef` is composed of nested `TriggerCondition` and `TriggerAction` types rather than plain scalar fields; see “Missing engine/editor features” below

## 4. Parse and validate textbox values before applying

`ui::InputTextBox` is only a plain text collector. It does not provide numeric parsing, typed validation, range checks, or formatting.

### Work still needed

For each numeric field, the modal must:

- read `textbox->text()`
- convert with a parser such as `std::from_chars`, `std::stof`, or the project’s existing `PropertyParser::string_to_number<T>` if you want consistency with serialization code
- reject invalid input or keep the old value
- ideally show validation feedback in the panel

### Recommended minimum behavior

- On Apply: validate every field
- If any field is invalid, do not close the mode
- Show a small validation label in the panel explaining the first invalid field

## 5. Write changes back into the correct data structures

Once controls exist, the mode still needs logic to commit changes.

### For base tile-definition fields

Update the selected tile’s definition record in `m_document.tile_defs[ tile.definition_id ]`:

- `category`
- `friction`
- `damage`
- `texture_name` if exposed in the modal

### For category-linked records

Depending on category:

- `Platform`: update or create the referenced `PlatformDef`
- `Spawner`: update or create the referenced `EntityDef`
- `Trigger`: update or create the referenced `TriggerDef`

### Additional editor bookkeeping

After applying changes, the editor should:

- mark `m_dirty = true`
- close the mode (`m_state = State::Done`)
- optionally refresh any cached display state if the tile preview depends on changed definitions

## 6. Render and update the panel properly

Even after controls are added, the modal still needs the standard mode behavior used elsewhere.

### Minimum required behavior

In `update()`:

- call `m_panel.update(mouse_, keyboard_)`
- detect dropdown selection changes
- rebuild context-aware controls when category changes
- handle Apply / Cancel buttons
- close on `Escape`

In `render()`:

- call `m_panel.draw(renderer_, m_parent.m_font)`
- draw any custom text such as selected tile position, validation errors, or texture preview

## Main gaps that still need to be closed for `SelectTool`

## 1. Create the class for real

`SelectTool` is only forward-declared. It still needs:

- a header, likely `engine/include/dny/editor/select_tool.hpp`
- a source file, likely `engine/src/editor/select_tool.cpp`
- inclusion in `level_editor.cpp`
- construction from a tool switch path

### Recommended class shape

Mirror `PaintTool`:

- store `LevelEditor& m_parent`
- implement `update(Mouse const&, Keyboard&)`

No rendering is required for a minimal first version.

## 2. Stop the workspace click path from always painting

Right now, a left click in the workspace still leads to painting in `LevelEditor::handle_workspace()`.

That means even if `SelectTool::update()` is added, the editor may still paint on the same click unless responsibility is clearly separated.

### Recommended fix

Make tool handling authoritative.

One clean option:

- when there is an active tool, let the active tool fully own left-click workspace behavior
- keep middle-mouse panning and wheel zoom in `handle_workspace()`
- remove the unconditional `place_tile(mouse)` call from `handle_workspace()`

That prevents the select tool from fighting the paint path.

## 3. Implement tile hit-testing in `SelectTool`

The tool should use the same conversion already used by `PaintTool` and editor placement:

- call `m_parent.m_tilemap_view.screen_to_tile_index(mouse.position(), m_parent.m_camera)`
- reject out-of-bounds results
- save the selected coordinate into the editor

## 4. Push `TileAttributesMode` onto the mode stack

Once a tile is hit, `SelectTool` should create the modal and push it using `transition_mode()`.

### Needed constructor change

`TileAttributesMode` currently only takes `(LevelEditor&, Rect<int32_t> const&)`.

To make selection explicit and avoid hidden dependencies, change it to take the selected tile coordinate too, for example:

```cpp
TileAttributesMode(LevelEditor& parent, vector2<std::int32_t> tile_index, Rect<std::int32_t> const& area) noexcept;
```

That is the cleanest way for the modal to know what it is editing.

## 5. Add a way to activate the select tool

The editor currently initializes `m_active_tool` to `PaintTool` and has a Tools button placeholder, but no actual tool switch UI.

### Minimum viable path

Provide at least one of these:

- keyboard toggle between paint/select
- a simple tools modal
- a sidebar button pair for Paint and Select

Without that, `SelectTool` can exist but is unreachable.

## Missing engine/editor features that may block a complete implementation

These are the feature gaps I would explicitly call out before coding, because they affect scope.

## 1. No persistent selected-tile state in `LevelEditor`

This is required for both the select workflow and for reopening or reflecting the current selection.

## 2. No existing `SelectTool` implementation or tool switching UI

The editor has tool abstractions, but only `PaintTool` is implemented and wired.

## 3. No helper API for ID allocation / lookup for linked records

The code currently assigns placeholder IDs (`1`) for platform, trigger, and entity linkage during tile placement.

To make the attributes modal safe and correct, the editor needs helper functions such as:

- allocate next platform ID
- allocate next entity ID
- allocate next trigger ID
- find definition record by ID
- create record if missing

Without those helpers, category changes and edits to linked records will be fragile.

## 4. Trigger editing is not yet UI-friendly

`TriggerDef` is structurally more complex than the other definitions because it contains nested condition/action data. A full trigger editor will likely need more UI than just text boxes:

- dropdowns for condition/action/target enums
- text boxes for IDs
- a better editor for region rectangles
- possibly entity/platform lookup assistance

If the goal is to finish the first usable version quickly, I would ship `TileAttributesMode` with full support for `Solid`, `Liquid`, `Platform`, and `Spawner`, then either:

- disable `Trigger` editing initially, or
- expose only a minimal raw-field version for trigger IDs/types

## 5. No dedicated boolean editor in the currently referenced UI plan

`PlatformDef::is_moveable` needs either:

- a checkbox/radio button,
- a dropdown with `true` / `false`, or
- a text field fallback.

A dropdown is probably the quickest solution if no checkbox already exists in the UI library.

## 6. No texture-picker integration from `TileAttributesMode`

The comments say platform and spawner records should use `TextureSelectMode`, but there is not yet a handoff contract for that flow from inside another modal.

This needs a design decision:

- either `TileAttributesMode` uses plain text for `texture_name` initially,
- or it launches `TextureSelectMode` and then refreshes its own field after that mode returns.

The plain text route is much easier for the first version.

## Recommended implementation order

## Phase 1: make selection work end-to-end

1. Add selected tile state to `LevelEditor`.
2. Implement `SelectTool`.
3. Add a way to activate `SelectTool`.
4. Make workspace left-click behavior owned by the active tool instead of always painting.
5. Pass selected tile coordinates into `TileAttributesMode`.

## Phase 2: ship a minimal but functional `TileAttributesMode`

1. Add category dropdown.
2. Add Apply / Cancel buttons.
3. Rebuild controls based on selected category.
4. Support `Solid` and `Liquid` first using `TileDef` fields only.
5. Add validation and writeback.

## Phase 3: add linked-record categories

1. Add helper functions to find/create `PlatformDef` and `EntityDef` records.
2. Support `Platform` editing.
3. Support `Spawner` editing.
4. Mark editor dirty after changes.

## Phase 4: finish harder categories

1. Decide whether trigger editing is in scope for the first release.
2. If yes, build enum dropdowns and region editors for `TriggerDef`.
3. Add optional texture-picker integration.

## Concrete “definition structure” mapping to use in the modal

If you want the attributes shown to correspond to the tile category and associated definition structures, the mapping should be:

- `Empty`, `Solid`, `Liquid`, `Decoration` -> `TileDef`
- `Platform` -> `TileDef` + `PlatformDef`
- `Spawner` -> `TileDef` + `EntityDef`
- `Trigger` -> `TileDef` + `TriggerDef`

That keeps the category dropdown as the top-level switch and makes the rest of the form context aware.

## Bottom line

To finish wiring this up, the minimum real work is:

1. implement and activate `SelectTool`
2. track the selected tile in `LevelEditor`
3. make `TileAttributesMode` know which tile it edits
4. dynamically build UI controls from `TileCategory`
5. parse/validate textbox data
6. update `TileDef` and linked definition records safely
7. add missing helper APIs for ID allocation and linked-record lookup

The biggest blockers are not rendering the modal itself, but the missing selection state, the missing `SelectTool` implementation, and the lack of helper infrastructure for category-linked records.
