#include "dny_platform.hpp"
#include "dny_soft_renderer_tiled.hpp"

using vertex_in =
dny::basic_vertex<dny::vector3<float>, dny::vector2<float>, dny::ColorF>;
using vertex_out =
dny::basic_vertex<dny::vector4<float>, dny::vector2<float>, dny::ColorF>;


class my_vertex_shader : public dny::basic_vertex_shader<
	vertex_in,
	vertex_out,
	dny::null_vs_constant_buffer>{
public:
	static constexpr std::size_t Position_ID = 0;
	my_vertex_shader() = default;

	vertex_out operator()( vertex_in const& vin )const noexcept{
		vertex_out vout;
		vout.m_fields = {
			dny::vector4<float>{ std::get<0>( vin.m_fields ), 1.0f },
			std::get<1>( vin.m_fields ),
			std::get<2>( vin.m_fields )
		};
		return vout;
	}
};

class my_pixel_shader : public dny::basic_pixel_shader<
	typename my_vertex_shader::vertex_out,
	dny::null_ps_constant_buffer,
	dny::null_sampler>{
public:
	static constexpr std::size_t Position_ID = 0;
	my_pixel_shader() = default;
	dny::ColorF operator()( vertex_in const& vin )const noexcept{
		return std::get<2>( vin.m_fields );
	}
};

std::int32_t main() {
	using effect = dny::basic_effect<my_vertex_shader, my_pixel_shader>;
	using pipeline_t = dny::pipeline<effect>;

	auto vbuffer = std::vector<vertex_in>{};
	vbuffer.reserve( 3 );
	{
		auto& vert = vbuffer.emplace_back();
		vert.m_fields = {
			dny::vector3<float>{ 0.0f, 0.5f, 0.0f },
			dny::vector2<float>{ 0.5f, 0.0f },
			dny::Colors::red
		};
	}
	{
		auto& vert = vbuffer.emplace_back();
		vert.m_fields = {
			dny::vector3<float>{ 0.5f, -0.5f, 0.0f },
			dny::vector2<float>{ 1.0f, 1.0f },
			dny::Colors::green
		};
	}
	{
		auto& vert = vbuffer.emplace_back();
		vert.m_fields = {
			dny::vector3<float>{ -0.5f, -0.5f, 0.0f },
			dny::vector2<float>{ 0.0f, 1.0f },
			dny::Colors::blue
		};
	}

	auto render_target = dny::surface<dny::Color32>{
		dny::screen_width / 2,
		dny::screen_height / 2
	};

	auto depth_buffer = std::vector<float>( render_target.width() * render_target.height(), 1.f );

	dny::platform platform;
	pipeline_t renderer;
	renderer.set_render_target( render_target );
	renderer.set_depth_buffer( depth_buffer );


	while( !platform.is_done() ){
		platform.process_message_pump();

		auto pixel_span = std::span{ render_target.pixels(), render_target.width() * render_target.height() };
		std::fill( pixel_span.begin(), pixel_span.end(), dny::Color32{ 0, 0, 0, 255 } );
		depth_buffer.assign( depth_buffer.size(), 1.f );
		renderer.render( vbuffer );

		platform.update_view(
			render_target.width(),
			render_target.height(),
			std::span{ render_target.pixels(), render_target.width() * render_target.height() }
		);
	}

	return 0;
}