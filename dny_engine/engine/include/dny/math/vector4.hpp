#pragma once

#include "dny_type_traits.hpp"
#include "dny_vector2.hpp"
#include "dny_vector3.hpp"

namespace dny{
	template<typename ElementT>
	struct vector4{
		static constexpr std::size_t num_elements = 4ull;
		using element_type = ElementT;

		constexpr vector4()noexcept = default;
		constexpr vector4( ElementT x_, ElementT y_, ElementT z_, ElementT w_ = 1.f )
			:
			x( x_ ), y( y_ ), z( z_ ), w( w_ ){}
		explicit constexpr vector4( vector3<ElementT> const& vec, ElementT w_ = 1.f )noexcept
			: x( vec.x ), y( vec.y ), z( vec.z ), w( w_ ){}

		constexpr operator vector2<ElementT>()const noexcept{
			return vector2<ElementT>{ x, y };
		}
		constexpr operator vector3<ElementT>()const noexcept{
			return vector3<ElementT>{ x, y, z };
		}

		constexpr vector4& operator+=( vector4 const& other )noexcept{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}
		constexpr vector4& operator-=( vector4 const& other )noexcept{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}
		constexpr vector4& operator*=( ElementT other )noexcept{
			x *= other;
			y *= other;
			z *= other;
			w *= other;
			return *this;
		}
		constexpr vector4& operator/=( ElementT other )noexcept{
			x /= other;
			y /= other;
			z /= other;
			w /= other;
			return *this;
		}

		ElementT x = {};
		ElementT y = {};
		ElementT z = {};
		ElementT w = {};
	};

	template<typename ElementT> struct is_math_vector<vector4<ElementT>> : std::true_type{};

}