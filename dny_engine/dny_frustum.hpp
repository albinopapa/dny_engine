#pragma once

#include "dny_simd.hpp"
#include "dny_matrix_4x4.hpp"
#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif // near


namespace dny{
	class frustum{
	public:
		frustum( dny::matrix_4x4<float> const& view_proj )noexcept{
			// Load rows as SIMD
			auto r0 = simd::load( view_proj.m_rows[ 0 ] );
			auto r1 = simd::load( view_proj.m_rows[ 1 ] );
			auto r2 = simd::load( view_proj.m_rows[ 2 ] );
			auto r3 = simd::load( view_proj.m_rows[ 3 ] );
			simd::transpose( r0, r1, r2, r3 );

			// Left   : r3 + r0
			// Right  : r3 - r0
			// Bottom : r3 + r1
			// Top    : r3 - r1
			// Near   : r3 + r2
			// Far    : r3 - r2
			left = normalize_plane( r3 + r0 );
			right = normalize_plane( r3 - r0 );
			bottom = normalize_plane( r3 + r1 );
			top = normalize_plane( r3 - r1 );
			near = normalize_plane( r3 + r2 );
			far = normalize_plane( r3 - r2 );
		}
		bool contains( dny::vector3 const& center, float radius )const noexcept{
			// Load center as (x,y,z,0)
			const auto c = simd::load( center );

			// Broadcast radius
			const auto r = simd::load( radius );

			auto test_plane = [ & ]( simd::float4 plane ){
				// plane = (nx, ny, nz, d)

				// dot(n, c)
				const auto prod = c * plane ;

				// sum xyz lanes
				// (nx*x + ny*y + nz*z + d)
				const auto sum = prod
					+ simd::shuffle<1, 1, 1, 1>( prod )
					+ simd::shuffle<2, 2, 2, 2>( prod )
					+ simd::shuffle<3, 3, 3, 3>( plane );

				// outside test: distance < -radius
				const auto outside = sum < ( -r );

				return simd::all( outside ); // culled
			};

			if( test_plane( left ) ) return false; // culled
			if( test_plane( right ) ) return false; // culled
			if( test_plane( bottom ) ) return false; // culled
			if( test_plane( top ) ) return false; // culled
			if( test_plane( near ) ) return false; // culled
			if( test_plane( far ) ) return false; // culled

			return true; // visible
		}

	private:
		simd::float4 normalize_plane( simd::float4 p ) noexcept{
			using namespace simd;

			// length = sqrt(nx*nx + ny*ny + nz*nz)
			const auto n2 = p * p;

			auto len =
				simd::shuffle<0, 0, 0, 0>( n2 ) +
				simd::shuffle<1, 1, 1, 1>( n2 ) +
				simd::shuffle<2, 2, 2, 2>( n2 );

			// reciprocal sqrt
			//const auto temp1 = _mm_rsqrt_ps( len );
			const auto temp2 = _mm_set1_ps( 1.f ) / _mm_sqrt_ps( len );

			return p * temp2;
		}

		simd::float4 left;
		simd::float4 right;
		simd::float4 bottom;
		simd::float4 top;
		simd::float4 near;
		simd::float4 far;
	};

}
#define far
#define near
