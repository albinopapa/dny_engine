# Level Editor Refactor Assessment

## Scope Reviewed
- `dny_engine/apps/editor/include/*`
- `dny_engine/apps/editor/src/*`

## High-Level Status
The editor code is currently in a **partially migrated** state. There are useful reusable model/types, but the UI/editor flow is split between old APIs and newer engine APIs and will not be stable until interfaces are unified.

---

## Reusable As-Is (or with minimal cleanup)

### Data model and domain types
- `dny_Definitions.hpp`
  - `TileDef`, `PlatformDef`, `EntityDef`, `TriggerDef` are good reusable domain structures.
  - Equality operators on IDs are practical for editor workflows.
- `dny_Level.hpp`
  - `Level` aggregate and `LevelSerializer` API shape are reusable.
  - Serialization boundary is in the right place.
- `dny_TriggerCategory.hpp`
  - Trigger enums + conversion helpers are reusable and clear.
- `dny_TileCategory.hpp`
  - Tile category enum and conversion helpers are reusable (with small fix noted below).
- `dny_IAppState.hpp`
  - Reusable generic app-state contract.

### Editor architecture concepts worth keeping
- `IMode` + nested mode classes (`EditorMode`, `SaveMode`, etc.)
  - Mode/state-machine approach is a good fit for modal editor interactions.
- `EditorCamera`
  - Orthographic camera model and cursor-centered zoom behavior are good reusable ideas.
- `TileMapView` / `TilePalette`
  - Separation of map viewport and palette panel is good and should be retained.

### Utility logic
- `dny_Filesystem.hpp`
  - Filename validation helpers are useful and reusable (move to shared utility if needed).

---

## Needs Change (priority)

## 1) API mismatch between headers and implementation (critical)
There are compile-level inconsistencies between `include` and `src`:
- `IMode` requires `handle_mouse(Input const&)` / `handle_keyboard(Input const&)`, but some mode headers declare parameterless overrides (`LoadMode`, `ResizeMode`, etc.).
- `LevelEditor.hpp` comments out core members (`m_layout`, `m_view`, `m_palette`, `m_mode`, `m_tilemap`, `m_tileset_sprites`), while `dny_LevelEditor.cpp` actively uses them.
- `EditorCamera` in header uses `ortho_scale`; implementation uses `cam.zoom` and `screen_to_world(...)` that do not exist in the header.
- `TilePalette` stores `std::array<std::int32_t, 8>` tile IDs, but exposes methods returning `Tile const&`.

**Recommendation:** First refactor pass should be a **contract alignment pass**: make header declarations and `.cpp` implementations consistent before feature work.

## 2) Legacy framework references mixed with new engine types
Several headers still include legacy paths/types (e.g., `../../dny_framework/include/...`, `Mouse`, `Point`, `RectF`, `vector2<float>`) while newer code uses engine types (`Input`, `vector2`, `Rect<float>`, `ui/*`).

**Recommendation:** Choose one stack (prefer current engine modules under `engine/include`) and remove legacy dependencies from editor headers.

## 3) C++14 compatibility violations
Workspace is C++14, but editor code uses later features:
- `std::format`
- `concept Number`
- `constexpr` functions constructing/using `std::string` (`dny_Filesystem.hpp`)

**Recommendation:** Add a portability pass:
- Replace `std::format` with stream/string-builder helpers.
- Replace concepts with `std::enable_if`/traits or remove if unused.
- Remove invalid `constexpr` on string-based runtime helpers.

## 4) Tile model split (`TileMap` vs `LevelDocument`)
`LevelDocument` contains `tilemap`, but `LevelEditor.cpp` accesses `m_tilemap` directly. This creates two sources of truth.

**Recommendation:** Keep a single authoritative map (`m_document.tilemap`) and route all map operations through that.

## 5) Incomplete rendering and placement logic
Large blocks are commented out in:
- `TileMapView::render`
- `TilePalette::render`
- `LevelEditor::place_tile`
- `load_tileset_sprites`

**Recommendation:** Re-enable in small vertical slices:
1. Draw tile IDs as colored debug blocks.
2. Enable placement with undo-safe write path.
3. Then reintroduce texture-backed rendering.

## 6) Safety/logic issues
- `category_from_int` assert currently excludes 0 (`num > 0`) but switch handles 0.
- `TilePalette::tile_at` has `assert(index >= 0 && ...)` where `index` is unsigned (`std::size_t`), so first check is meaningless.
- `tile_count()` should be `const`.

**Recommendation:** Fix these immediately; they are low-risk correctness improvements.

---

## Suggested Refactor Plan

## Phase 1: Stabilize interfaces (no feature additions)
- Align all mode signatures to `IMode`.
- Unify `LevelEditor.hpp` members with actual usage in `.cpp`.
- Resolve `EditorCamera` API mismatch.
- Ensure code compiles under C++14 baseline.

## Phase 2: Establish clean editor core
- Make `LevelDocument` the only state owner.
- Keep `TileMapView` and `TilePalette` as pure UI/views over document state.
- Introduce thin command functions (`place_tile`, `erase_tile`, `resize_map`).

## Phase 3: Re-enable functionality incrementally
- Basic map render + brush placement.
- Palette selection and active-tile preview.
- Save/load mode wiring.
- Texture mapping + special tile rules (single player spawn, etc.).

## Phase 4: Mode and dialog cleanup
- Extract common dialog behavior (OK/Cancel/input focus/list handling) to shared helpers.
- Keep each mode focused only on mode-specific validation/actions.

---

## What to Reuse Directly in Upcoming Work
- Domain structs (`TileDef`, `PlatformDef`, `EntityDef`, `TriggerDef`)
- `Level`/`LevelSerializer` boundary
- Mode-based editor interaction design
- `EditorCamera` behavior concept
- `TileMapView` and `TilePalette` separation

## What to Rewrite or Heavily Edit First
- `LevelEditor.hpp` + `dny_LevelEditor.cpp` contract alignment
- Legacy include/type usage in mode headers
- `IMode` compliance across all mode classes
- C++14-incompatible language/library usage
- Tile source-of-truth (`m_document.tilemap` vs `m_tilemap`)

---

## Immediate Next Step
Start with a **compilation-first refactor branch** that only resolves signature/type/state mismatches, then layer feature restoration after the core editor loop compiles and runs cleanly.