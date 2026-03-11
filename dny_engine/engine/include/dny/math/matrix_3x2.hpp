#pragma once

#include "dny_vector2.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

namespace dny{
static constexpr float m_gravity = 9.81f * 100.f;
static constexpr float m_slop = 0.001f;

template<typename T>
struct matrix_3x2{
	using scalar_type = T;

	T m00{}, m01{};
	T m10{}, m11{};
	T m20{}, m21{};

	constexpr matrix_3x2() noexcept = default;

	constexpr matrix_3x2(
		T _m00, T _m01,
		T _m10, T _m11,
		T _m20, T _m21
	) noexcept
		:
		m00( _m00 ), m01( _m01 ),
		m10( _m10 ), m11( _m11 ),
		m20( _m20 ), m21( _m21 ){}

	constexpr vector2<T> transform_point( vector2<T> const& v ) const noexcept{
		return {
			v.x * m00 + v.y * m10 + m20,
			v.x * m01 + v.y * m11 + m21
		};
	}

	constexpr vector2<T> transform_vector( vector2<T> const& v ) const noexcept{
		return {
			v.x * m00 + v.y * m10,
			v.x * m01 + v.y * m11
		};
	}
};
}
