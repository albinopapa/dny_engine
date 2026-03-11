# Engine Repository Reorganization Plan

## Current Structure

The repository currently has a **mixed flat layout** under `dny_engine/` where most engine headers, source files, runtime/game files, and UI code are colocated. The only major subfolders are `Assets/`, `Docs/`, and `ui/`.

### Observed top-level areas
- Root docs/plans (`README.md`, `editor_design_plan.md`, roadmap docs)
- Visual Studio solution/project files (`dny_engine.sln`, `dny_engine/dny_engine.vcxproj`)
- Engine/game code under `dny_engine/`
- UI controls under `dny_engine/ui/`
- Assets under `dny_engine/Assets/Textures/`
- Design/spec docs under `dny_engine/Docs/`

### System relationships (as currently implemented)
- `Source.cpp` is the app entry point and runs the platform + game loop.
- `Game` is an integration point tying together platform, input, renderer, math, physics, textures, font/text, and optional audio.
- Core subsystems (math, renderer, graphics, input, platform, physics, asset loading, audio) are present but not directory-separated.
- Editor runtime code is not yet implemented as a standalone module; editor intent exists mainly as design docs and editor-focused UI primitives.

---

## Proposed Module Layout

```text
engine/
  include/
    dny/
      core/
      math/
      graphics/
      renderer/
      physics/
      input/
      platform/
      assets/
      audio/
      ui/

  src/
    input/
    assets/
    audio/
    ui/
    platform/         # optional now, useful when platform impl moves out of header

apps/
  runtime/
    src/
  editor/
    src/

sandbox/
  src/

docs/
  editor/
  engine/

assets/
  textures/
```

### Include hierarchy convention
Public engine headers should be reachable via:
- `#include <dny/<module>/<file>.hpp>`

Examples:
- `#include <dny/input/keyboard.hpp>`
- `#include <dny/renderer/soft_renderer.hpp>`

---

## Module Descriptions

### core
Shared foundational types/utilities not specific to any single subsystem.
- Likely contents: `dny_concepts.hpp`, `dny_type_traits.hpp`, `dny_utilities.hpp`, `dny_timer.hpp`, dims/rect/surface/color helpers as needed.

### math
All math primitives and algorithms.
- Likely contents: vectors, matrices, constants, frustum, SIMD, general math helpers.

### renderer
Pipeline and render architecture internals.
- Likely contents: `dny_soft_renderer.hpp`, primitive generators, shader/effect definitions.

### graphics
2D immediate drawing and text rendering utilities.
- Likely contents: `dny_graphics.hpp` and related drawing helpers.

### physics
Collision and geometry queries.
- Likely contents: `dny_aabb.hpp`, `dny_physics.hpp`, `dny_qtree.hpp`.

### input
Input devices and action binding abstraction.
- Likely contents: keyboard/mouse/gamepad types + `Input` action mapping.

### platform
Window/display + OS integration layer.
- Likely contents: `dny_platform.hpp`, `dny_display.hpp`, platform SDK wrappers.

### assets
Runtime asset loading/build utilities.
- Likely contents: image loader, text atlas builder, font support helpers.

### audio
Audio engine and clip management.
- Likely contents: `dny_audio.hpp/.cpp`, example/test support files.

### ui
Editor-oriented retained UI primitives.
- Likely contents: `ui/element`, `button`, `panel`, `textbox`, etc.

### apps/runtime and apps/editor
Executable orchestration code (not core engine library code).
- Runtime bootstrap and game/editor host code should live here.

---

## File Categorization

## 1) Library Modules (Reusable Engine Systems)

These should live under `engine/include/dny/<module>/` and `engine/src/<module>/`:

- `dny_*.hpp` subsystem headers (math, renderer, graphics, physics, platform, assets, audio, core)
- Input device and mapping files:
  - `keyboard.hpp/.cpp`
  - `mouse.hpp/.cpp`
  - `gamepad.hpp/.cpp`
  - `input.hpp/.cpp`
- Asset/audio implementations:
  - `dny_image_loader.hpp/.cpp`
  - `dny_text_atlas_builder.hpp/.cpp`
  - `dny_audio.hpp/.cpp`
- UI primitives under `ui/` (library-style controls)

## 2) Game Engine / Editor Application Layer

These are app-level orchestration and gameplay/editor host code:
- `Source.cpp` (runtime entry)
- `game.hpp/.cpp`
- `game_camera.hpp`
- `game_player.hpp`
- `game_effects.hpp`

Editor design docs indicate planned editor architecture but current codebase does not yet have a dedicated `editor/` runtime module implementation.

## 3) Sandbox / Test Code

Explicitly sandbox by instruction:
- `game.hpp`
- `game.cpp`
- `Source.cpp`

Also strongly consider moving experimentation/test-only code here (or into dedicated test targets):
- `dny_audio_test.cpp`

---

## Required File Moves

## A) Create the target directories

Create:
- `engine/include/dny/{core,math,graphics,renderer,physics,input,platform,assets,audio,ui}`
- `engine/src/{input,assets,audio,ui,platform}`
- `apps/runtime/src`
- `apps/editor/src`
- `sandbox/src`
- `docs/{editor,engine}`
- `assets/textures`

## B) Move library headers into `engine/include`

Examples:
- Move `dny_engine/keyboard.hpp` to `engine/include/dny/input/keyboard.hpp`
- Move `dny_engine/mouse.hpp` to `engine/include/dny/input/mouse.hpp`
- Move `dny_engine/gamepad.hpp` to `engine/include/dny/input/gamepad.hpp`
- Move `dny_engine/input.hpp` to `engine/include/dny/input/input.hpp`

- Move `dny_engine/dny_soft_renderer.hpp` to `engine/include/dny/renderer/soft_renderer.hpp`
- Move `dny_engine/dny_graphics.hpp` to `engine/include/dny/graphics/graphics.hpp`
- Move `dny_engine/dny_physics.hpp` to `engine/include/dny/physics/physics.hpp`
- Move `dny_engine/dny_platform.hpp` to `engine/include/dny/platform/platform.hpp`

- Move `dny_engine/ui/button.hpp` to `engine/include/dny/ui/button.hpp`
- Move `dny_engine/ui/element.hpp` to `engine/include/dny/ui/element.hpp`
- (repeat for all UI headers)

## C) Move implementation files into `engine/src`

Examples:
- Move `dny_engine/keyboard.cpp` to `engine/src/input/keyboard.cpp`
- Move `dny_engine/mouse.cpp` to `engine/src/input/mouse.cpp`
- Move `dny_engine/gamepad.cpp` to `engine/src/input/gamepad.cpp`
- Move `dny_engine/input.cpp` to `engine/src/input/input.cpp`

- Move `dny_engine/dny_image_loader.cpp` to `engine/src/assets/image_loader.cpp`
- Move `dny_engine/dny_text_atlas_builder.cpp` to `engine/src/assets/text_atlas_builder.cpp`
- Move `dny_engine/dny_audio.cpp` to `engine/src/audio/audio.cpp`

- Move `dny_engine/ui/button.cpp` to `engine/src/ui/button.cpp`
- (repeat for all UI `.cpp` files)

## D) Move sandbox files (explicit requirement)

- Move `dny_engine/game.hpp` to `sandbox/src/game.hpp`
- Move `dny_engine/game.cpp` to `sandbox/src/game.cpp`
- Move `dny_engine/Source.cpp` to `sandbox/src/main.cpp`

## E) Move docs/assets to clearer ownership roots

- Move `dny_engine/Assets/Textures/*` to `assets/textures/*`
- Move editor planning docs (`editor_design_plan.md`, `dny_engine/Docs/editor_*`) into `docs/editor/`
- Move rendering/input/audio technical notes into `docs/engine/` by topic

## F) Update build configuration files

After file moves, update:
- `dny_engine/dny_engine.vcxproj`
- `dny_engine/dny_engine.vcxproj.filters`
- possibly solution project references (`dny_engine.sln`)

Key build updates:
- Include path root: `engine/include`
- Source path updates to `engine/src/*` and `sandbox/src/*` (or `apps/runtime/src/*`)
- Split targets where possible:
  - `dny_engine_lib` (engine static/shared lib)
  - `sandbox_app` (or runtime app)

---

## Sandbox Files

The following files are sandbox and should be isolated:
- `game.hpp`
- `game.cpp`
- `Source.cpp`

Target location:

```text
sandbox/
  src/
    main.cpp
    game.hpp
    game.cpp
```

`dny_audio_test.cpp` should also be treated as non-production and moved either to:
- `sandbox/src/audio_test.cpp`, or
- `tests/audio/audio_api_test.cpp` if you introduce a formal test layout.

---

## Naming Recommendations

1. **Drop redundant `dny_` filename prefix inside `dny/` include tree**
   - Example: `dny_soft_renderer.hpp` -> `soft_renderer.hpp`
   - Namespace remains `dny`, so clarity is preserved.

2. **Keep stable module-based include paths**
   - Prefer: `<dny/renderer/soft_renderer.hpp>` over relative includes.

3. **Standardize source naming**
   - Match headers and sources 1:1 where possible:
     - `platform.hpp` + `platform.cpp`
     - `input.hpp` + `input.cpp`

4. **Quarantine transitional/experimental variants**
   - `dny_math_v2.hpp` should be clearly marked experimental (e.g., `math/experimental/math_v2.hpp`) until merged/replaced.

5. **Separate app names from engine names**
   - Avoid app-specific names in engine core (`game_*` belongs in app/sandbox).

---

## Recommended Next Steps

1. **Create folder skeleton first** (no moves yet).
2. **Move sandbox trio first** (`game.hpp`, `game.cpp`, `Source.cpp`) to reduce coupling in engine root.
3. **Migrate input module** (`keyboard/mouse/gamepad/input`) and validate build.
4. **Migrate assets/audio module** and validate build.
5. **Migrate UI primitives** and validate build.
6. **Migrate remaining header-only modules** (math/core/physics/renderer/graphics/platform).
7. **Update project files** (`.vcxproj/.filters/.sln`) to new layout.
8. **Add architecture doc + ownership map** in `docs/engine/`.
9. **Optionally split into multiple projects** (engine library, sandbox app, future editor app).

This staged sequence minimizes breakage and keeps each migration step reviewable.
