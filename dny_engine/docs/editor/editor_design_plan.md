# In-Game Level Editor Design Plan

## Overview
This document defines a maintainable implementation plan for an in-game level editor that runs inside the existing `dny_engine` runtime.

The editor is designed as a runtime-integrated toolset (not a separate app) with clear boundaries between:
- **Runtime systems** (rendering, scene update, input backend, asset loading)
- **Editor systems** (tools, UI panels, selection/manipulation, command history, editor metadata)

### Objectives
- Provide efficient scene authoring and iteration inside the engine.
- Preserve runtime stability by isolating editor-only logic and data.
- Support reversible authoring actions with undo/redo.
- Ensure scene persistence supports both runtime loading and editor metadata.

### Scope Constraints
- Planning/design only (no implementation in this phase).
- Editor executes in the same process and frame loop as the engine.
- Architecture favors extensibility (additional tools/panels can be added incrementally).

---

## Architecture

### 1) High-Level Editor Subsystems
1. **Editor Host**
   - Lifecycle entry point for all editor systems.
   - Coordinates initialization, frame update, and shutdown.

2. **Editor Context**
   - Shared state store (active scene, current tool, selected entities, settings, history).
   - Provides service accessors to runtime systems.

3. **Editor UI Layer**
   - Hierarchy panel, Inspector panel, Asset Browser panel, Viewport toolbar/overlay.
   - Routes UI actions into command-based scene operations.

4. **Scene Interaction Layer**
   - Handles picking, selection rules, transform manipulation, object placement.
   - Owns tool mode transitions and tool-specific state.

5. **Command & History Layer**
   - Encapsulates all mutating edits as commands.
   - Provides undo/redo and command coalescing for drag operations.

6. **Serialization Layer**
   - Saves/loads scene data and editor metadata.
   - Versioned format for forward compatibility.

7. **Debug/Gizmo Rendering Layer**
   - Draws selection outlines, transform gizmos, guides, and overlays.

### 2) Runtime/Editor Interaction Model
- Runtime remains authoritative for:
  - Scene ownership/storage
  - Render backend
  - Low-level input backend
  - Asset loading and resource handles
- Editor integrates by:
  - Registering per-frame hooks (`tick`, `render_overlay`)
  - Consuming relevant input before gameplay controllers
  - Issuing explicit scene-edit commands via a controlled API

### 3) Editor Execution Modes
- **Edit Mode**
  - Authoring enabled.
  - Simulation paused or step-driven.
- **Play-In-Editor (PIE) Mode**
  - Runtime simulation runs with an isolated scene state.
  - Exiting PIE restores pre-PIE authoring state.

---

## File Layout

```text
dny_engine/
  editor/
    editor_module.hpp
    editor_module.cpp

    editor_context.hpp
    editor_context.cpp

    editor_mode_controller.hpp
    editor_mode_controller.cpp

    editor_input_router.hpp
    editor_input_router.cpp

    editor_camera.hpp
    editor_camera.cpp

    editor_ui.hpp
    editor_ui.cpp

    panels/
      hierarchy_panel.hpp
      hierarchy_panel.cpp
      inspector_panel.hpp
      inspector_panel.cpp
      viewport_panel.hpp
      viewport_panel.cpp
      asset_browser_panel.hpp
      asset_browser_panel.cpp

    tools/
      tool_base.hpp
      selection_tool.hpp
      selection_tool.cpp
      transform_tool.hpp
      transform_tool.cpp
      placement_tool.hpp
      placement_tool.cpp
      camera_nav_tool.hpp
      camera_nav_tool.cpp

    interaction/
      ray_picking.hpp
      ray_picking.cpp
      selection_system.hpp
      selection_system.cpp
      gizmo_system.hpp
      gizmo_system.cpp

    commands/
      command.hpp
      command_history.hpp
      command_history.cpp
      cmd_create_entity.hpp
      cmd_create_entity.cpp
      cmd_delete_entity.hpp
      cmd_delete_entity.cpp
      cmd_set_transform.hpp
      cmd_set_transform.cpp
      cmd_reparent_entity.hpp
      cmd_reparent_entity.cpp
      cmd_set_component_property.hpp
      cmd_set_component_property.cpp

    serialization/
      editor_scene_serializer.hpp
      editor_scene_serializer.cpp
      editor_metadata_serializer.hpp
      editor_metadata_serializer.cpp

    debug/
      editor_debug_draw.hpp
      editor_debug_draw.cpp

    editor_types.hpp
```

---

## Core Classes

### `EditorModule`
**Purpose**
- Top-level editor orchestrator.

**Responsibilities**
- Create/destroy editor subsystems.
- Call subsystem update/render in deterministic order.
- Expose mode toggles and global editor enable state.

**Key Methods**
- `bool initialize(EngineServices& services)`
- `void shutdown()`
- `void tick(float dt)`
- `void render_overlay()`
- `void set_enabled(bool enabled)`

### `EditorContext`
**Purpose**
- Shared editor state and runtime service access.

**Responsibilities**
- Hold active scene handle, selection set, tool state, settings.
- Hold pointers/references to runtime interfaces.

**Key Methods**
- `SceneHandle active_scene() const`
- `SelectionSet& selection()`
- `EditorToolState& tool_state()`
- `CommandHistory& history()`

### `EditorModeController`
**Purpose**
- Manage Edit ↔ PIE transitions.

**Responsibilities**
- Snapshot and restore scene/editor state boundaries.
- Guard unsafe operations while PIE is active.

**Key Methods**
- `void enter_edit_mode()`
- `void enter_play_mode()`
- `bool is_in_play_mode() const`

### `EditorInputRouter`
**Purpose**
- Resolve input ownership between UI, tools, and gameplay.

**Responsibilities**
- Prioritized dispatch (UI first, then active tool, then gameplay).
- Shortcut mapping and rebinding support.

**Key Methods**
- `void process_input(const InputState&)`
- `bool is_input_captured() const`
- `void bind_shortcut(EditorAction, KeyChord)`

### `SelectionSystem`
**Purpose**
- Maintain object selection state.

**Responsibilities**
- Single/multi selection, toggle/add/remove behaviors.
- Marquee and click-pick selection entry points.

**Key Methods**
- `void clear()`
- `void select(EntityId id, SelectionFlags flags)`
- `bool contains(EntityId id) const`

### `TransformTool`
**Purpose**
- Translate/rotate/scale selected entities using gizmos.

**Responsibilities**
- Local/world modes, snapping, pivot handling.
- Start/update/end drag gestures and command emission.

**Key Methods**
- `void set_mode(TransformMode mode)`
- `void begin_drag(const PickHit&)`
- `void update_drag(const Ray&)`
- `void end_drag()`

### `PlacementTool`
**Purpose**
- Place assets/primitives into the scene.

**Responsibilities**
- Placement preview, snapping to grid/surface.
- Instantiate entity with default components.

**Key Methods**
- `EntityId begin_place(AssetId asset)`
- `EntityId place_at(const Transform&)`
- `void cancel_place()`

### `CommandHistory`
**Purpose**
- Undo/redo infrastructure.

**Responsibilities**
- Execute commands and push onto undo stack.
- Redo stack management and command coalescing.

**Key Methods**
- `void execute(std::unique_ptr<IEditorCommand>)`
- `void undo()`
- `void redo()`
- `bool can_undo() const`
- `bool can_redo() const`

### `EditorSceneSerializer`
**Purpose**
- Persist and load authored data.

**Responsibilities**
- Serialize runtime scene-relevant entity/component data.
- Serialize editor-only metadata in sidecar/embedded block.

**Key Methods**
- `bool save_scene(const Scene&, const SaveOptions&)`
- `bool load_scene(Scene&, const LoadOptions&)`
- `uint32_t schema_version() const`

---

## Editor Systems

### Scene Interaction
- Viewport interactions (hover, click, drag, context menu).
- Hierarchy interactions (rename, reparent, reorder with validation).
- Inspector interactions (property edits through command system).

### Object Selection
- Raycast selection from cursor-to-world ray.
- Multi-select with modifier keys.
- Optional marquee rectangle selecting entities by bounds overlap.

### Transform Manipulation
- Gizmo-driven move/rotate/scale.
- Coordinate spaces: local/world.
- Snapping: translation increments, angle steps, scale steps.
- Pivot modes: selection center, active object, individual origins.

### Editor UI
- **Hierarchy Panel:** entity tree and structure operations.
- **Inspector Panel:** component list and property editing.
- **Viewport Panel:** render target + overlay controls.
- **Asset Browser Panel:** searchable placeable assets.
- **Toolbar:** tool mode, snapping, coordinate space, play/edit controls.

### Tool Modes
- `Select`
- `Translate`
- `Rotate`
- `Scale`
- `Place`
- `Camera Navigate`

---

## Data Structures

### Scene/Selection
- `EditorEntityRef`
  - `EntityId id`
  - `std::string display_name`
  - `EntityId parent_id` (optional)
- `SelectionSet`
  - ordered `std::vector<EntityId>`
  - fast lookup `std::unordered_set<EntityId>`

### Tool State
- `EditorToolState`
  - active tool enum
  - transform mode enum
  - snapping settings
  - local/world toggle
- `TransformSession`
  - drag start transform(s)
  - active axis/plane constraint
  - pivot definition

### Serialization
- `SceneFileHeader`
  - magic/version/flags
- `SerializedEntity`
  - identity + hierarchy links + component payload
- `EditorMetadata`
  - hidden/locked state
  - viewport bookmarks
  - per-entity labels/colors

---

## Utilities

### Ray Picking Utilities
- Build picking ray from camera matrices and mouse coordinates.
- Intersect against entity bounds/proxies.
- Return sorted hit list and nearest valid hit.

### Gizmo Math Helpers
- Axis/plane projection math for drag translation.
- Angular delta solve for rotation rings.
- Stable transform compose/decompose helpers.

### Command Utilities
- Generic value-change command templates.
- Composite/batch command for grouped edits.
- Coalescing helper for continuous drags.

### Editor Input Utilities
- Action mapping + shortcut lookup.
- Input capture checks for focused UI controls.
- Per-tool input adapters.

### Debug Rendering Utilities
- Selection wireframes and bounding boxes.
- Pivot and axis indicators.
- Grid/snap guides and placement previews.

---

## Engine Integration

### Rendering Integration
- Add an editor overlay render pass after main scene rendering.
- Reuse existing debug draw pipeline for gizmos/handles where possible.
- Support depth-tested and x-ray overlay variants.

### Input Integration
- Route engine input through `EditorInputRouter`.
- If editor does not consume an event/action, forward to gameplay input handling.

### Scene Integration
- Perform scene edits through a constrained scene-edit API.
- Keep editor-only state out of runtime component definitions.

### Asset Integration
- Pull asset lists from existing asset/resource systems.
- Support drag-drop payloads into viewport/hierarchy placement flows.

### Save/Load Integration
- Integrate serializer with existing level load/save paths.
- Ensure runtime can ignore editor metadata safely.

---

## Implementation Phases

### Phase 1: Foundations
- Add editor module/context/mode controller.
- Add viewport camera navigation and basic selection.
- Add translation gizmo with command-backed edits.

### Phase 2: Authoring Baseline
- Add hierarchy + inspector + asset browser panels.
- Add command history with undo/redo hotkeys.
- Add save/load for scene plus versioned metadata.

### Phase 3: Workflow Expansion
- Add rotate/scale tools and snapping options.
- Add placement workflows and hierarchy reparenting.
- Add PIE state isolation and restoration.

### Phase 4: Robustness
- Add validation warnings and diagnostics.
- Add shortcut rebinding and user preferences.
- Add performance tuning for large-scene picking/selection.

---

## Quality and Maintainability Guidelines
- Keep editor interfaces narrow and explicit.
- Use command pattern for all mutating edits (no direct ad-hoc mutation from UI).
- Prefer composition over monolithic manager classes.
- Add schema versioning from first serializer revision.
- Isolate editor-only modules under `dny_engine/editor/` to reduce runtime coupling.
