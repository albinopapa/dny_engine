# Engine Repository Restructure Script
# Generated from Docs/dny_engine_reorganization_plan.md

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Ensure-Directory {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Host "[CREATE] $Path"
    }
    else {
        Write-Host "[EXISTS] $Path"
    }
}

function Safe-Move {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source)) {
        Write-Host "[SKIP] Missing source: $Source"
        return
    }

    $destDir = Split-Path -Parent $Destination
    if ($destDir) {
        Ensure-Directory -Path $destDir
    }

    if (Test-Path -LiteralPath $Destination) {
        Write-Host "[SKIP] Destination exists, not overwriting: $Destination"
        return
    }

    Move-Item -LiteralPath $Source -Destination $Destination
    Write-Host "[MOVE] $Source -> $Destination"
}

function Safe-Move-ByPattern {
    param(
        [Parameter(Mandatory = $true)][string]$SourcePattern,
        [Parameter(Mandatory = $true)][string]$DestinationDirectory
    )

    Ensure-Directory -Path $DestinationDirectory

    $files = Get-ChildItem -Path $SourcePattern -File -ErrorAction SilentlyContinue
    if (-not $files) {
        Write-Host "[SKIP] No files matched pattern: $SourcePattern"
        return
    }

    foreach ($file in $files) {
        $dest = Join-Path $DestinationDirectory $file.Name
        if (Test-Path -LiteralPath $dest) {
            Write-Host "[SKIP] Destination exists, not overwriting: $dest"
            continue
        }

        Move-Item -LiteralPath $file.FullName -Destination $dest
        Write-Host "[MOVE] $($file.FullName) -> $dest"
    }
}

Write-Host "Creating module directories..."

$directories = @(
    'engine/include/dny/core',
    'engine/include/dny/math',
    'engine/include/dny/graphics',
    'engine/include/dny/renderer',
    'engine/include/dny/physics',
    'engine/include/dny/input',
    'engine/include/dny/platform',
    'engine/include/dny/assets',
    'engine/include/dny/audio',
    'engine/include/dny/ui',
    'engine/src/input',
    'engine/src/assets',
    'engine/src/audio',
    'engine/src/ui',
    'engine/src/platform',
    'apps/runtime/src',
    'apps/editor/src',
    'sandbox/src',
    'docs/editor',
    'docs/engine',
    'assets/textures'
)

foreach ($dir in $directories) {
    Ensure-Directory -Path $dir
}

Write-Host "Moving files..."

# Input module headers
Safe-Move 'dny_engine/keyboard.hpp' 'engine/include/dny/input/keyboard.hpp'
Safe-Move 'dny_engine/mouse.hpp' 'engine/include/dny/input/mouse.hpp'
Safe-Move 'dny_engine/gamepad.hpp' 'engine/include/dny/input/gamepad.hpp'
Safe-Move 'dny_engine/input.hpp' 'engine/include/dny/input/input.hpp'
Safe-Move 'dny_engine/dny_input.hpp' 'engine/include/dny/input/dny_input.hpp'

# Input module source
Safe-Move 'dny_engine/keyboard.cpp' 'engine/src/input/keyboard.cpp'
Safe-Move 'dny_engine/mouse.cpp' 'engine/src/input/mouse.cpp'
Safe-Move 'dny_engine/gamepad.cpp' 'engine/src/input/gamepad.cpp'
Safe-Move 'dny_engine/input.cpp' 'engine/src/input/input.cpp'

# Renderer / graphics / physics / platform headers
Safe-Move 'dny_engine/dny_soft_renderer.hpp' 'engine/include/dny/renderer/soft_renderer.hpp'
Safe-Move 'dny_engine/dny_raster_state.hpp' 'engine/include/dny/renderer/raster_state.hpp'
Safe-Move 'dny_engine/dny_primitive_generators.hpp' 'engine/include/dny/renderer/primitive_generators.hpp'
Safe-Move 'dny_engine/game_effects.hpp' 'engine/include/dny/renderer/game_effects.hpp'

Safe-Move 'dny_engine/dny_graphics.hpp' 'engine/include/dny/graphics/graphics.hpp'

Safe-Move 'dny_engine/dny_physics.hpp' 'engine/include/dny/physics/physics.hpp'
Safe-Move 'dny_engine/dny_aabb.hpp' 'engine/include/dny/physics/aabb.hpp'
Safe-Move 'dny_engine/dny_qtree.hpp' 'engine/include/dny/physics/qtree.hpp'

Safe-Move 'dny_engine/dny_platform.hpp' 'engine/include/dny/platform/platform.hpp'
Safe-Move 'dny_engine/dny_display.hpp' 'engine/include/dny/platform/display.hpp'
Safe-Move 'dny_engine/dny_win32sdk.hpp' 'engine/include/dny/platform/win32sdk.hpp'
Safe-Move 'dny_engine/dny_d3d11sdk.hpp' 'engine/include/dny/platform/d3d11sdk.hpp'
Safe-Move 'dny_engine/dny_d2dsdk.hpp' 'engine/include/dny/platform/d2dsdk.hpp'
Safe-Move 'dny_engine/dny_dwritesdk.hpp' 'engine/include/dny/platform/dwritesdk.hpp'
Safe-Move 'dny_engine/dny_wicsdk.hpp' 'engine/include/dny/platform/wicsdk.hpp'

# Assets module
Safe-Move 'dny_engine/dny_image_loader.hpp' 'engine/include/dny/assets/image_loader.hpp'
Safe-Move 'dny_engine/dny_text_atlas_builder.hpp' 'engine/include/dny/assets/text_atlas_builder.hpp'
Safe-Move 'dny_engine/dny_font.hpp' 'engine/include/dny/assets/font.hpp'
Safe-Move 'dny_engine/dny_image_loader.cpp' 'engine/src/assets/image_loader.cpp'
Safe-Move 'dny_engine/dny_text_atlas_builder.cpp' 'engine/src/assets/text_atlas_builder.cpp'

# Audio module
Safe-Move 'dny_engine/dny_audio.hpp' 'engine/include/dny/audio/audio.hpp'
Safe-Move 'dny_engine/dny_audio_example.hpp' 'engine/include/dny/audio/audio_example.hpp'
Safe-Move 'dny_engine/dny_audio.cpp' 'engine/src/audio/audio.cpp'

# Core / math headers
Safe-Move 'dny_engine/dny_concepts.hpp' 'engine/include/dny/core/concepts.hpp'
Safe-Move 'dny_engine/dny_type_traits.hpp' 'engine/include/dny/core/type_traits.hpp'
Safe-Move 'dny_engine/dny_utilities.hpp' 'engine/include/dny/core/utilities.hpp'
Safe-Move 'dny_engine/dny_timer.hpp' 'engine/include/dny/core/timer.hpp'
Safe-Move 'dny_engine/dny_colors.hpp' 'engine/include/dny/core/colors.hpp'
Safe-Move 'dny_engine/dny_dims.hpp' 'engine/include/dny/core/dims.hpp'
Safe-Move 'dny_engine/dny_dims2.hpp' 'engine/include/dny/core/dims2.hpp'
Safe-Move 'dny_engine/dny_rectangle.hpp' 'engine/include/dny/core/rectangle.hpp'
Safe-Move 'dny_engine/dny_surface.hpp' 'engine/include/dny/core/surface.hpp'

Safe-Move 'dny_engine/dny_math.hpp' 'engine/include/dny/math/math.hpp'
Safe-Move 'dny_engine/dny_math_constants.hpp' 'engine/include/dny/math/math_constants.hpp'
Safe-Move 'dny_engine/dny_math_v2.hpp' 'engine/include/dny/math/experimental/math_v2.hpp'
Safe-Move 'dny_engine/dny_simd.hpp' 'engine/include/dny/math/simd.hpp'
Safe-Move 'dny_engine/dny_vector2.hpp' 'engine/include/dny/math/vector2.hpp'
Safe-Move 'dny_engine/dny_vector3.hpp' 'engine/include/dny/math/vector3.hpp'
Safe-Move 'dny_engine/dny_vector4.hpp' 'engine/include/dny/math/vector4.hpp'
Safe-Move 'dny_engine/dny_matrix_3x2.hpp' 'engine/include/dny/math/matrix_3x2.hpp'
Safe-Move 'dny_engine/dny_matrix_3x3.hpp' 'engine/include/dny/math/matrix_3x3.hpp'
Safe-Move 'dny_engine/dny_matrix_4x4.hpp' 'engine/include/dny/math/matrix_4x4.hpp'
Safe-Move 'dny_engine/dny_frustum.hpp' 'engine/include/dny/math/frustum.hpp'

# UI headers and source
Safe-Move 'dny_engine/ui/element.hpp' 'engine/include/dny/ui/element.hpp'
Safe-Move 'dny_engine/ui/button.hpp' 'engine/include/dny/ui/button.hpp'
Safe-Move 'dny_engine/ui/panel.hpp' 'engine/include/dny/ui/panel.hpp'
Safe-Move 'dny_engine/ui/textbox.hpp' 'engine/include/dny/ui/textbox.hpp'
Safe-Move 'dny_engine/ui/input_textbox.hpp' 'engine/include/dny/ui/input_textbox.hpp'
Safe-Move 'dny_engine/ui/listbox.hpp' 'engine/include/dny/ui/listbox.hpp'
Safe-Move 'dny_engine/ui/checkbox.hpp' 'engine/include/dny/ui/checkbox.hpp'
Safe-Move 'dny_engine/ui/radiobutton.hpp' 'engine/include/dny/ui/radiobutton.hpp'
Safe-Move 'dny_engine/ui/dny_ui.hpp' 'engine/include/dny/ui/dny_ui.hpp'

Safe-Move 'dny_engine/ui/element.cpp' 'engine/src/ui/element.cpp'
Safe-Move 'dny_engine/ui/button.cpp' 'engine/src/ui/button.cpp'
Safe-Move 'dny_engine/ui/panel.cpp' 'engine/src/ui/panel.cpp'
Safe-Move 'dny_engine/ui/textbox.cpp' 'engine/src/ui/textbox.cpp'
Safe-Move 'dny_engine/ui/input_textbox.cpp' 'engine/src/ui/input_textbox.cpp'
Safe-Move 'dny_engine/ui/listbox.cpp' 'engine/src/ui/listbox.cpp'
Safe-Move 'dny_engine/ui/checkbox.cpp' 'engine/src/ui/checkbox.cpp'
Safe-Move 'dny_engine/ui/radiobutton.cpp' 'engine/src/ui/radiobutton.cpp'

# Sandbox relocation (explicit)
Safe-Move 'dny_engine/game.hpp' 'sandbox/src/game.hpp'
Safe-Move 'dny_engine/game.cpp' 'sandbox/src/game.cpp'
Safe-Move 'dny_engine/Source.cpp' 'sandbox/src/main.cpp'
Safe-Move 'dny_engine/dny_audio_test.cpp' 'sandbox/src/audio_test.cpp'

# Assets/doc ownership roots
Safe-Move-ByPattern 'dny_engine/Assets/Textures/*' 'assets/textures'

Safe-Move 'editor_design_plan.md' 'docs/editor/editor_design_plan.md'
Safe-Move 'dny_engine/Docs/editor_spec_doc.txt' 'docs/editor/editor_spec_doc.txt'
Safe-Move 'dny_engine/Docs/editor_ui_library_design.md' 'docs/editor/editor_ui_library_design.md'
Safe-Move 'dny_engine/Docs/editor_ui_example.cpp' 'docs/editor/editor_ui_example.cpp'

Safe-Move 'dny_engine/dny_audio_README.md' 'docs/engine/audio_readme.md'
Safe-Move 'dny_engine/AUDIO_BUILD_FIXES.md' 'docs/engine/audio_build_fixes.md'
Safe-Move 'dny_engine/input_subsystem_design.md' 'docs/engine/input_subsystem_design.md'
Safe-Move 'dny_engine/Docs/soft_renderer_raster_state_design.md' 'docs/engine/soft_renderer_raster_state_design.md'
Safe-Move 'dny_engine/Docs/raster_state_implementation_status.md' 'docs/engine/raster_state_implementation_status.md'
Safe-Move 'dny_engine/Docs/dny_engine_reorganization_plan.md' 'docs/engine/dny_engine_reorganization_plan.md'

Write-Host "Repository restructure complete."
