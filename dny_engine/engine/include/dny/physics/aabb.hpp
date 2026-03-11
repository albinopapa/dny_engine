#pragma once

#include "core/concepts.hpp"
#include "math/vector3.hpp"

namespace dny{
	template<math_scalar T>
	struct aabb{
		constexpr aabb() = default;
		constexpr aabb( const vector3<T>& min_, const vector3<T>& max_ )
			: min_pt( min_ ), max_pt( max_ ){}

		constexpr vector3<T> center() const noexcept{
			static constexpr auto half = static_cast< T >( 0.5 );
			return vector3<T>{
				( max_pt.x - min_pt.x ) * half,
				( max_pt.y - min_pt.y ) * half,
				( max_pt.z - min_pt.z ) * half
			};
		}

		constexpr vector3<T> extents() const noexcept{
			return vector3<T>{
				( max_pt.x - min_pt.x ),
				( max_pt.y - min_pt.y ),
				( max_pt.z - min_pt.z )
			};
		}

		constexpr aabb translated( const vector3<T>& delta ) const noexcept{
			const auto new_min = vector3<T>{
				min_pt.x + delta.x,
				min_pt.y + delta.y,
				min_pt.z + delta.z
			};
			const auto new_max = vector3<T>{
				max_pt.x + delta.x,
				max_pt.y + delta.y,
				max_pt.z + delta.z
			};
			return { new_min, new_max };
		}

		vector3<T> min_pt{};
		vector3<T> max_pt{};
	};
}