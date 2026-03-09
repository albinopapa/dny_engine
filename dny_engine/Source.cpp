#include "dny_font.hpp"
#include "dny_graphics.hpp"
#include "dny_image_loader.hpp"
#include "dny_math.hpp"
#include "dny_platform.hpp"
#include "dny_physics.hpp"
#include "dny_primitive_generators.hpp"
#include "dny_timer.hpp"

#include <array>
#include <algorithm>
#include <format>
#include <filesystem>
#include <numeric>
#include <string>
#include <vector>

using vertex_in = dny::primitives::pnu_vertex;
using vertex_out = dny::basic_vertex<
	dny::vector4<float>, 
	dny::vector3<float>, 
	dny::vector2<float>
>;

struct transform_constant_buffer{
	dny::matrix_4x4<float> world;
	dny::matrix_4x4<float> view;
	dny::matrix_4x4<float> projection;
};

class pnu_vertex_shader : public dny::basic_vertex_shader<
	vertex_in,
	vertex_out,
	transform_constant_buffer>{
public:
	static constexpr std::size_t Position_ID = 0;

	vertex_out operator()( vertex_in const& vin )const noexcept{
		vertex_out vout;

		auto pos = dny::vector4<float>{
			std::get<0>( vin.m_fields ),
			1.0f
		};

		pos = pos * cbuffer.world;
		pos = pos * cbuffer.view;
		pos = pos * cbuffer.projection;
		vout.m_fields = {
			pos,
			std::get<1>( vin.m_fields ),
			std::get<2>( vin.m_fields )
		};
		return vout;
	}
};

class pnu_pixel_shader : public dny::basic_pixel_shader<
	typename pnu_vertex_shader::vertex_out,
	dny::null_ps_constant_buffer,
	dny::bilinear_sampler>{
public:
	static constexpr std::size_t Position_ID = 0;
	dny::ColorF operator()( vertex_in const& vin )const noexcept{
		auto uv = std::get<2>( vin.m_fields );
		return sampler.sample( uv, texture[ 0 ] );
	}
};

using pnu_effect = dny::basic_effect<pnu_vertex_shader, pnu_pixel_shader>;
using pnu_pipeline_t = dny::pipeline<pnu_effect>;

using debug_vertex_in = dny::basic_vertex<dny::vector3<float>, dny::ColorF>;
using debug_vertex_out = dny::basic_vertex<dny::vector4<float>, dny::ColorF>;

class debug_vertex_shader : public dny::basic_vertex_shader<
	debug_vertex_in,
	debug_vertex_out,
	transform_constant_buffer>{
public:
	static constexpr std::size_t Position_ID = 0;

	debug_vertex_out operator()( debug_vertex_in const& vin )const noexcept{
		auto pos = dny::vector4<float>{ std::get<0>( vin.m_fields ), 1.f };
		pos = pos * cbuffer.world;
		pos = pos * cbuffer.view;
		pos = pos * cbuffer.projection;
		return debug_vertex_out{ { pos, std::get<1>( vin.m_fields ) } };
	}
};

class debug_pixel_shader : public dny::basic_pixel_shader<
	typename debug_vertex_shader::vertex_out,
	dny::null_ps_constant_buffer>{
public:
	static constexpr std::size_t Position_ID = 0;

	dny::ColorF operator()( vertex_in const& vin )const noexcept{
		return std::get<1>( vin.m_fields );
	}
};

using debug_effect = dny::basic_effect<debug_vertex_shader, debug_pixel_shader>;
using debug_pipeline_t = dny::pipeline<debug_effect>;

static constexpr float action_plane_z = 5.f;

class Player{
public:
	dny::vector2<float> const& get_position()const{
		return position;
	}
	void update( float dt, dny::input const& input, dny::polyline_collider<float> const& terrain_ ){
		const auto move_dir =
			( input.is_key_down( 'D' ) ? 1.f : 0.f ) -
			( input.is_key_down( 'A' ) ? 1.f : 0.f );

		position.x += move_dir * move_speed * dt;

		if( input.is_key_down( VK_SPACE ) && is_on_ground ){
			velocity.y = jump_velocity;
			is_on_ground = false;
		}

		velocity.y += gravity * dt;
		position.y += velocity.y * dt;

		auto player_bounds = dny::aabb<float>{
			{
				position.x - size.width * 0.5f,
				position.y - size.height * 0.5f,
				action_plane_z - 0.5f
			},
			{
				position.x + size.width * 0.5f,
				position.y + size.height * 0.5f,
				action_plane_z + 0.5f
			}
		};

		const auto resolution = dny::resolve_aabb_vs_polyline( player_bounds, terrain_, 0.75f );
		if( resolution.y != 0.f ){
			position.y += resolution.y;
			velocity.y = 0.f;
			is_on_ground = true;
		}
		else if( position.y < fallback_ground_height ){
			position.y = fallback_ground_height;
			velocity.y = 0.f;
			is_on_ground = true;
		}
		else{
			is_on_ground = false;
		}
	}
	dny::matrix_4x4<float> get_transform()const{
		auto rotation = dny::matrix_4x4<float>::rotation_x( dny::to_radians( 90.f ) );
		auto translation = dny::matrix_4x4<float>::translation( { position.x, position.y, action_plane_z } );
		auto scaling = dny::matrix_4x4<float>::scaling( { size.width, size.height, 1.f } );
		return rotation * scaling * translation;
	}
private:
	static constexpr float move_speed = 12.f;
	static constexpr float jump_velocity = 16.f;
	static constexpr float gravity = -36.f;
	static constexpr float fallback_ground_height = -4.f;
	static constexpr float dim = 4.f;
	static constexpr dny::dims2<float> size{ dim, dim };
	dny::vector2<float> position{ 0.f, fallback_ground_height };
	dny::vector2<float> velocity{ 0.f, 0.f };
	bool is_on_ground = true;

};

class lock_on_camera {
public:
	void update( dny::vector2<float> const& target_position, dny::aabb<float> const& world_bounds ) noexcept{
		position.x = std::clamp( target_position.x, world_bounds.min_pt.x, world_bounds.max_pt.x );
		position.y = std::clamp( target_position.y, world_bounds.min_pt.y, world_bounds.max_pt.y );
	}

	void zoom_in( float amount_degrees ) noexcept{
		fov = std::clamp( fov - amount_degrees, min_fov, max_fov );
	}

	void zoom_out( float amount_degrees ) noexcept{
		fov = std::clamp( fov + amount_degrees, min_fov, max_fov );
	}

	dny::matrix_4x4<float> get_view_matrix() const noexcept{
		return dny::look_to<float, dny::handedness_t::left>(
			position,
			forward,
			up
		);
	}

	float get_fov() const noexcept{
		return dny::to_radians( fov );
	}

	const dny::vector3<float>& get_position() const noexcept{
		return position;
	}

private:
	static constexpr float min_fov = 30.f;
	static constexpr float max_fov = 90.f;

	dny::vector3<float> position{ 0.f, 0.f, -10.f };
	dny::vector3<float> forward{ 0.f, 0.f, 1.f };
	dny::vector3<float> up{ 0.f, 1.f, 0.f };
	float fov = 90.f;
};

using frame_pack = std::vector<dny::surface<dny::ColorF>>;
using texture2d = dny::surface<dny::ColorF>;
class Game{
public:
	Game( dny::platform& platform_ )
		:
		platform( platform_ ){
		platform.set_title( L"DNY Engine - Software Renderer" );
		init_textures();
		init_debug_vertices();
	}
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

		if( platform.get_input().is_key_down( 'Q' ) ){
			camera.zoom_in( zoom_speed * dt );
		}
		if( platform.get_input().is_key_down( 'E' ) ){
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
		if(frame_count >= frame_times.size() ){
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

		renderer.set_pixel_shader_textures( 0, terrain_texture );
		for(const auto& pos : terrain_positions ){
			auto terrain_cb = transform_constant_buffer{
				dny::matrix_4x4<float>::translation( { pos.x, pos.y, action_plane_z } ),
				view_matrix,
				projection_matrix
			};
			renderer.set_cbuffer( terrain_cb );
			renderer.render( terrain_vbuffer );
		}

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

		const auto text_pos = dny::vector2<std::int32_t>{ 10, 10 };
				
		dny::draw(
			std::format( "FPS: {:.2f}", frame_rate ),
			text_pos,
			consolas,
			dny::Color32{ dny::Colors::white },
			render_target
		);
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


	void init_textures(){
		{
			const auto filename = "Assets/Textures/girl_step_ini.png";
			const auto data = 
				dny::load_image_data( std::filesystem::path{ filename } );
			girl_step = dny::surface<dny::ColorF>{ data.width, data.height };

			for( std::size_t j = 0; j < data.width * data.height; ++j ){
				const auto r = data.pixels[ j * data.channels_per_pixel + 2 ];
				const auto g = data.pixels[ j * data.channels_per_pixel + 1 ];
				const auto b = data.pixels[ j * data.channels_per_pixel + 0 ];
				const auto a = ( data.channels_per_pixel >= 4 ) ?
					data.pixels[ j * data.channels_per_pixel + 3 ] : 255ui8;

				girl_step.pixels()[ j ] = dny::ColorF{ r, g, b, a };
			}
		}

		for( auto i = 0; i < 30; ++i ){
			const auto filename =
				std::format( "Assets/Textures/girl_walking{:02}.png", i );
			const auto data =
				dny::load_image_data( std::filesystem::path{ filename } );
			auto& frame =
				girl_walking_frames.emplace_back( data.width, data.height );

			for( std::size_t j = 0; j < data.width * data.height; ++j ){
				const auto r = data.pixels[ j * data.channels_per_pixel + 2 ];
				const auto g = data.pixels[ j * data.channels_per_pixel + 1 ];
				const auto b = data.pixels[ j * data.channels_per_pixel + 0 ];
				const auto a = ( data.channels_per_pixel >= 4 ) ?
					data.pixels[ j * data.channels_per_pixel + 3 ] : 255ui8;
				frame.pixels()[ j ] = dny::ColorF{ r, g, b, a };
			}
		}

		// Load terrain texture
		{
			const auto filename = "Assets/Textures/test.png";
			const auto data =
				dny::load_image_data( std::filesystem::path{ filename } );
			terrain_texture = dny::surface<dny::ColorF>{ data.width, data.height };

			for( std::size_t j = 0; j < data.width * data.height; ++j ){
				const auto r = data.pixels[ j * data.channels_per_pixel + 2 ];
				const auto g = data.pixels[ j * data.channels_per_pixel + 1 ];
				const auto b = data.pixels[ j * data.channels_per_pixel + 0 ];
				const auto a = ( data.channels_per_pixel >= 4 ) ?
					data.pixels[ j * data.channels_per_pixel + 3 ] : 255ui8;
				terrain_texture.pixels()[ j ] = dny::ColorF{ r, g, b, a };
			}

		}

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
		{ -2.f, -2.f, action_plane_z },
		{ -1.f, -2.f, action_plane_z },
		{  0.f, -2.f, action_plane_z },
		{  1.f, -2.f, action_plane_z },
		{  2.f, -2.f, action_plane_z }
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
};

std::int32_t main() {
	auto platform = dny::platform{};
	auto game = Game{ platform };
	
	while( !platform.is_done() ){
		platform.process_message_pump();
		game.run();
	}

	return 0;
}
