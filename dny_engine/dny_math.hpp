#pragma once

#include "dny_concepts.hpp"
#include "dny_dims2.hpp"
#include "dny_math_constants.hpp"
#include "dny_vector2.hpp"
#include "dny_vector3.hpp"
#include "dny_vector4.hpp"
#include "dny_matrix_3x3.hpp"
#include "dny_matrix_4x4.hpp"
#include "dny_rectangle.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <limits>

namespace dny{
	constexpr auto operator+( dny::vector_type auto const& lhs, dny::vector_type auto const& rhs )noexcept{
		auto result = lhs;
		result += rhs;
		return result;
	}

	constexpr auto operator-( dny::vector_type auto const& lhs, dny::vector_type auto const& rhs )noexcept{
		auto result = lhs;
		result -= rhs;
		return result;
	}

	constexpr auto operator-( dny::vector_type auto const& lhs )noexcept{
		using type = std::decay_t<decltype( lhs )>;
		auto result = type{};
		result.x = -lhs.x;
		result.y = -lhs.y;
		if constexpr( type::num_elements == 3 ){
			result.z = -lhs.z;
		}
		if constexpr( type::num_elements == 4 ){
			result.w = -lhs.w;
		}

		return result;
	}

	constexpr auto operator*( dny::vector_type auto const& lhs, dny::vector_type auto const& rhs )noexcept{
		static_assert(
			std::is_same_v<decltype( lhs ), decltype( rhs )>,
			"Vector types must be same for Haddamard multiplication."
		);
		using type = std::decay_t<decltype( lhs )>;

		auto result = lhs;
		result.x = lhs.x * rhs.x;
		result.y = lhs.y * rhs.y;
		if constexpr( type::num_elements > 2 ){
			result.z = lhs.z * rhs.z;
		}
		if constexpr( type::num_elements > 3 ){
			result.w = lhs.w * rhs.w;
		}
		return result;
	}

	constexpr auto operator*( dny::vector_type auto const& lhs, auto const& rhs )noexcept{
		auto result = lhs;
		result *= rhs;
		return result;
	}

	constexpr auto operator*( auto const& lhs, dny::vector_type auto const& rhs )noexcept{
		auto result = rhs;
		result *= lhs;
		return result;
	}

	constexpr auto operator/( dny::vector_type auto const& lhs, auto const& rhs )noexcept{
		auto result = lhs;
		result /= rhs;
		return result;
	}

	template<typename ElementT>
	constexpr auto operator+( dny::Rect<ElementT> const& lhs, dny::vector2<ElementT> const& rhs )noexcept{
		auto result = lhs;
		result.translate( rhs );
		return result;
	}

	template<typename ElementT>
	constexpr auto operator+( 
		dny::matrix_4x4<ElementT> const& lhs, 
		dny::matrix_4x4<ElementT>  const& rhs )noexcept{
		auto result = dny::matrix_4x4<ElementT>{};
		for( int i = 0; i < 4; ++i ){
			result.m_rows[ i ] = lhs.m_rows[ i ] + rhs.m_rows[ i ];
		}

		return result;
	}

	template<typename ElementT>
	constexpr auto operator-( 
		dny::matrix_4x4<ElementT> const& lhs, 
		dny::matrix_4x4<ElementT> const& rhs )noexcept{
		auto result = dny::matrix_4x4<ElementT>{};
		for( int i = 0; i < 4; ++i ){
			result.m_rows[ i ] = lhs.m_rows[ i ] - rhs.m_rows[ i ];
		}

		return result;
	}

	template<typename ElementT>
	auto operator*( 
		dny::vector4<ElementT> const& lhs, 
		dny::matrix_4x4<ElementT> const& rhs )noexcept{
		const auto r0 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 0 ] ) );
		const auto r1 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 1 ] ) );
		const auto r2 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 2 ] ) );
		const auto r3 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 3 ] ) );

		const auto l = _mm_loadu_ps( reinterpret_cast< float const* >( &lhs ) );
		const auto x = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 0, 0, 0, 0 ) );
		const auto y = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 1, 1, 1, 1 ) );
		const auto z = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 2, 2, 2, 2 ) );
		const auto w = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 3, 3, 3, 3 ) );

		__m128 sum = _mm_setzero_ps();
		sum = _mm_fmadd_ps( x, r0, sum );
		sum = _mm_fmadd_ps( y, r1, sum );
		sum = _mm_fmadd_ps( z, r2, sum );
		sum = _mm_fmadd_ps( w, r3, sum );

		dny::vector4<ElementT> result = {};
		_mm_storeu_ps( reinterpret_cast< float* >( &result ), sum );

		return result;
	}

	template<typename ElementT>
	constexpr auto operator*( 
		dny::vector3<ElementT> const& lhs, 
		dny::matrix_3x3<ElementT> const& rhs )noexcept{
		return dny::vector3<ElementT>{
			lhs.x * rhs.m_rows[ 0 ] +
			lhs.y * rhs.m_rows[ 1 ] +
			lhs.z * rhs.m_rows[ 2 ]
		};
	}

	template<typename ElementT>
	auto operator*( 
		dny::matrix_4x4<ElementT> const& lhs, 
		dny::matrix_4x4<ElementT> const& rhs )noexcept{
		dny::matrix_4x4<ElementT> result;

		// If left handed
		const auto r0 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 0 ] ) );
		const auto r1 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 1 ] ) );
		const auto r2 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 2 ] ) );
		const auto r3 = _mm_loadu_ps( reinterpret_cast< float const* >( &rhs.m_rows[ 3 ] ) );

		auto mul = [ & ]( dny::vector4<ElementT> const& v ){
			const auto l = _mm_loadu_ps( reinterpret_cast< float const* >( &v ) );
			const auto x = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 0, 0, 0, 0 ) );
			const auto y = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 1, 1, 1, 1 ) );
			const auto z = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 2, 2, 2, 2 ) );
			const auto w = _mm_shuffle_ps( l, l, _MM_SHUFFLE( 3, 3, 3, 3 ) );
			auto sum = _mm_setzero_ps();
			sum = _mm_fmadd_ps( x, r0, sum );
			sum = _mm_fmadd_ps( y, r1, sum );
			sum = _mm_fmadd_ps( z, r2, sum );
			sum = _mm_fmadd_ps( w, r3, sum );

			dny::vector4<ElementT> result;
			_mm_storeu_ps( reinterpret_cast< float* >( &result), sum );
			return result;
		};

		result.m_rows[ 0 ] = mul( lhs.m_rows[ 0 ] );
		result.m_rows[ 1 ] = mul( lhs.m_rows[ 1 ] );
		result.m_rows[ 2 ] = mul( lhs.m_rows[ 2 ] );
		result.m_rows[ 3 ] = mul( lhs.m_rows[ 3 ] );

		return result;
	}

	template<typename ElementT>
	constexpr auto operator*( dny::matrix_4x4<ElementT> const& lhs, ElementT rhs )noexcept{
		return dny::matrix_4x4<ElementT>{
			lhs.m_rows[ 0 ] * rhs,
			lhs.m_rows[ 1 ] * rhs,
			lhs.m_rows[ 2 ] * rhs,
			lhs.m_rows[ 3 ] * rhs
		};
	}

	constexpr float to_radians( float degrees )noexcept{
		return ( dny::PI / 180.f ) * degrees;
	}

	constexpr auto dot( dny::vector_type auto const& lhs, dny::vector_type auto const& rhs )noexcept{
		using type = std::decay_t<decltype( lhs )>;
		static_assert(
			std::is_same_v<std::decay_t<decltype( lhs )>, std::decay_t<decltype( rhs )>>,
			"Vector types must match for dot product."
		);

		const auto mul_result = lhs * rhs;
		auto result = mul_result.x + mul_result.y;
		if constexpr( type::num_elements > 2 ){
			result += mul_result.z;
		}
		if constexpr( type::num_elements > 3 ){
			result += mul_result.w;
		}

		return result;
	}

	template<typename ElementT>
	constexpr auto cross( dny::vector2<ElementT> const& lhs, dny::vector2<ElementT> const& rhs )noexcept{
		return ( lhs.x * rhs.y ) - ( lhs.y * rhs.x );
	}

	template<typename ElementT>
	constexpr auto cross( dny::vector3<ElementT> const& lhs, dny::vector3<ElementT> const& rhs )noexcept{
		return( dny::vector3{ lhs.y, lhs.z, lhs.x } * dny::vector3{ rhs.z, rhs.x, rhs.y } )
			- ( dny::vector3{ lhs.z, lhs.x, lhs.y } * dny::vector3{ rhs.y, rhs.z, rhs.x } );
	}
	
	constexpr auto sq_length( dny::vector_type auto const& lhs )noexcept{
		return dot( lhs, lhs );
	}
	
	constexpr auto length( dny::vector_type auto const& lhs )noexcept{
		return std::sqrt( sq_length( lhs ) );
	}
	
	constexpr auto normalize( dny::vector_type auto const& lhs )noexcept{
		const auto len_sq = sq_length( lhs );
		if( len_sq == 0.f )return lhs;
		const auto inv_len = 1.f / length( lhs );
		return lhs * inv_len;
	}
	
	template<typename T>
	constexpr auto lerp( T const& a, T const& b, float t ){
		return a + ( b - a ) * t;
	}
	
	constexpr auto raise_to( float val, std::uint32_t shift )noexcept{
		float r = 1.f;
		for( auto i = 0u; i < shift; ++i )
			r *= val;
		return r;
	}
	
	template<typename ElementT>
	constexpr auto signed_area( 
		dny::vector2<ElementT> const& a, 
		dny::vector2<ElementT> const& b, 
		dny::vector2<ElementT> const& c )noexcept{
		const auto e1 = b - a;
		const auto e2 = c - a;
		return cross( e1, e2 );
	}
	
	template<typename ElementT>
	constexpr bool is_top_left( dny::vector2<ElementT> const& v0, dny::vector2<ElementT>  const& v1 ){
		const auto dist = v1 - v0;
		return ( dist.y < 0.f ) || ( dist.y == 0.f && dist.x > 0.f );
	};

	template<dny::handedness_t handed>
	static auto projection( float fovy, float aspect, float zn, float zf ) noexcept{
		static_assert( handed != dny::handedness_t::invalid, "Handedness not set." );

		const float height = 1.f / std::tan( fovy * 0.5f );
		constexpr auto sign = [](){
			if constexpr( handed == handedness_t::right ){
				return -1.f;
			}
			else{
				return 1.f;
			}
		}( );
		const auto f_range = [&](){
			if constexpr( handed == dny::handedness_t::right ){
				return zf / ( zn - zf );
			}
			else{
				return zf / ( zf - zn );
			}
		}( );

		auto m = dny::matrix_4x4<float>{};

		m.m_rows[ 0 ] = { height / aspect,    0.f,                      0.f,  0.f };
		m.m_rows[ 1 ] = {             0.f, height,                      0.f,  0.f };
		m.m_rows[ 2 ] = {             0.f,    0.f,                  f_range, sign };
		m.m_rows[ 3 ] = {             0.f,    0.f, ( -sign * f_range ) * zn,  0.f };

		return m;
	}

	template<typename ElementT, handedness_t handed = handedness_t::left>
	auto look_to(
		const dny::vector3<ElementT>& eye,
		const dny::vector3<ElementT>& direction,
		const dny::vector3<ElementT>& up ){
		const auto R2 = normalize( handed == dny::handedness_t::right ? -direction : direction );
		const auto R0 = normalize( cross( up, R2 ) );
		const auto R1 = cross( R2, R0 );

		const auto neg_eye = -eye;
		return dny::matrix_4x4<ElementT>{
			dny::vector4<ElementT>{ R0, 0.f },
			dny::vector4<ElementT>{ R1, 0.f },
			dny::vector4<ElementT>{ R2, 0.f },
			dny::vector4<ElementT>{ dot( R0, neg_eye ), dot( R1, neg_eye ), dot( R2, neg_eye ), 1.f }
		};
	}

	template<typename ElementT, dny::handedness_t handed>
	auto look_at( const dny::vector3<ElementT>& eye, const dny::vector3<ElementT>& target, const vector3<ElementT>& up ) noexcept{
		static_assert( handed != dny::handedness_t::invalid, "Handedness not set." );
		if constexpr( handed == dny::handedness_t::right ){
			return look_to<ElementT, dny::handedness_t::left>( eye, eye - target, up );		// Forward (camera -Z)
		}
		else{
			return look_to<ElementT, dny::handedness_t::left>( eye, target - eye, up );		// Forward (camera +Z)
		}
	}

}
