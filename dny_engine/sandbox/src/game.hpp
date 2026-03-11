#pragma once

#include "game_camera.hpp"
#include "game_effects.hpp"
#include "game_player.hpp"

#include "dny_audio.hpp"
#include "dny_font.hpp"
#include "dny_graphics.hpp"
#include "dny_image_loader.hpp"
#include "dny_math.hpp"
#include "dny_platform.hpp"
#include "dny_timer.hpp"

#include <filesystem>
#include <format>
#include <numeric>
#include <vector>

using frame_pack = std::vector<dny::surface<dny::ColorF>>;
using texture2d = dny::surface<dny::ColorF>;
class Game{
public:
	Game( dny::platform& platform_ );
	~Game();
	
	void run(){
		begin_frame();
		update();
		render();
		end_frame();
	}

private:
	void begin_frame(){
		auto pixel_span = std::span{ render_target.pixels(), render_target.width() * render_target.height() };
		std::fill( pixel_span.begin(), pixel_span.end(), dny::Color32{ 0, 0, 0, 255 } );
		depth_buffer.assign( depth_buffer.size(), 1.f );
	}
	void update(){
		const auto dt = timer.mark();
		player.update( dt, platform.get_input(), terrain_collider );

		if( platform.get_input().is_held( "zoom_in" ) ){
			camera.zoom_in( zoom_speed * dt );
		}
		if( platform.get_input().is_held( "zoom_out" ) ){
			camera.zoom_out( zoom_speed * dt );
		}

		camera.update( player.get_position(), world_bounds );
		projection_matrix = dny::projection<dny::handedness_t::left>(
			camera.get_fov(),
			aspect_ratio,
			0.1f,
			100.f
		);

		frame_times[ ( frame_count++ ) % frame_times.size() ] = dt;
		if( frame_count >= frame_times.size() ){
			const auto sum = std::accumulate(
				frame_times.begin(),
				frame_times.end(),
				0.f
			);
			frame_rate = static_cast< float >( frame_times.size() ) / sum;
			frame_count = 0;
		}
	}
	void render(){
		view_matrix = camera.get_view_matrix();

		render_debug_colliders();
		render_player();
		render_terrain();

		const auto text_pos = dny::vector2<std::int32_t>{ 10, 10 };

		dny::draw_text(
			std::format( "FPS: {:.2f}", frame_rate ),
			text_pos,
			consolas,
			dny::Color32{ dny::Colors::white },
			render_target
		);
	}

	void render_debug_colliders(){
		if( debug_draw_colliders && !debug_vertices.empty() ){
			auto debug_cb = transform_constant_buffer{
				dny::matrix_4x4<float>::identity(),
				view_matrix,
				projection_matrix
			};
			debug_renderer.set_render_target( render_target );
			debug_renderer.set_depth_buffer( depth_buffer );
			debug_renderer.set_cbuffer( debug_cb );
			debug_renderer.render( debug_vertices );
		}
	}

	void render_terrain(){
		renderer.set_render_target( render_target );
		renderer.set_depth_buffer( depth_buffer );
		renderer.set_pixel_shader_textures( 0, terrain_texture );


		for( const auto& pos : terrain_positions ){
			auto world =
				dny::matrix_4x4<float>::scaling( { 1.f, 1.f, 2.f } ) *
				dny::matrix_4x4<float>::translation( { pos.x, pos.y, action_plane_z } );
			auto terrain_cb = transform_constant_buffer{
				.world      = world,
				.view       = view_matrix,
				.projection = projection_matrix
			};
			renderer.set_cbuffer( terrain_cb );
			renderer.render( terrain_vbuffer );
		}
	}

	void render_player(){
		auto cb = transform_constant_buffer{
			player.get_transform(),
			view_matrix,
			projection_matrix
		};
		//renderer.set_pixel_shader_textures( 0, girl_walking_frames[ 0 ] );
		renderer.set_pixel_shader_textures( 0, girl_step );
		renderer.set_render_target( render_target );
		renderer.set_depth_buffer( depth_buffer );

		renderer.set_cbuffer( cb );
		renderer.render( girl_vbuffer );
	}

	void init_debug_vertices(){
		debug_vertices.clear();
		constexpr auto debug_color = dny::ColorF{ dny::Colors::chartreuse };
		constexpr auto debug_line_thickness = 0.25f;

		for( std::size_t i = 0; i + 1 < terrain_collider.points.size(); ++i ){
			const auto start = terrain_collider.points[ i ];
			const auto end = terrain_collider.points[ i + 1 ];

			auto segment_vertices = dny::primitives::generate_debug_line_segment(
				dny::vector3<float>{ start.x, start.y, 0.f },
				dny::vector3<float>{ end.x, end.y, 0.f },
				debug_line_thickness,
				debug_color
			);

			for( auto const& vertex : segment_vertices ){
				debug_vertices.push_back( debug_vertex_in{ { vertex.position, vertex.color } } );
			}
		}
	}
	void end_frame(){
		platform.update_view(
			render_target.width(),
			render_target.height(),
			std::span{ render_target.pixels(), render_target.width() * render_target.height() }
		);
	}

	texture2d image_data_to_texture( std::filesystem::path filename_ ){
		const auto data = dny::load_image_data( filename_ );
		auto result = texture2d{ data.width, data.height };

		for( std::size_t j = 0; j < data.width * data.height; ++j ){
			const auto r = data.pixels[ j * data.channels_per_pixel + 2 ];
			const auto g = data.pixels[ j * data.channels_per_pixel + 1 ];
			const auto b = data.pixels[ j * data.channels_per_pixel + 0 ];
			const auto a = ( data.channels_per_pixel >= 4 ) ?
				data.pixels[ j * data.channels_per_pixel + 3 ] : 255ui8;
			result.pixels()[ j ] = dny::ColorF{ r, g, b, a };
		}

		return result;
	}
	void init_textures(){
		girl_step = image_data_to_texture(
			std::filesystem::path{ "Assets/Textures/girl_step_ini.png" }
		);

		for( auto i = 0; i < 30; ++i ){
			const auto filename = std::filesystem::path{
				std::format( "Assets/Textures/girl_walking{:02}.png", i )
			};
			girl_walking_frames.emplace_back(
				image_data_to_texture( filename )
			);
		}

		// Load terrain texture
		terrain_texture = image_data_to_texture(
			std::filesystem::path{ "Assets/Textures/test.png" }
		);
	}
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
	// dny::audio_engine audio;
	// dny::audio_clip jump_sound;
	// dny::audio_clip background_music;
};
