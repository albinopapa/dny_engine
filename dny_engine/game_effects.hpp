#pragma once

#include "dny_physics.hpp"
#include "dny_primitive_generators.hpp"

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

