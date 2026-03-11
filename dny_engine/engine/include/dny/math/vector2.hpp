#pragma once

#include "core/type_traits.hpp"

#include <cstddef>

namespace dny{
	template<typename ElementT>
	struct vector2{
		using element_type = ElementT;
		static constexpr std::size_t num_elements = 2ull;
		constexpr vector2& operator+=( vector2 const& other )noexcept{
			x += other.x;
			y += other.y;
			return *this;
		}
		constexpr vector2& operator-=( vector2 const& other )noexcept{
			x -= other.x;
			y -= other.y;
			return *this;
		}
		constexpr vector2& operator*=( ElementT other )noexcept{
			x *= other;
			y *= other;
			return *this;
		}
		constexpr vector2& operator/=( ElementT other )noexcept{
			x /= other;
			y /= other;
			return *this;
		}

		ElementT x = {};
		ElementT y = {};
	};

	template<typename ElementT > struct is_math_vector<vector2<ElementT>> : std::true_type{};
}