#pragma once

#include "../dny/graphics/camera.hpp"
#include "../dny/renderer/effects.hpp"
#include "../../apps/runtime/include/player.hpp"

#include "../dny/audio/audio.hpp"
#include "../dny/graphics/font.hpp"
#include "../dny/math/math.hpp"
#include "../dny/platform/platform.hpp"
#include "../dny/core/timer.hpp"

#include <filesystem>
#include <vector>

using frame_pack = std::vector<dny::surface<dny::ColorF>>;
using texture2d = dny::surface<dny::ColorF>;
class Game{
public:
	Game( dny::platform& platform_ );
	~Game();
	
	void run();

private:
	void begin_frame();
	void update();
	void render();
	void render_debug_colliders();
	void render_terrain();
	void render_player();
	void init_debug_vertices();
	void end_frame();
	texture2d image_data_to_texture( std::filesystem::path filename_ );
	void init_textures();

private:
	static constexpr std::int32_t view_width = dny::screen_width / 2;
	static constexpr std::int32_t view_height = dny::screen_height / 2;
	static constexpr float aspect_ratio = static_cast< float >( view_width ) / static_cast< float >( view_height );
	static constexpr dny::dims2<float> cube_size{ 50.f, 50.f };
	static constexpr float zoom_speed = 50.f;

	// Reference to the platform for window management and input
	dny::platform& platform;

	// The software renderer pipeline
	pnu_pipeline_t renderer;
	debug_pipeline_t debug_renderer;
	using pnu_vertex_buffer = std::vector<vertex_in>;

	// Render target and depth buffer
	dny::surface<dny::Color32> render_target =
		dny::surface<dny::Color32>{ view_width, view_height };
	std::vector<float> depth_buffer =
		std::vector<float>( render_target.width() * render_target.height(), 1.f );


	dny::matrix_4x4<float> view_matrix = dny::matrix_4x4<float>::identity();
	dny::matrix_4x4<float> projection_matrix = dny::projection<dny::handedness_t::left>(
		dny::to_radians( 90.f ),
		aspect_ratio,
		0.1f,
		100.f
	);

	pnu_vertex_buffer girl_vbuffer = dny::primitives::generate_plane();
	frame_pack girl_walking_frames;
	pnu_vertex_buffer terrain_vbuffer = dny::primitives::generate_cube();
	std::vector<debug_vertex_in> debug_vertices;
	texture2d terrain_texture;
	texture2d girl_step;

	Player player;
	lock_on_camera camera;
	dny::aabb<float> world_bounds{
		{ -8.f, -4.f, -10.f },
		{ 8.f, 8.f, -10.f }
	};
	std::vector<dny::vector3<float>> terrain_positions{
		{ -2.f, -2.f, 0.f },
		{ -1.f, -2.f, 0.f },
		{  0.f, -2.f, 0.f },
		{  1.f, -2.f, 0.f },
		{  2.f, -2.f, 0.f }
	};
	dny::polyline_collider<float> terrain_collider = dny::generate_polyline_collider<float>( {
		{ -8.f, -4.f },
		{ -4.f, -4.f },
		{ -1.f, -2.5f },
		{ 2.f, -1.f },
		{ 5.f, -1.75f },
		{ 8.f, -1.75f }
		} );
	bool debug_draw_colliders = true;

	dny::Timer timer;
	std::size_t frame_count = 0;
	std::array<float, 100> frame_times = {};
	dny::Font consolas{ L"Consolas", 24 };
	float frame_rate = 0.f;

	// Audio system - currently disabled due to incomplete type issues
	// To use audio, you must define Game destructor in game.cpp where dny_audio.cpp is visible
	dny::audio_engine audio;
	dny::audio_clip jump_sound;
	dny::audio_clip background_music;
};
