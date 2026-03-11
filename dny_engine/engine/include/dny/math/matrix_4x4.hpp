#pragma once

#include "math_constants.hpp"
#include "matrix_3x3.hpp"
#include "vector4.hpp"

#include <array>
#include <cmath>
#include <mdspan>

namespace dny{
	template<typename ElementT>
	class matrix_4x4{
	public:
		using element_type = ElementT;
		static constexpr std::size_t num_rows = 4;
		static constexpr std::size_t num_columns = 4;

		constexpr matrix_4x4()noexcept = default;
		constexpr matrix_4x4(
			vector4<ElementT> const& r1,
			vector4<ElementT> const& r2,
			vector4<ElementT> const& r3,
			vector4<ElementT> const& r4 )noexcept
			:
			m_rows{ r1, r2, r3, r4 }{}
		constexpr matrix_4x4( matrix_3x3<ElementT> const& other )noexcept{
			m_rows[ 0 ] = vector4<ElementT>( other.m_rows[ 0 ], {} );
			m_rows[ 1 ] = vector4<ElementT>( other.m_rows[ 1 ], {} );
			m_rows[ 2 ] = vector4<ElementT>( other.m_rows[ 2 ], {} );
			m_rows[ 3 ] = vector4<ElementT>( {}, {}, {}, static_cast< ElementT >( 1 ) );
		}

		constexpr operator matrix_3x3<ElementT>()const noexcept{
			return matrix_3x3{
				vector3{ m_rows[ 0 ].x, m_rows[ 0 ].y, m_rows[ 0 ].z },
				vector3{ m_rows[ 1 ].x, m_rows[ 1 ].y, m_rows[ 1 ].z },
				vector3{ m_rows[ 2 ].x, m_rows[ 2 ].y, m_rows[ 2 ].z }
			};
		}
		static constexpr matrix_4x4 identity(){
			static constexpr auto one = static_cast< ElementT >( 1 );
			return{
				vector4<ElementT>{ one, {}, {}, {} },
				vector4<ElementT>{ {}, one, {}, {} },
				vector4<ElementT>{ {}, {}, one, {} },
				vector4<ElementT>{ {}, {}, {}, one }
			};
		}
		static constexpr auto translation( vector3<ElementT> const& v )noexcept{
			auto result = matrix_4x4::identity();
			result.m_rows[ 3 ] = vector4<ElementT>{ v };
			return result;
		}
		static constexpr auto scaling( vector3<ElementT> const& v )noexcept{
			auto result = matrix_4x4::identity();
			result.m_rows[ 0 ].x = v.x;
			result.m_rows[ 1 ].y = v.y;
			result.m_rows[ 2 ].z = v.z;
			return result;
		}
		static auto rotation_x( float angle )noexcept{
			auto result = matrix_4x4{};
			const auto c = std::cos( angle );
			const auto s = std::sin( angle );
			static constexpr auto one = static_cast< ElementT >( 1 );
			result.m_rows[ 0 ] = vector4<ElementT>{ one, {}, {}, {} };
			result.m_rows[ 1 ] = vector4<ElementT>{ {}, c, -s, {} };
			result.m_rows[ 2 ] = vector4<ElementT>{ {}, s, c, {} };
			result.m_rows[ 3 ] = vector4<ElementT>{ {}, {}, {}, one };

			return result;
		}
		static auto rotation_y( ElementT angle )noexcept{
			auto result = matrix_4x4::identity();
			const auto c = std::cos( angle );
			const auto s = std::sin( angle );
			static constexpr auto one = static_cast< ElementT >( 1 );
			result.m_rows[ 0 ] = vector4{ c, {}, s, {} };
			result.m_rows[ 1 ] = vector4{ {}, one, {}, {} };
			result.m_rows[ 2 ] = vector4{ -s, {}, c, {} };
			result.m_rows[ 3 ] = vector4{ {}, {}, {}, one };

			return result;
		}
		static auto rotation_z( float angle )noexcept{
			auto result = matrix_4x4::identity();
			const auto c = std::cos( angle );
			const auto s = std::sin( angle );
			static constexpr auto one = static_cast< ElementT >( 1 );

			result.m_rows[ 0 ] = vector4<ElementT>{ c, -s, {}, {} };
			result.m_rows[ 1 ] = vector4<ElementT>{ s, c, {}, {} };
			result.m_rows[ 2 ] = vector4<ElementT>{ {}, {}, one, {} };
			result.m_rows[ 3 ] = vector4<ElementT>{ {}, {}, {}, one };

			return result;
		}
		static auto rotation( float yaw, float pitch, float roll ) noexcept{
			return rotation_y( yaw ) * rotation_x( pitch ) * rotation_z( roll );
		}
	public:
		std::array<vector4<ElementT>, 4> m_rows = {};
	};

}