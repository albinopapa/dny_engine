#pragma once

#include "iapp_state.hpp"
#include "imode.hpp"
#include "level_serializer.hpp"
#include "tilemap_view.hpp"
#include "utilities.hpp"

#include "utilities/rectangle.hpp"
#include "graphics/surface.hpp"
#include "graphics/font.hpp"
#include "math/math.hpp"
#include "ui/dny_ui.hpp"

#include <array>
#include <cassert>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace dny{
	struct EditorCamera{
		// ---------------------------------------------------------------------
		// Camera state
		// ---------------------------------------------------------------------

		Rect<float> viewport{};        // screen-space viewport
		vector3<float> position{ 0.f, 0.f, -10.f };
		// Camera position in world
		// Must be < 0 so it looks toward +Z

		float ortho_scale = 1.f;

		static constexpr float min_scale = 0.25f;
		static constexpr float max_scale = 8.f;

		// Forward = +Z (positive Z away from camera)
		static constexpr vector3<float> forward{ 0.f, 0.f, 1.f };

		// ---------------------------------------------------------------------
		// Screen -> world on Z = 0 plane
		// ---------------------------------------------------------------------
		vector3<float> screen_to_plane( vector2<float> const& screen_pos ) const noexcept{
			// Convert screen -> NDC (0..1 inside viewport)
			const float nx =
				( screen_pos.x - viewport.left ) /
				( viewport.right - viewport.left );

			const float ny =
				( screen_pos.y - viewport.top ) /
				( viewport.bottom - viewport.top );

			// Convert to centered coords (-0.5 .. 0.5)
			const float cx = nx - 0.5f;
			const float cy = ny - 0.5f;

			const float width =
				( viewport.right - viewport.left ) * ortho_scale;

			const float height =
				( viewport.bottom - viewport.top ) * ortho_scale;

			// World position on Z=0 plane
			vector3<float> world;

			world.x = position.x + cx * width;
			world.y = position.y + cy * height;
			world.z = 0.f;

			return world;
		}

		// ---------------------------------------------------------------------
		// Zoom toward cursor (orthographic)
		// ---------------------------------------------------------------------
		void zoom_in( vector2<float> const& screen_pos ) noexcept{
			const auto old_world = screen_to_plane( screen_pos );

			ortho_scale = std::clamp(
				ortho_scale * 0.9f, min_scale, max_scale
			);

			const auto new_world = screen_to_plane( screen_pos );

			const auto delta = old_world - new_world;

			position.x += delta.x;
			position.y += delta.y;
		}

		void zoom_out( vector2<float> const& screen_pos ) noexcept{
			const auto old_world =
				screen_to_plane( screen_pos );

			ortho_scale =
				std::clamp(
					ortho_scale * 1.1f,
					min_scale,
					max_scale
				);

			const auto new_world =
				screen_to_plane( screen_pos );

			const auto delta = old_world - new_world;

			position.x += delta.x;
			position.y += delta.y;
		}

		// ---------------------------------------------------------------------
		// Pan in screen space
		// ---------------------------------------------------------------------
		void pan( vector2<float> const& delta_screen ) noexcept{
			const float width =
				( viewport.right - viewport.left ) * ortho_scale;

			const float height =
				( viewport.bottom - viewport.top ) * ortho_scale;

			position.x -=
				delta_screen.x / ( viewport.right - viewport.left ) * width;

			position.y -=
				delta_screen.y / ( viewport.bottom - viewport.top ) * height;
		}

		// ---------------------------------------------------------------------
		// Helpers for renderer (optional)
		// ---------------------------------------------------------------------
		float ortho_width() const noexcept{
			return
				( viewport.right - viewport.left ) *
				ortho_scale;
		}

		float ortho_height() const noexcept{
			return
				( viewport.bottom - viewport.top ) *
				ortho_scale;
		}
	};

	enum class EditorTools{
		None,
		TileSelect,
		Brush,
		Polyline,
	};


	class LevelEditor : public IAppState{
	public:
		LevelEditor( Rect<std::int32_t> const& viewport, LevelDocument& document_ );
		LevelEditor( LevelEditor const& ) = delete;
		LevelEditor& operator=( LevelEditor const& ) = delete;

		void update( Input& input_, float dt )override;
		void render( renderer2d& renderer_, Font const& font_ ) const override;

	private:
		class EditorMode;
		class LoadMode;
		class ResizeMode;
		class SaveMode;
		class SaveBeforeExitMode;
		class TextureSelectMode;
		class TilePaletteMode;

	private:
		void clamp_camera() noexcept;

		void resize_tilemap( dims2<std::int32_t> const& new_size );
		void transition_mode( std::unique_ptr<IMode> next_mode );
		void load_tileset_sprites();
		void place_tile( Mouse const& mouse );
		
		friend class TileMapView;
		friend struct LevelSerializer;

	private:
		LevelDocument& m_document;
		EditorCamera m_camera{};
		TileMapView m_tilemap_view;
		Rect<std::int32_t> m_viewport{};
		std::unordered_map<std::string, surface<ColorF>> m_textures;
		std::string m_current_filename;
		std::string m_selected_texture_name;
		std::unique_ptr<IMode> m_mode;
		std::unique_ptr<IMode> m_next_mode;
		std::int32_t m_selection_index = 0;
		vector2<std::int32_t> m_mouse_position{};
		std::int32_t m_active_tile_index = 0;
		// TODO: Implement an editor view.
		// It's task will be to render the editor UI and the tilemap, 
		// and handle mouse input for tile placement, selection, etc.
	};
}