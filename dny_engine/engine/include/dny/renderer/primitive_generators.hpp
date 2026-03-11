#pragma once

#include "math/math.hpp"
#include "soft_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace dny::primitives{
	using pnu_vertex = dny::basic_vertex<dny::vector3<float>, dny::vector3<float>, dny::vector2<float>>;

	struct debug_vertex{
		dny::vector3<float> position;
		ColorF color;

		constexpr debug_vertex& operator+=( debug_vertex const& other )noexcept{
			position += other.position;
			color += other.color;
			return *this;
		}
		constexpr debug_vertex& operator-=( debug_vertex const& other )noexcept{
			position -= other.position;
			color -= other.color;
			return *this;
		}
		constexpr debug_vertex& operator*=( float factor )noexcept{
			position *= factor;
			color *= factor;
			return *this;
		}
		constexpr debug_vertex& operator/=( float factor )noexcept{
			position /= factor;
			color /= factor;
			return *this;
		}
	};

	inline auto make_vertex(
		dny::vector3<float> const& position,
		dny::vector3<float> const& normal,
		dny::vector2<float> const& texcoord ) -> pnu_vertex{
		return pnu_vertex{ { position, normal, texcoord } };
	}

	inline auto make_debug_vertex(
		dny::vector3<float> const& position,
		ColorF const& color ) -> debug_vertex{
		return debug_vertex{ position, color };
	}

	inline auto generate_debug_line_segment(
		dny::vector3<float> const& start,
		dny::vector3<float> const& end,
		float thickness,
		ColorF const& color ) -> std::vector<debug_vertex>{
		auto start_xy = start;
		auto end_xy = end;
		start_xy.z = 0.0f;
		end_xy.z = 0.0f;

		const auto direction = end_xy - start_xy;
		const auto length_sq = dny::dot( direction, direction );

		if( length_sq <= 0.0f || thickness <= 0.0f ){
			return {};
		}

		const auto half_thickness = thickness * 0.5f;
		const auto inv_length = 1.0f / std::sqrt( length_sq );
		const auto unit_direction = direction * inv_length;
		const auto perpendicular = dny::vector3<float>{ -unit_direction.y, unit_direction.x, 0.0f } * half_thickness;

		const auto p0 = dny::vector3<float>{ start_xy.x + perpendicular.x, start_xy.y + perpendicular.y, 0.0f };
		const auto p1 = dny::vector3<float>{ start_xy.x - perpendicular.x, start_xy.y - perpendicular.y, 0.0f };
		const auto p2 = dny::vector3<float>{ end_xy.x + perpendicular.x, end_xy.y + perpendicular.y, 0.0f };
		const auto p3 = dny::vector3<float>{ end_xy.x - perpendicular.x, end_xy.y - perpendicular.y, 0.0f };

		std::vector<debug_vertex> vertices;
		vertices.reserve( 6 );

		vertices.push_back( make_debug_vertex( p0, color ) );
		vertices.push_back( make_debug_vertex( p1, color ) );
		vertices.push_back( make_debug_vertex( p2, color ) );

		vertices.push_back( make_debug_vertex( p2, color ) );
		vertices.push_back( make_debug_vertex( p1, color ) );
		vertices.push_back( make_debug_vertex( p3, color ) );

		return vertices;
	}

	inline auto generate_plane( float width = 1.0f, float depth = 1.0f ) -> std::vector<pnu_vertex>{
		const auto half_width = width * 0.5f;
		const auto half_depth = depth * 0.5f;
		const auto normal = dny::vector3<float>{ 0.0f, 1.0f, 0.0f };

		std::vector<pnu_vertex> vertices;
		vertices.reserve( 6 );

		const auto p0 = dny::vector3<float>{ -half_width, 0.0f, -half_depth };
		const auto p1 = dny::vector3<float>{ +half_width, 0.0f, -half_depth };
		const auto p2 = dny::vector3<float>{ +half_width, 0.0f, +half_depth };
		const auto p3 = dny::vector3<float>{ -half_width, 0.0f, +half_depth };

		vertices.push_back( make_vertex( p0, normal, { 0.0f, 1.0f } ) );
		vertices.push_back( make_vertex( p1, normal, { 1.0f, 1.0f } ) );
		vertices.push_back( make_vertex( p2, normal, { 1.0f, 0.0f } ) );

		vertices.push_back( make_vertex( p0, normal, { 0.0f, 1.0f } ) );
		vertices.push_back( make_vertex( p2, normal, { 1.0f, 0.0f } ) );
		vertices.push_back( make_vertex( p3, normal, { 0.0f, 0.0f } ) );

		return vertices;
	}

	inline auto generate_cube( float size = 1.0f ) -> std::vector<pnu_vertex>{
		const auto h = size * 0.5f;

		struct face_data{
			dny::vector3<float> normal;
			dny::vector3<float> corners[ 4 ];
		};

		const face_data faces[] = {
			{ { 0.0f, 0.0f, 1.0f }, { { -h, -h, +h }, { +h, -h, +h }, { +h, +h, +h }, { -h, +h, +h } } },
			{ { 0.0f, 0.0f, -1.0f }, { { +h, -h, -h }, { -h, -h, -h }, { -h, +h, -h }, { +h, +h, -h } } },
			{ { 1.0f, 0.0f, 0.0f }, { { +h, -h, +h }, { +h, -h, -h }, { +h, +h, -h }, { +h, +h, +h } } },
			{ { -1.0f, 0.0f, 0.0f }, { { -h, -h, -h }, { -h, -h, +h }, { -h, +h, +h }, { -h, +h, -h } } },
			{ { 0.0f, 1.0f, 0.0f }, { { -h, +h, +h }, { +h, +h, +h }, { +h, +h, -h }, { -h, +h, -h } } },
			{ { 0.0f, -1.0f, 0.0f }, { { -h, -h, -h }, { +h, -h, -h }, { +h, -h, +h }, { -h, -h, +h } } }
		};

		std::vector<pnu_vertex> vertices;
		vertices.reserve( 36 );

		for( auto const& face : faces ){
			vertices.push_back( make_vertex( face.corners[ 0 ], face.normal, { 0.0f, 1.0f } ) );
			vertices.push_back( make_vertex( face.corners[ 1 ], face.normal, { 1.0f, 1.0f } ) );
			vertices.push_back( make_vertex( face.corners[ 2 ], face.normal, { 1.0f, 0.0f } ) );

			vertices.push_back( make_vertex( face.corners[ 0 ], face.normal, { 0.0f, 1.0f } ) );
			vertices.push_back( make_vertex( face.corners[ 2 ], face.normal, { 1.0f, 0.0f } ) );
			vertices.push_back( make_vertex( face.corners[ 3 ], face.normal, { 0.0f, 0.0f } ) );
		}

		return vertices;
	}

	inline auto generate_sphere(
		float radius = 0.5f,
		std::uint32_t slices = 32,
		std::uint32_t stacks = 16 ) -> std::vector<pnu_vertex>{
		slices = std::max( 3u, slices );
		stacks = std::max( 2u, stacks );

		std::vector<pnu_vertex> vertices;
		vertices.reserve( static_cast<std::size_t>( slices ) * static_cast<std::size_t>( stacks ) * 6ull );

		for( std::uint32_t stack = 0; stack < stacks; ++stack ){
			const auto v0 = static_cast<float>( stack ) / static_cast<float>( stacks );
			const auto v1 = static_cast<float>( stack + 1u ) / static_cast<float>( stacks );
			const auto phi0 = dny::PI * v0;
			const auto phi1 = dny::PI * v1;

			for( std::uint32_t slice = 0; slice < slices; ++slice ){
				const auto u0 = static_cast<float>( slice ) / static_cast<float>( slices );
				const auto u1 = static_cast<float>( slice + 1u ) / static_cast<float>( slices );
				const auto theta0 = dny::PI2 * u0;
				const auto theta1 = dny::PI2 * u1;

				auto make_sphere_vertex = [ & ]( float theta, float phi, float u, float v ){
					const auto sin_phi = std::sin( phi );
					auto normal = dny::vector3<float>{
						std::cos( theta ) * sin_phi,
						std::cos( phi ),
						std::sin( theta ) * sin_phi
					};
					normal = dny::normalize( normal );
					const auto position = normal * radius;
					return make_vertex( position, normal, { u, v } );
				};

				const auto v00 = make_sphere_vertex( theta0, phi0, u0, v0 );
				const auto v10 = make_sphere_vertex( theta1, phi0, u1, v0 );
				const auto v01 = make_sphere_vertex( theta0, phi1, u0, v1 );
				const auto v11 = make_sphere_vertex( theta1, phi1, u1, v1 );

				vertices.push_back( v00 );
				vertices.push_back( v10 );
				vertices.push_back( v11 );

				vertices.push_back( v00 );
				vertices.push_back( v11 );
				vertices.push_back( v01 );
			}
		}

		return vertices;
	}

	inline auto generate_cylinder(
		float radius = 0.5f,
		float height = 1.0f,
		std::uint32_t slices = 32,
		std::uint32_t stacks = 1,
		bool include_caps = true ) -> std::vector<pnu_vertex>{
		slices = std::max( 3u, slices );
		stacks = std::max( 1u, stacks );

		std::vector<pnu_vertex> vertices;
		vertices.reserve( static_cast<std::size_t>( slices ) * static_cast<std::size_t>( stacks ) * 6ull + static_cast<std::size_t>( slices ) * 6ull );

		const auto half_height = height * 0.5f;

		for( std::uint32_t stack = 0; stack < stacks; ++stack ){
			const auto t0 = static_cast<float>( stack ) / static_cast<float>( stacks );
			const auto t1 = static_cast<float>( stack + 1u ) / static_cast<float>( stacks );
			const auto y0 = -half_height + height * t0;
			const auto y1 = -half_height + height * t1;

			for( std::uint32_t slice = 0; slice < slices; ++slice ){
				const auto u0 = static_cast<float>( slice ) / static_cast<float>( slices );
				const auto u1 = static_cast<float>( slice + 1u ) / static_cast<float>( slices );
				const auto theta0 = dny::PI2 * u0;
				const auto theta1 = dny::PI2 * u1;

				const auto n0 = dny::normalize( dny::vector3<float>{ std::cos( theta0 ), 0.0f, std::sin( theta0 ) } );
				const auto n1 = dny::normalize( dny::vector3<float>{ std::cos( theta1 ), 0.0f, std::sin( theta1 ) } );

				const auto p00 = dny::vector3<float>{ n0.x * radius, y0, n0.z * radius };
				const auto p10 = dny::vector3<float>{ n1.x * radius, y0, n1.z * radius };
				const auto p01 = dny::vector3<float>{ n0.x * radius, y1, n0.z * radius };
				const auto p11 = dny::vector3<float>{ n1.x * radius, y1, n1.z * radius };

				vertices.push_back( make_vertex( p00, n0, { u0, t0 } ) );
				vertices.push_back( make_vertex( p10, n1, { u1, t0 } ) );
				vertices.push_back( make_vertex( p11, n1, { u1, t1 } ) );

				vertices.push_back( make_vertex( p00, n0, { u0, t0 } ) );
				vertices.push_back( make_vertex( p11, n1, { u1, t1 } ) );
				vertices.push_back( make_vertex( p01, n0, { u0, t1 } ) );
			}
		}

		if( !include_caps ){
			return vertices;
		}

		auto emit_cap = [ & ]( float y, float normal_y ){
			const auto normal = dny::vector3<float>{ 0.0f, normal_y, 0.0f };
			const auto center = dny::vector3<float>{ 0.0f, y, 0.0f };

			for( std::uint32_t slice = 0; slice < slices; ++slice ){
				const auto u0 = static_cast<float>( slice ) / static_cast<float>( slices );
				const auto u1 = static_cast<float>( slice + 1u ) / static_cast<float>( slices );
				const auto theta0 = dny::PI2 * u0;
				const auto theta1 = dny::PI2 * u1;

				const auto edge0 = dny::vector3<float>{ std::cos( theta0 ) * radius, y, std::sin( theta0 ) * radius };
				const auto edge1 = dny::vector3<float>{ std::cos( theta1 ) * radius, y, std::sin( theta1 ) * radius };

				auto uv_from_pos = [ radius ]( dny::vector3<float> const& p ){
					return dny::vector2<float>{ 0.5f + ( p.x / ( radius * 2.0f ) ), 0.5f - ( p.z / ( radius * 2.0f ) ) };
				};

				const auto center_uv = dny::vector2<float>{ 0.5f, 0.5f };
				const auto uv0 = uv_from_pos( edge0 );
				const auto uv1 = uv_from_pos( edge1 );

				if( normal_y > 0.0f ){
					vertices.push_back( make_vertex( center, normal, center_uv ) );
					vertices.push_back( make_vertex( edge0, normal, uv0 ) );
					vertices.push_back( make_vertex( edge1, normal, uv1 ) );
				}
				else{
					vertices.push_back( make_vertex( center, normal, center_uv ) );
					vertices.push_back( make_vertex( edge1, normal, uv1 ) );
					vertices.push_back( make_vertex( edge0, normal, uv0 ) );
				}
			}
		};

		emit_cap( +half_height, +1.0f );
		emit_cap( -half_height, -1.0f );

		return vertices;
	}
}


namespace dny{
	template<>
	struct is_vertex<primitives::debug_vertex> : std::true_type{};
}
