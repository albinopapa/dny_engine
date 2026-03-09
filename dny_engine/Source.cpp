#include "dny_font.hpp"
#include "dny_graphics.hpp"
#include "dny_image_loader.hpp"
#include "dny_math.hpp"
#include "dny_platform.hpp"
#include "dny_physics.hpp"
#include "dny_primitive_generators.hpp"
#include "dny_timer.hpp"

#include <array>
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

		auto player_bounds = dny::Rect<float>{
			position.x - size.width * 0.5f,
			position.y - size.height * 0.5f,
			position.x + size.width * 0.5f,
			position.y + size.height * 0.5f
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

using frame_pack = std::vector<dny::surface<dny::ColorF>>;
using texture2d = dny::surface<dny::ColorF>;
class Game{
public:
	Game( dny::platform& platform_ )
		:
		platform( platform_ ){
		platform.set_title( L"DNY Engine - Software Renderer" );
		init_textures();
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
		camera_position = {
			player.get_position().x,
			player.get_position().y,
			camera_distance
		};

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
		const auto player_pos = dny::vector3<float>( player.get_position(), action_plane_z );
		view_matrix = dny::look_at<float, dny::handedness_t::left>(
			camera_position,
			player_pos,
			dny::vector3<float>{ 0.f, 1.f, 0.f }
		);
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

		if( debug_draw_colliders ){
			draw_terrain_polyline_debug();
		}

		const auto text_pos = dny::vector2<std::int32_t>{ 10, 10 };

		dny::draw(
			"0123456789 ABCDEFGHIJ KLMNOPQRST UVWXYZ",
			text_pos,
			consolas,
			dny::Color32{ dny::Colors::white },
			render_target
		);
		//dny::draw(
		//	std::format( "FPS: {:.2f}", frame_rate ),
		//	dny::vector2<std::int32_t>{ 10, 10 },
		//	consolas,
		//	dny::Color32{ dny::Colors::white },
		//	render_target
		//);
	}

	void draw_terrain_polyline_debug(){
		auto project = [ this ]( dny::vector2<float> const& point_ ){
			return dny::vector2<std::int32_t>{
				static_cast<std::int32_t>( 40.f + ( point_.x + 8.f ) * 18.f ),
				static_cast<std::int32_t>( static_cast<float>( render_target.height() ) - ( 40.f + ( point_.y + 6.f ) * 18.f ) )
			};
		};

		for( std::size_t i = 0; i + 1 < terrain_collider.points.size(); ++i ){
			dny::draw_line(
				project( terrain_collider.points[ i ] ),
				project( terrain_collider.points[ i + 1 ] ),
				dny::Color32{ dny::Colors::chartreuse },
				render_target
			);
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
	static constexpr float camera_speed = 25.f;

	// Reference to the platform for window management and input
	dny::platform& platform;

	// The software renderer pipeline
	pnu_pipeline_t renderer;
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
	texture2d terrain_texture;
	texture2d girl_step;
	
	Player player;
	dny::vector3<float> camera_position{ 0.f, 0.f, -10.f };
	float camera_distance = -10.f;
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
