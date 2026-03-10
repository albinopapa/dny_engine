#pragma once

#include "dny_aabb.hpp"
#include "dny_concepts.hpp"
#include "dny_rectangle.hpp"
#include "dny_vector2.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace dny{
	template<math_scalar T>
	struct polyline_collider{
		std::vector<vector2<T>> points;

		[[nodiscard]] constexpr bool valid() const noexcept{
			return points.size() >= 2;
		}
	};

	template<math_scalar T>
	inline polyline_collider<T> generate_polyline_collider( std::vector<vector2<T>> points_ ){
		points_.erase(
			std::unique( points_.begin(), points_.end(), []( vector2<T> const& lhs_, vector2<T> const& rhs_ ){
				return lhs_.x == rhs_.x && lhs_.y == rhs_.y;
			} ),
			points_.end()
		);

		return { std::move( points_ ) };
	}

	template<math_scalar T>
	inline std::optional<T> sample_polyline_height( polyline_collider<T> const& polyline_, T x_ ){
		if( !polyline_.valid() ){
			return std::nullopt;
		}

		auto sampled_height = std::optional<T>{};
		for( std::size_t i = 0; i + 1 < polyline_.points.size(); ++i ){
			const auto p0 = polyline_.points[ i ];
			const auto p1 = polyline_.points[ i + 1 ];

			const auto min_x = std::min( p0.x, p1.x );
			const auto max_x = std::max( p0.x, p1.x );
			if( x_ < min_x || x_ > max_x ){
				continue;
			}
			constexpr T epsilon = static_cast< T >( 1e-4 );

			if( std::abs( p0.x - p1.x ) < epsilon ){
				const auto y = std::max( p0.y, p1.y );
				sampled_height = sampled_height ? std::max( *sampled_height, y ) : y;
				continue;
			}

			const auto t = ( x_ - p0.x ) / ( p1.x - p0.x );
			const auto y = p0.y + ( p1.y - p0.y ) * t;
			sampled_height = sampled_height ? std::max( *sampled_height, y ) : y;
		}

		return sampled_height;
	}

	template<math_scalar T>
	inline bool intersects( aabb<T> const& box_, polyline_collider<T> const& polyline_ ){
		if( !polyline_.valid() ){
			return false;
		}

		const auto center_x = ( box_.min_pt.x + box_.max_pt.x ) * static_cast<T>( 0.5 );
		const auto surface = sample_polyline_height( polyline_, center_x );
		if( !surface ){
			return false;
		}

		return box_.min_pt.y < *surface && box_.max_pt.y > *surface;
	}

	template<math_scalar T>
	inline vector2<T> resolve_aabb_vs_polyline( aabb<T>& box_, polyline_collider<T> const& polyline_, T snap_distance_ = static_cast<T>( 1.0 ) ){
		if( !polyline_.valid() ){
			return {};
		}

		const auto center_x = ( box_.min_pt.x + box_.max_pt.x ) * static_cast<T>( 0.5 );
		const auto surface = sample_polyline_height( polyline_, center_x );
		if( !surface ){
			return {};
		}

		const auto penetration = *surface - box_.min_pt.y;
		if( penetration < -snap_distance_ ){
			return {};
		}
		constexpr T epsilon = static_cast< T >( 0.0001 );
		if( std::abs( penetration ) < epsilon )
			return {};

		box_ = box_.translated( { T{}, penetration, T{} } );
		return { T{}, penetration };
	}


	// ---------------------------------------------
	// Rect<T> queries (screen-space: top < bottom)
	// ---------------------------------------------
	template<math_scalar T>
	constexpr bool contains( const Rect<T>& r, const vector2<T>& p ) noexcept{
		return
			p.x >= r.left && p.x <= r.right &&
			p.y >= r.top && p.y <= r.bottom;
	}

	template<math_scalar T>
	constexpr bool contains( Rect<T> const& lhs_, Rect<T> const& rhs_ ) noexcept{
		return
			( lhs_.left < rhs_.left && lhs_.right > rhs_.right ) &&
			( lhs_.top < rhs_.top && lhs_.bottom > rhs_.bottom );
	}

	template<math_scalar T>
	constexpr bool intersects( const Rect<T>& a, const Rect<T>& b ) noexcept{
		return
			a.right > b.left   &&
			a.left < b.right   &&
			a.bottom > b.top   &&
			a.top < b.bottom;
	}

	template<math_scalar T>
	constexpr vector2<T> penetration_vector( const Rect<T>& a, const Rect<T>& b ) noexcept{
		const auto overlap_x = std::min( a.right, b.right ) - std::max( a.left, b.left );
		const auto overlap_y = std::min( a.bottom, b.bottom ) - std::max( a.top, b.top );

		if( overlap_x <= T{} || overlap_y <= T{} ){
			return {};
		}

		if( overlap_x < overlap_y ){
			const auto a_center_x = ( a.left + a.right ) * static_cast<T>( 0.5 );
			const auto b_center_x = ( b.left + b.right ) * static_cast<T>( 0.5 );
			return { a_center_x < b_center_x ? -overlap_x : overlap_x, T{} };
		}

		const auto a_center_y = ( a.top + a.bottom ) * static_cast<T>( 0.5 );
		const auto b_center_y = ( b.top + b.bottom ) * static_cast<T>( 0.5 );
		return { T{}, a_center_y < b_center_y ? -overlap_y : overlap_y };
	}

	template<math_scalar T>
	constexpr bool intersects( const aabb<T>& a, const aabb<T>& b ) noexcept{
		return
			a.max_pt.x > b.min_pt.x && a.min_pt.x < b.max_pt.x &&
			a.max_pt.y > b.min_pt.y && a.min_pt.y < b.max_pt.y &&
			a.max_pt.z > b.min_pt.z && a.min_pt.z < b.max_pt.z;
	}

}
