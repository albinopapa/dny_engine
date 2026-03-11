#pragma once

#include "dny_type_traits.hpp"
#include "dny_vector2.hpp"

#include <format>

namespace dny{
	template<typename ElementT>
	struct vector3{
		static constexpr std::size_t num_elements = 3ull;
		using element_type = ElementT;

		constexpr vector3()noexcept = default;
		constexpr vector3( element_type x_, element_type y_, element_type z_ )
			:
			x( x_ ), y( y_ ), z( z_ ){}
		constexpr explicit vector3( vector2<element_type> const& v_, element_type z_ = 1.f )
			:
			x( v_.x ), y( v_.y ), z( z_ ){}
		operator vector2<element_type>()const{
			return vector2<ElementT>{ x, y };
		}

		constexpr vector3& operator+=( vector3 const& other )noexcept{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}
		constexpr vector3& operator-=( vector3 const& other )noexcept{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}
		constexpr vector3& operator*=( element_type other )noexcept{
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}
		constexpr vector3& operator/=( element_type other )noexcept{
			x /= other;
			y /= other;
			z /= other;
			return *this;
		}

		constexpr vector3 broadcast( std::size_t element_idx )const{
			if( element_idx > 3 )throw std::out_of_range( std::format( "elment_idx: {} > 3.", element_idx ) );
			
			switch( element_idx ){
				default: return vector3{ x, x, x };
				case 1:  return vector3{ y, y, y };
				case 2:  return vector3{ z, z, z };
			}
		}

		element_type x = {};
		element_type y = {};
		element_type z = {};
	};

	template<typename ElementT> struct is_math_vector<vector3<ElementT>> : std::true_type{};
}