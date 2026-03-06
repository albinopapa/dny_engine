#pragma once

#include "dny_aabb.hpp"
#include "dny_concepts.hpp"
#include "dny_rectangle.hpp"
#include "dny_vector2.hpp"

namespace dny{

	// ---------------------------------------------
	// Rect<T> queries
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
			a.right  > b.left  &&
			a.left   < b.right &&
			a.bottom > b.top   &&
			a.top    < b.bottom;
	}

	template<math_scalar T>
	constexpr vector2<T> penetration_vector( const Rect<T>& a, const Rect<T>& b ) noexcept{
		T left   = b.right  - a.left;
		T right  = a.right  - b.left;
		T bottom = b.bottom - a.top;
		T top    = a.bottom - b.top;

		T pen_x  = ( left < right ) ? left : -right;
		T pen_y  = ( bottom < top ) ? bottom : -top;

		if( std::abs( pen_x ) < std::abs( pen_y ) )
			return { pen_x, T{} };
		else
			return { T{}, pen_y };
	}


	template<math_scalar T>
	constexpr bool intersects( const aabb<T>& a, const aabb<T>& b ) noexcept{
		return
			a.max.x > b.min.x && a.min.x < b.max.x &&
			a.max.y > b.min.y && a.min.y < b.max.y &&
			a.max.z > b.min.z && a.min.z < b.max.z;
	}

}