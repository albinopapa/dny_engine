#pragma once

#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include "math_constants.hpp"

#include "core/colors.hpp"

#if defined( _MSC_VER )
#include <intrin.h>
#elif defined( __SSE__ ) || defined( __x86_64__ ) || defined( __i386__ )
#include <xmmintrin.h>
#else
#error "dny::simd headers require SSE-capable x86 architecture"
#endif

#include <span>

namespace dny::simd{
	using float4 = __m128;

	inline float4 _vectorcall load( float const* src )noexcept{
		return _mm_loadu_ps( src );
	}
	inline float4 _vectorcall load( float src )noexcept{
		return _mm_set_ps1( src );
	}
	inline float4 _vectorcall load( float f1, float f2, float f3, float f4 )noexcept{
		return _mm_set_ps( f4, f3, f2, f1 );
	}
	inline float4 _vectorcall load( dny::vector2<float> const& src ) noexcept{
		return _mm_set_ps( 0.f, 0.f, src.y, src.x );
	}
	inline float4 _vectorcall load( dny::vector3<float> const& src ) noexcept{
		return _mm_set_ps( 0.f, src.z, src.y, src.x );
	}
	inline float4 _vectorcall load( dny::vector4<float> const& src ) noexcept{
		return _mm_set_ps( src.w, src.z, src.y, src.x );
	}
	inline float4 _vectorcall load( ColorF const& src ) noexcept{
		return _mm_loadu_ps(
			reinterpret_cast< float const* >( &src )
		);
	}

	template<std::size_t A, std::size_t B, std::size_t C, std::size_t D>
	inline float4 _vectorcall make_control_mask(){
		static_assert( ( A | B | C | D ) <= 1, "mask bits must be 0 or 1" );

		constexpr auto vD = D == 0 ? 0.f : qnan;
		constexpr auto vC = C == 0 ? 0.f : qnan;
		constexpr auto vB = B == 0 ? 0.f : qnan;
		constexpr auto vA = A == 0 ? 0.f : qnan;

		return _mm_set_ps( vD, vC, vB, vA );
	}
	inline float4 _vectorcall mask_merge( float4 const& lhs, float4 const& rhs, float4 const& mask )noexcept{
		//_mm_permute_ps( lhs, 0 );
		const auto t_mask = _mm_and_ps( mask, lhs );
		const auto f_mask = _mm_andnot_ps( mask, rhs );
		return _mm_or_ps( t_mask, f_mask );
	}
	template<std::size_t A, std::size_t B, std::size_t C, std::size_t D>
	inline float4 _vectorcall mask_merge( float4 const& lhs, float4 const& rhs )noexcept{
		return mask_merge( lhs, rhs, make_control_mask<A, B, C, D>() );
	}

	inline bool _vectorcall all( float4 const& value )noexcept{
		return _mm_movemask_ps( value ) == 0b1111;
	}
	inline bool _vectorcall any( float4 const& value )noexcept{
		return _mm_movemask_ps( value ) != 0b0000;
	}
	inline bool _vectorcall none( float4 const& value )noexcept{
		return _mm_movemask_ps( value ) == 0b0000;
	}

	template<std::size_t A, std::size_t B, std::size_t C, std::size_t D>
	float4 _vectorcall shuffle( float4 const& value )noexcept{
		return _mm_shuffle_ps( value, value, _MM_SHUFFLE( D, C, B, A ) );
	}
	template<std::size_t A, std::size_t B, std::size_t C, std::size_t D>
	float4 _vectorcall shuffle( float4 const& lhs, float4 const& rhs )noexcept{
		return _mm_shuffle_ps( lhs, rhs, _MM_SHUFFLE( D, C, B, A ) );
	}
	template<std::size_t I>
	float4 _vectorcall broadcast( float4 const& value )noexcept{
		return _mm_shuffle_ps( value, value, _MM_SHUFFLE( I, I, I, I ) );
	}
	template<std::size_t I>
	float _vectorcall extract( float4 const& value )noexcept{
		if constexpr( I > 3 ){
			static_assert( I >= 0 && I <= 3, "Index out of bounds in simd::extract." );
		}
		return _mm_cvtss_f32( shuffle<I, I, I, I>( value ) );
	}

	inline float4 _vectorcall min( float4 const& lhs, float4 const& rhs )noexcept{
		return _mm_min_ps( lhs, rhs );
	}
	inline float4 _vectorcall max( float4 const& lhs, float4 const& rhs )noexcept{
		return _mm_max_ps( lhs, rhs );
	}
	inline float4 _vectorcall clamp( float4 const& value, float4 const& lo, float4 const& hi )noexcept{
		return min( hi, max( value, lo ) );
	}
	inline float4 _vectorcall abs( float4 const& value )noexcept{
		const auto mask = _mm_set1_ps( -0.f );
		return _mm_andnot_ps( mask, value );
	}
	inline float4 _vectorcall floor( float4 const& value )noexcept{
		return _mm_floor_ps( value );
	}
	inline float4 _vectorcall ceil( float4 const& value )noexcept{
		return _mm_ceil_ps( value );
	}
	inline float4 _vectorcall truncate( float4 const& value )noexcept{
		return _mm_trunc_ps( value );
	}
	inline void _vectorcall transpose( float4& r0, float4& r1, float4& r2, float4& r3 )noexcept{
		auto temp0 = shuffle<0, 1, 0, 1>( r0, r1 );
		auto temp1 = shuffle<2, 3, 2, 3>( r0, r1 );
		auto temp2 = shuffle<0, 1, 0, 1>( r2, r3 );
		auto temp3 = shuffle<2, 3, 2, 3>( r2, r3 );
		//_MM_TRANSPOSE4_PS( r0, r1, r2, r3 );
		r0 = shuffle<0, 2, 0, 2>( temp0, temp2 );
		r1 = shuffle<1, 3, 1, 3>( temp0, temp2 );
		r2 = shuffle<0, 2, 0, 2>( temp1, temp3 );
		r3 = shuffle<1, 3, 1, 3>( temp1, temp3 );
	}

	inline void _vectorcall store( float4 src, std::span<float, 4> dst ){
		_mm_storeu_ps( dst.data(), src );
	}
	inline void _vectorcall store( float4 src, float& dst )noexcept{
		_mm_store_ss( &dst, src );
	}
	inline void _vectorcall store( float4 src, dny::vector2<float>& dst ) noexcept{
		dst = dny::vector2{
			extract<0>( src ),
			extract<1>( src )
		};
	}
	inline void _vectorcall store( float4 src, dny::vector3<float>& dst ) noexcept{
		dst = dny::vector3{
			extract<0>( src ),
			extract<1>( src ),
			extract<2>( src )
		};
	}
	inline void _vectorcall store( float4 src, dny::vector4<float>& dst ) noexcept{
		_mm_storeu_ps( reinterpret_cast< float* >( &dst ), src );
	}
	inline void _vectorcall store( float4 src, ColorF& dst ) noexcept{
		_mm_storeu_ps( reinterpret_cast< float* >( &dst ), src );
	}
}

namespace dny{
	inline simd::float4 _vectorcall operator+( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_add_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator-( simd::float4 const& lhs )noexcept{
		return _mm_sub_ps( _mm_setzero_ps(), lhs );
	}
	inline simd::float4 _vectorcall operator-( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_sub_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator*( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_mul_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator/( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_div_ps( lhs, rhs );
	}

	inline simd::float4 _vectorcall operator<( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_cmplt_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator>( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_cmpgt_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator<=( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_cmple_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator>=( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_cmpge_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator==( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_cmpeq_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator!=( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_cmpneq_ps( lhs, rhs );
	}

	inline simd::float4 _vectorcall operator||( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_or_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator&&( simd::float4 const& lhs, simd::float4 const& rhs )noexcept{
		return _mm_and_ps( lhs, rhs );
	}
	inline simd::float4 _vectorcall operator!( simd::float4 const& mask )noexcept{
		const auto true_mask = _mm_setzero_ps() == _mm_setzero_ps();
		return _mm_andnot_ps( mask, true_mask );
	}

	inline simd::float4& _vectorcall operator+=( simd::float4& lhs, simd::float4 const& rhs )noexcept{
		lhs = lhs + rhs;
		return lhs;
	}
	inline simd::float4& _vectorcall operator-=( simd::float4& lhs, simd::float4 const& rhs )noexcept{
		lhs = lhs - rhs;
		return lhs;
	}
	inline simd::float4& _vectorcall operator*=( simd::float4& lhs, simd::float4 const& rhs )noexcept{
		lhs = lhs * rhs;
		return lhs;
	}
	inline simd::float4& _vectorcall operator/=( simd::float4& lhs, simd::float4 const& rhs )noexcept{
		lhs = lhs / rhs;
		return lhs;
	}
}
