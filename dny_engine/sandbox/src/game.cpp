#include "game.hpp"

#include "../dny/graphics/image_loader.hpp"
#include "../dny/graphics/graphics.hpp"

#include <format>
#include <numeric>

Game::Game( dny::platform& platform_ )
	:
	platform( platform_ ),
	debug_panel( "", "Debug", {}, { 500, 100 } )
{
	platform.set_title( L"DNY Engine - Software Renderer" );
	auto& input = platform.get_input();
	input.bind( "move_left", dny::Key::A );
	input.bind( "move_right", dny::Key::D );
	input.bind( "jump", dny::Key::Space );
	input.bind( "zoom_in", dny::Key::Q );
	input.bind( "zoom_out", dny::Key::E );
	input.bind( "move_ground_away", dny::Key::Up );
	input.bind( "move_ground_closer", dny::Key::Down );
	input.bind( "fire", dny::MouseButton::Left );
	input.bind( "dash", dny::GamepadButton::A );
	init_textures();
	init_debug_vertices();
}

Game::~Game() = default;

void Game::run(){
	begin_frame();
	update();
	render();
	end_frame();
}

void Game::begin_frame(){
	auto pixel_span = std::span{ render_target.pixels(), render_target.width() * render_target.height() };
	std::fill( pixel_span.begin(), pixel_span.end(), dny::Color32{ 0, 0, 0, 255 } );
	depth_buffer.assign( depth_buffer.size(), 1.f );
}

void Game::update(){
	const auto dt = timer.mark();
	player.update( dt, platform.get_input(), terrain_collider );

	if( platform.get_input().is_held( "zoom_in" ) ){
		camera.zoom_in( zoom_speed * dt );
	}
	if( platform.get_input().is_held( "zoom_out" ) ){
		camera.zoom_out( zoom_speed * dt );
	}
	if(platform.get_input().is_held( "move_ground_away" ) ){
		terrain_position.z += .25f * zoom_speed * dt;
	}
	if( platform.get_input().is_held( "move_ground_closer" ) ){
		terrain_position.z -= .25f * zoom_speed * dt;
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

void Game::render(){
	view_matrix = camera.get_view_matrix();

	render_debug_colliders();
	render_player();
	render_terrain();

	renderer2d.begin( render_target, depth_buffer );
	auto text_pos = dny::vector2<float>{ 
		10.f, 
		static_cast<float>( consolas.char_height() )
	};
	debug_panel.draw( render_target, consolas );
	renderer2d.draw_text(
		std::format( "FPS: {:.2f}", frame_rate ),
		text_pos,
		arial,
		dny::Colors::white
	);
	text_pos.y += arial.char_height();
	renderer2d.draw_text(
		std::format( "Ground Z: {:.2f}", terrain_position.z ),
		text_pos,
		arial,
		dny::Colors::white
	);
	//const auto text_pos = dny::vector2<std::int32_t>{ 10, 10 };
	//dny::draw_text(
	//	std::format( "FPS: {:.2f}", frame_rate ),
	//	text_pos,
	//	consolas,
	//	dny::Color32{ dny::Colors::white },
	//	render_target
	//);
}

void Game::render_debug_colliders(){
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

void Game::render_terrain(){
	renderer.set_render_target( render_target );
	renderer.set_depth_buffer( depth_buffer );
	renderer.set_pixel_shader_textures( 0, terrain_texture );

	auto world =
		dny::matrix_4x4<float>::scaling( { 4.f, 1.f, 4.f } ) *
		dny::matrix_4x4<float>::translation( terrain_position );
	auto terrain_cb = transform_constant_buffer{
		.world      = world,
		.view       = view_matrix,
		.projection = projection_matrix
	};
	renderer.set_cbuffer( terrain_cb );
	renderer.render( terrain_vbuffer );

	//for( const auto& pos : terrain_positions ){
	//	auto world =
	//		dny::matrix_4x4<float>::scaling( { 1.f, 1.f, 2.f } ) *
	//		dny::matrix_4x4<float>::translation( { pos.x, pos.y, pos.z } );
	//	auto terrain_cb = transform_constant_buffer{
	//		.world      = world,
	//		.view       = view_matrix,
	//		.projection = projection_matrix
	//	};
	//	renderer.set_cbuffer( terrain_cb );
	//	renderer.render( terrain_vbuffer );
	//}
}

void Game::render_player(){
	auto cb = transform_constant_buffer{
		player.get_transform(),
		view_matrix,
		projection_matrix
	};
	//renderer.set_pixel_shader_textures( 0, girl_walking_frames[ 0 ] );
	sprite_renderer.set_pixel_shader_textures( 0, girl_step );
	sprite_renderer.set_render_target( render_target );
	sprite_renderer.set_depth_buffer( depth_buffer );

	sprite_renderer.set_cbuffer( cb );
	sprite_renderer.render( girl_vbuffer );
}

void Game::init_debug_vertices(){
	debug_vertices.clear();
	constexpr auto debug_color = dny::ColorF{ dny::Colors::chartreuse };
	constexpr auto debug_line_thickness = 0.25f;

	for( std::size_t i = 0; i + 1 < terrain_collider.points.size(); ++i ){
		const auto start = terrain_collider.points[ i ];
		const auto end = terrain_collider.points[ i + 1 ];

		auto segment_vertices = dny::primitives::generate_debug_line_segment(
			dny::vector3<float>{ start.x, start.y, 1.f },
			dny::vector3<float>{ end.x,   end.y,   1.f },
			debug_line_thickness,
			debug_color
		);

		for( auto const& vertex : segment_vertices ){
			debug_vertices.push_back( debug_vertex_in{ { vertex.position, vertex.color } } );
		}
	}
}

void Game::end_frame(){
	platform.update_view(
		render_target.width(),
		render_target.height(),
		std::span{ render_target.pixels(), render_target.width() * render_target.height() }
	);
}

texture2d Game::image_data_to_texture( std::filesystem::path filename_ ){
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

void Game::init_textures(){
	girl_step = image_data_to_texture(
		std::filesystem::path{ "assets/textures/n_girl/girl_step_ini.png" }
	);

	for( auto i = 0; i < 30; ++i ){
		const auto filename = std::filesystem::path{
			std::format( "assets/textures/n_girl/girl_walking{:02}.png", i )
		};
		girl_walking_frames.emplace_back(
			image_data_to_texture( filename )
		);
	}

	// Load terrain texture
	terrain_texture = image_data_to_texture(
		std::filesystem::path{ "assets/textures/test/test.png" }
	);
}
