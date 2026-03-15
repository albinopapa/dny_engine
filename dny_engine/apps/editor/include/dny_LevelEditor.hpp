#pragma once

#include "dny_IAppState.hpp"
#include "dny_IMode.hpp"
#include "dny_Level.hpp"
#include "dny_TileMap.hpp"
#include "dny_Utils.hpp"

#include "core/rectangle.hpp"
#include "core/surface.hpp"
#include "math/vector2.hpp"
#include "ui/dny_ui.hpp"

#include <array>
#include <cassert>
#include <optional>
#include <string_view>
#include <unordered_map>


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
		Brush,
		Polyline,
	};

	class LevelEditor;
	struct LevelDocument{
		TileMap tilemap;

		std::vector<TileDef> tile_defs;
		std::vector<PlatformDef> platform_defs;
		bool m_player_spawn_used = false;

		// future
		// std::vector<Spawn>
		// std::vector<Trigger>
		// std::vector<Region>
		// std::vector<Entity>
	};

	// Simple view over the editable tilemap region (left side)
	class TileMapView{
	public:
		TileMapView() = default;
		TileMapView( LevelEditor& editor, Rect<std::int32_t> const& area ) noexcept;

		void render( EditorCamera const& cam ) const;
		std::optional<vector2<std::int32_t>> tile_at( vector2<std::int32_t> const& screen_pos, EditorCamera const& cam ) const noexcept;
		void set_tile( vector2<std::int32_t> const& tile_index, Tile const& tile );

	private:
		LevelEditor* m_editor = nullptr;
		Rect<std::int32_t> m_area{};
		// Layout constants
		static constexpr std::int32_t m_tile_size = Tile::size;
	};

	// Palette of selectable tiles (right side)
	class TilePalette{
	public:
		TilePalette() = default;
		TilePalette( LevelEditor& editor, Rect<std::int32_t> const& area ) noexcept;

		void render() const;
		std::optional<std::size_t> hit_test( vector2<std::int32_t> const& screen_pos ) const noexcept;

		std::size_t get_active_index()const noexcept{
			return m_active_index;
		}
		void set_active_index( std::size_t index ) noexcept;
		Tile const& active_tile() const noexcept;

		std::size_t tile_count() noexcept{
			return m_tileset.size();
		}

		Tile const& tile_at( std::size_t index )const noexcept{
			assert( index < m_tileset.size() );
			return m_tileset[ index ];
		}

		Tile& tile_at( std::size_t index )noexcept{
			assert( index >= 0 && index < m_tileset.size() );
			return m_tileset[ index ];
		}
	private:
		static constexpr std::int32_t m_tile_size = Tile::size;
		static constexpr std::int32_t m_tile_spacing = 10;

		// Order matters: must match rendering / hit-test order
		std::array<std::int32_t, 8> m_tileset = std::array{
			0, // Empty
			1, // Dirt
			2, // Platform
			3, // Water
			4, // Lava
			5, // PlayerSpawn
			6, // HopperSpawn
			7  // FireBallSpawn
		};

		LevelEditor* m_editor = nullptr;
		Rect<std::int32_t> m_area{};
		std::size_t m_active_index = 1; // Default to Solid
	};

	class LevelEditor : public IAppState<dny_Engine_Request>{
	public:
		LevelEditor();
		LevelEditor( LevelEditor const& ) = delete;
		LevelEditor& operator=( LevelEditor const& ) = delete;

		void update( float dt )override;
		void render() const override;

	private:
		class EditorMode;
		class LoadMode;
		class ResizeMode;
		class SaveMode;
		class SaveBeforeExitMode;
		class TextureSelectMode;

	private:
		void handle_mouse();
		void handle_keyboard();
		void clamp_camera() noexcept;

		void resize_tilemap( dims2<std::int32_t> const& new_size );
		void transition_mode( std::unique_ptr<IMode> next_mode );
		void load_tileset_sprites();
		void place_tile();
		friend class TilePalette;
		friend class TileMapView;
		friend class TileMapSerializer;
	private:
		static constexpr Rect<std::int32_t> m_editor_area = { 0, 0, 1216, 720 };
		static constexpr Rect<std::int32_t> m_tileset_area = { m_editor_area.right, m_editor_area.top, 1280, 720 };

		LevelDocument m_document;
		EditorCamera m_camera{};

		//std::string m_current_filename;
		//std::unordered_map<std::int32_t, surface<Color32>> m_tileset_sprites;
		//TileMapView m_view;
		//TilePalette m_palette;
		//std::unique_ptr<IMode> m_mode, m_next_mode;
		//ui::Panel m_layout;
		//bool m_map_changed = false;
	};
}