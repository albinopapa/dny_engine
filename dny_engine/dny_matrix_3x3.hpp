#pragma once

#include "dny_vector3.hpp"

#include <array>
#include <cmath>

namespace dny{
	template<typename ElementT>
	class matrix_3x3{
	public:
		using element_type = ElementT;
		static constexpr std::size_t num_rows    = 3;
		static constexpr std::size_t num_columns = 3;

		constexpr matrix_3x3()noexcept = default;
		constexpr matrix_3x3( 
			vector3<ElementT> const& r1, 
			vector3<ElementT> const& r2, 
			vector3<ElementT> const& r3 )noexcept
			:
			m_rows{ r1, r2, r3 }{}

		//static constexpr matrix_3x3 identity(){
		//	return{
		//		vector3{ 1.f, 0.f, 0.f },
		//		vector3{ 0.f, 1.f, 0.f },
		//		vector3{ 0.f, 0.f, 1.f }
		//	};
		//}
		//static constexpr auto translation( vector2<ElementT> const& v )noexcept{
		//	auto result = matrix_3x3::identity();
		//	result.m_rows[ 3 ] = vector3<ElementT>{ v, 1.f };
		//	return result;
		//}
		//static constexpr auto scaling( vector2<ElementT> const& v )noexcept{
		//	auto result = matrix_3x3::identity();
		//	result.m_rows[ 0 ].x = v.x;
		//	result.m_rows[ 1 ].y = v.y;
		//	return result;
		//}

		//static auto rotation_z( float angle )noexcept{
		//	auto result = matrix_3x3::identity();
		//	const auto c = std::cos( angle );
		//	const auto s = std::sin( angle );

		//	result.m_rows[ 0 ].x = c; result.m_rows[ 0 ].y = -s;
		//	result.m_rows[ 1 ].x = s; result.m_rows[ 1 ].y = c;

		//	return result;
		//}

	public:
		std::array<vector3<ElementT>, 3> m_rows = {};
	};
}