# Metroidvania Software-Renderer Engine Roadmap (No 3rd-Party Dependencies)

## 0) Current codebase snapshot (what exists)

- **Project shape:** single Visual Studio C++ solution (`dny_engine.sln`) with one primary project and mostly header-only subsystems under `dny_engine/`.
- **Rendering core:** custom software rasterizer pipeline in `dny_soft_renderer.hpp` and a tiled variant in `dny_soft_renderer_tiled.hpp` with:
  - generic vertex format (`basic_vertex`)
  - template shader model (`basic_vertex_shader`, `basic_pixel_shader`)
  - clipping/rasterization, barycentric interpolation, depth handling (tiled version uses explicit depth buffer)
- **Platform/display layer:** Win32 display + message loop abstractions (`dny_platform.hpp`, `dny_display.hpp`, `dny_win32sdk.hpp`).
- **Math/data types:** vectors, matrices, colors, geometry helpers (`dny_vector*`, `dny_matrix_*`, `dny_math*`, `dny_aabb.hpp`, `dny_rectangle.hpp`).
- **Utilities present:** font/text atlas/image loading/quadtree/physics scaffolding (`dny_font.hpp`, `dny_text_atlas_builder.hpp`, `dny_image_loader.hpp`, `dny_qtree.hpp`, `dny_physics.hpp`).
- **Current executable path:** `Source.cpp` draws a simple triangle through the tiled renderer to a software surface and presents it.

---

## 1) Immediate fixes and hardening (pre-MVP gate)

These should be addressed before building gameplay systems so the foundation is reliable.

### 1.1 Fix `dny_graphics.hpp` compile/runtime issues
- `draw(...)` references `m_width`/`m_height` identifiers that are not in scope; derive width/height from `canvas_`.
- `clip_rect` currently requires two arguments, but call sites invoke a one-argument overload (`clip_rect(rect_)`) that does not exist.
- Standardize clipping utility signatures and ensure all draw/fill helpers clip against the target canvas.

### 1.2 Reconcile non-tiled vs tiled renderer API consistency
- `dny_soft_renderer.hpp` references `m_target->depth_test(...)`, but `surface` has no such member.
- Either remove/retire the non-tiled path or rework it to match the explicit depth-buffer model used by `dny_soft_renderer_tiled.hpp`.
- Keep one canonical renderer path for MVP to avoid divergence.

### 1.3 Validate shader interface correctness in sample entry
- In `Source.cpp`, ensure pixel shader call operator uses the expected interpolated input type (vertex shader output) and naming is consistent with pipeline constraints.
- Add a minimal “known-good” scene sample (textured quad + depth overlap) as a regression baseline.

### 1.4 Add deterministic frame timing + fixed-step simulation loop
- Introduce a fixed update step (e.g., 60 Hz) decoupled from render frequency.
- Keep variable rendering, fixed simulation to stabilize collision and movement behavior.

---

## 2) Organization and architecture tasks

### 2.1 Restructure into explicit engine modules
Create clear top-level module boundaries (folders/namespaces):
- `core/` (app loop, timing, logging, alloc helpers)
- `platform/` (window/input/filesystem abstraction, Win32 implementation)
- `render/` (software renderer, material/shader wrappers, scene submission)
- `assets/` (texture/font/mesh/level loading)
- `gameplay/` (entity components, systems)
- `tools/` (offline processors, atlas and level packers)

### 2.2 Define stable API boundaries (wrappers)
- Keep low-level headers internal.
- Expose a narrow public engine API for game code (init, load level, run frame, submit drawables).
- Add per-module readme docs and ownership notes.

### 2.3 Build + quality workflow (no dependencies)
- Add repeatable build scripts for MSVC and optionally clang-cl (batch or PowerShell).
- Add lightweight self-tests executable(s) using only standard library assertions.
- Add style conventions document (naming, memory ownership, header policy).

---

## 3) Backend wrappers to create (MVP-critical)

> “Backend wrappers” here means internal abstraction layers separating gameplay/runtime code from concrete implementations.

### 3.1 Platform wrapper
- `IWindow`, `IInput`, `ITimer`, `IFileSystem` interfaces.
- `Win32Window`, `Win32Input`, etc. implementations in platform backend.
- Game/runtime should consume interfaces, not raw Win32 calls.

### 3.2 Rendering backend wrapper
- `IRenderDevice` with commands:
  - begin/end frame
  - set camera
  - submit static mesh / sprite quad / UI glyphs
  - set material parameters (albedo, normal map, tint)
- First implementation: software raster backend (`SoftwareRenderDevice`).

### 3.3 Resource backend wrapper
- `ITextureStore`, `IMeshStore`, `IAnimationStore`, `ILevelStore`.
- Caching + lifetime management (handle IDs) so systems don’t pass raw pointers.

### 3.4 Audio wrapper (minimal)
- Even if delayed for MVP polish, create `IAudioDevice` now with no-op implementation.
- Prevents gameplay code from hard-coding platform audio details.

---

## 4) Core MVP feature roadmap (2.5D side-scroller)

## Phase A — Rendering MVP
1. Camera system:
   - perspective camera for 3D environment
   - constrained side-scroll follow behavior
2. Draw primitives:
   - textured quads for player/enemies/props
   - static 3D mesh rendering for level geometry
3. Material MVP:
   - unlit textured rendering
   - vertex color/tint multiply
4. Depth + sorting:
   - strict depth test, consistent near/far setup
   - transparent object strategy (basic sorted pass)
5. Visibility:
   - frustum culling for mesh chunks
   - optional quadtree/grid culling for entities

## Phase B — Gameplay MVP
1. Entity model:
   - lightweight ECS-style or component lists (no dependency)
   - transforms, sprite/mesh render component, collider, motion
2. 2D action-plane rules:
   - lock actors to action-plane Z
   - preserve 3D environment rendering around that plane
3. Character controller:
   - move/jump/fall, coyote time, jump buffering
4. Collision system:
   - broadphase (grid/quadtree), narrowphase (AABB first)
   - one-way platforms, slopes optional post-MVP
5. Enemy baseline:
   - one patrol enemy + basic hitbox combat loop
6. Save/checkpoint:
   - simple level restart + checkpoint state serialization

## Phase C — Content pipeline MVP
1. Level format:
   - define a simple plain-text/binary level schema (tiles/chunks/spawn points)
2. Offline tools:
   - sprite atlas packer (reuse text atlas patterns where practical)
   - mesh conversion tool for static environment chunks
3. Runtime loading:
   - streaming by room/chunk boundaries

## Phase D — Normal-mapped lighting (post-MVP extension)
1. Tangent-space normal map support in software pixel shader.
2. One dynamic directional light + ambient term.
3. Optional per-room point lights (small capped count).
4. Debug visualization modes (normals, light accumulation, depth).

---

## 5) Useful utilities to add

- Frame capture/debug dump utility (write color/depth buffers to image).
- In-engine debug overlay (FPS, timings, entity counts, draw calls, memory usage).
- Deterministic replay recorder (input stream record/playback for bug reproduction).
- Resource hot-reload hooks (safe for dev builds, optional if no file watchers).
- Geometry/debug draw helpers (AABB, rays, collision normals, tile bounds).
- Build-time config header generator (feature flags for debug/profiling).

---

## 6) Suggested milestone plan (time-boxed)

### Milestone 1 — Engine foundation (1–2 weeks)
- Complete section 1 fixes
- Establish module layout and wrappers (sections 2 and 3)
- Produce fixed-step loop + debug overlay

### Milestone 2 — Render + movement vertical slice (2–4 weeks)
- Camera + textured quads + static mesh world
- Basic player controller + collisions + one room loaded from file
- One enemy type and checkpoint restart

### Milestone 3 — MVP playable room set (2–4 weeks)
- Multiple connected rooms/chunks
- Stable performance with culling and tile/chunk streaming
- Core combat interaction + HUD text

### Milestone 4 — Lighting extension (post-MVP)
- Normal-mapped directional lighting
- Artist-facing material workflow documentation

---

## 7) MVP definition checklist

You have an MVP when all are true:
- Player can traverse at least 3 connected rooms in a side-scrolling path.
- World geometry is rendered in 3D while gameplay remains on 2D action plane.
- Entities are textured quads with functioning collision/combat.
- Engine runs on software renderer path only (no third-party dependencies).
- Level/content loads from data files rather than hard-coded scene.
- Debug overlay shows frame time and major subsystem timings.

---

## 8) No-third-party dependency policy (implementation notes)

- Use only C++ standard library + Win32 APIs already in project scope.
- Keep custom image/font/asset pipelines in-repo.
- Prefer simple custom binary formats over bringing external serializers.
- If SIMD portability is needed beyond SSE/x86, implement internal compile-time backend switches (SSE scalar fallback) rather than external libs.
