# Soft Renderer Raster-State Extension Proposal (pipeline + basic_effect)

## 1) Renderer architecture summary

### Pipeline staging today

The non-tiled `pipeline<Effect>` currently executes a straightforward software graphics pipeline:

1. **Vertex stage** via `m_effect.vshader(v)` in `render(...)`. The shader is expected to return clip-space output.  
2. **Near-plane clipping** in `clip_triangle(...)` (currently only near-plane is active).  
3. **Triangle rasterization** in `rasterize_simd(...)`, including barycentrics and perspective-correct interpolation.  
4. **Depth test/write and pixel shading** in the inner loop, then direct target write via `m_target->pixel(x, y) = m_effect.pshader(...)`.

This keeps the renderer compact and shader-model driven through templates.

### Shader contract today

`basic_effect<VShader, PShader>` defines the VS/PS pairing and enforces `VShader::vertex_out == PShader::vertex_in` at compile time. That compile-time compatibility is the key design anchor and should be preserved.

### Vertex buffers today

`basic_effect` owns `std::vector<buffer_type const*> vertex_buffer`, but `pipeline::render(...)` currently takes a mesh buffer directly and does not consume `basic_effect::vertex_buffer`.

### Rasterization / framebuffer write today

The raster loop performs:

- hardcoded backface culling assumption (CW front-facing in comment, with signed area test)
- hardcoded depth compare (`if (depth >= depth_buffer[idx]) continue;`)
- unconditional depth write on pass
- unconditional color write of all channels
- no blending (source color overwrites destination)

So raster state exists implicitly in code, not as configurable policy.

---

## 2) Identified extension points

The cleanest extension points consistent with current design:

1. **`basic_effect`**: own state objects (Blend/Depth/Raster/ColorMask), because this class already bundles shader configuration for a draw effect.
2. **`pipeline`**: consume those states in raster/depth/output merge points; keep it branch-light with fast paths.
3. **New lightweight state structs/enums**: value types with defaults matching current behavior, no virtual dispatch.

Avoid placing state only as free pipeline globals; effect-local state is easier to reason about and closer to modern graphics pipeline objects.

---

## 3) Proposed raster-state system

Introduce a compact state block with default values equal to current behavior.

### Blend state

- `enabled`
- `src_factor`, `dst_factor`
- `color_op`, `alpha_op` (optionally same op initially)

### Depth-stencil subset (depth only for now)

- `depth_test_enable`
- `depth_write_enable`
- `depth_compare`

### Rasterizer state

- `cull_mode` (`none/front/back`)
- `front_face` (`cw/ccw`)

### Color write mask

- per-channel `write_r/g/b/a`

This can be grouped as `pipeline_state` inside `basic_effect`.

---

## 4) New data structures (header additions)

Suggested additions in `dny_soft_renderer.hpp` (or split into `dny_raster_state.hpp` and included from soft renderer):

```cpp
namespace dny {

enum class blend_factor : std::uint8_t {
    zero,
    one,
    src_color,
    inv_src_color,
    dst_color,
    inv_dst_color,
    src_alpha,
    inv_src_alpha,
    dst_alpha,
    inv_dst_alpha
};

enum class blend_op : std::uint8_t {
    add,
    subtract,
    rev_subtract,
    min,
    max
};

enum class depth_func : std::uint8_t {
    never,
    less,
    less_equal,
    equal,
    greater,
    greater_equal,
    not_equal,
    always
};

enum class cull_mode : std::uint8_t { none, front, back };
enum class front_face : std::uint8_t { cw, ccw };

struct blend_state {
    bool enabled = false;
    blend_factor src_color = blend_factor::one;
    blend_factor dst_color = blend_factor::zero;
    blend_op color_op = blend_op::add;

    blend_factor src_alpha = blend_factor::one;
    blend_factor dst_alpha = blend_factor::zero;
    blend_op alpha_op = blend_op::add;
};

struct depth_state {
    bool test_enable = true;
    bool write_enable = true;
    depth_func compare = depth_func::less;
};

struct raster_state {
    cull_mode cull = cull_mode::back;
    front_face front = front_face::cw;
};

struct color_write_mask {
    bool r = true;
    bool g = true;
    bool b = true;
    bool a = true;
};

struct pipeline_state {
    blend_state blend{};
    depth_state depth{};
    raster_state raster{};
    color_write_mask color_mask{};
};

} // namespace dny
```

Default values above replicate current output semantics:

- no blending
- depth test/write on with LESS behavior
- backface culling enabled, CW front face
- full RGBA write

---

## 5) Required `pipeline` changes

Only modify state-sensitive points in `pipeline::rasterize_simd(...)`.

### A) Culling

Replace hardcoded signed-area cull check with `raster_state` policy.

Pseudo logic:

```cpp
const bool front_is_negative = (state.raster.front == front_face::cw);
const bool tri_is_front = front_is_negative ? (area_scalar < 0.f) : (area_scalar > 0.f);

if (state.raster.cull == cull_mode::back && !tri_is_front) return;
if (state.raster.cull == cull_mode::front && tri_is_front) return;
```

### B) Depth test + optional depth write

Refactor current compare/write block:

```cpp
bool pass_depth = true;
if (state.depth.test_enable) {
    pass_depth = depth_compare(depth, m_depth_buffer->at(idx), state.depth.compare);
}
if (!pass_depth) continue;
if (state.depth.write_enable) {
    m_depth_buffer->at(idx) = depth;
}
```

### C) Output merge (blend + color mask)

Current behavior is direct assignment. Proposed output merger:

```cpp
Color32 src = to_color32(m_effect.pshader(frag_pshader));
Color32 dst = m_target->pixel(x, y);

Color32 out = src;
if (state.blend.enabled) {
    out = blend(src, dst, state.blend);
}
out = apply_color_mask(out, dst, state.color_mask);
m_target->pixel(x, y) = out;
```

### D) Keep branch-light fast paths

Maintain a hot fast path:

- `if (!blend.enabled && all_mask_enabled)` -> direct write
- avoid per-channel branches by using mask select helpers

---

## 6) Required `basic_effect` changes

Add pipeline-state ownership to effect:

```cpp
template<vertex_shader VShader, pixel_shader PShader>
requires std::is_same_v<typename VShader::vertex_out, typename PShader::vertex_in>
class basic_effect {
public:
    using vshader_type = VShader;
    using pshader_type = PShader;
    using buffer_type = std::vector<typename VShader::vertex_in>;

    vshader_type vshader;
    pshader_type pshader;
    std::vector<buffer_type const*> vertex_buffer;

    pipeline_state state{}; // NEW
};
```

This keeps the existing template model intact and avoids introducing dynamic polymorphism.

Optional compile-time variant (future):

```cpp
template<vertex_shader VShader, pixel_shader PShader, typename FixedStateTag = default_state_tag>
class basic_effect { /* ... */ };
```

with `FixedStateTag` exposing `static constexpr pipeline_state value` for specialized pipelines.

---

## 7) Example usage

```cpp
using effect_t = dny::basic_effect<MyVS, MyPS>;

// Setup
pipeline<effect_t> p;
// ... render target / depth buffer / cbuffers

// Raster state
p.effect().state.raster.cull = dny::cull_mode::back;
p.effect().state.raster.front = dny::front_face::cw;

// Depth state
p.effect().state.depth.test_enable = true;
p.effect().state.depth.write_enable = true;
p.effect().state.depth.compare = dny::depth_func::less_equal;

// Alpha blending
p.effect().state.blend.enabled = true;
p.effect().state.blend.src_color = dny::blend_factor::src_alpha;
p.effect().state.blend.dst_color = dny::blend_factor::inv_src_alpha;
p.effect().state.blend.color_op = dny::blend_op::add;

// Color mask (RGB-only write)
p.effect().state.color_mask = { true, true, true, false };
```

If direct effect access is currently private in `pipeline`, add one tiny accessor:

```cpp
Effect& effect() noexcept { return m_effect; }
Effect const& effect() const noexcept { return m_effect; }
```

---

## 8) Implementation guidance (minimal-impact plan)

1. Add enums/state structs and helper functions (`depth_compare`, `blend_factor_eval`, `blend_eval`, `apply_color_mask`).
2. Add `pipeline_state state{}` to `basic_effect`.
3. In `pipeline::rasterize_simd(...)`, replace hardcoded cull/depth/write/output with state-driven logic.
4. Keep existing defaults equal to old fixed behavior to preserve compatibility.
5. Keep tiled pipeline unchanged for now (as requested); only mirror later after validating non-tiled behavior.

### Performance notes

- No virtual dispatch introduced.
- State checks are small boolean branches with coherent behavior per draw call.
- Optional future optimization: cache function pointers/lambdas for depth and blend ops per draw call to remove switch from inner loop.

### Compatibility notes

- Existing effects compile unchanged if defaults are used.
- Existing shaders unchanged; this proposal only changes fixed-function-esque raster/output behavior.
