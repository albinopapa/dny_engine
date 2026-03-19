#pragma once

#include "dims2.hpp"
#include "math/vector2.hpp"

#include <algorithm>

namespace dny{
	template<typename ElementT>
	struct Rect{
		using element_type = ElementT;

		constexpr Rect() noexcept = default;

		constexpr Rect( ElementT left_, ElementT top_, ElementT right_, ElementT bottom_ ) noexcept
			: 
			left( left_ )
			, top( top_ )
			, right( right_ )
			, bottom( bottom_ ){}
			/*left( std::min( left_, right_ ) )
			, top( std::min( top_, bottom_ ) )
			, right( std::max( left_, right_ ) )
			, bottom( std::max( top_, bottom_ ) ){}*/

		constexpr Rect( vector2<ElementT> const& p0_, vector2<ElementT> const& p1_ ) noexcept
			: Rect( p0_.x, p0_.y, p1_.x, p1_.y ){}

		constexpr Rect( vector2<ElementT> const& position_, dims2<ElementT> const& size_ ) noexcept
			: Rect(
				position_.x,
				position_.y,
				position_.x + size_.width,
				position_.y + size_.height ){}

		constexpr vector2<ElementT> top_left()const noexcept{
			return { left, top };
		}

		constexpr vector2<ElementT> top_right()const noexcept{
			return { right, top };
		}

		constexpr vector2<ElementT> bottom_left()const noexcept{
			return { left, bottom };
		}

		constexpr vector2<ElementT> bottom_right()const noexcept{
			return { right, bottom };
		}

		constexpr ElementT width()const noexcept{
			return right - left;
		}

		constexpr ElementT height()const noexcept{
			return bottom - top;
		}

		constexpr dims2<ElementT> size() const noexcept{ return { width(), height() }; }

		constexpr vector2<ElementT> center()const noexcept{
			return vector2<ElementT>{
				left + (  width() / static_cast< ElementT >( 2 ) ),
				top + ( height() / static_cast< ElementT >( 2 ) ) 
			};
		}

		constexpr Rect& translate( vector2<ElementT> const& offset )noexcept{
			left += offset.x;
			top += offset.y;
			right += offset.x;
			bottom += offset.y;

			return *this;
		}

		// Expansion (useful for broadphase)
		constexpr Rect& expand( ElementT amount )noexcept{
			*this = Rect{
				left - amount,
				top - amount,
				right + amount,
				bottom + amount
			};

			return *this;
		}

		ElementT left   = {};
		ElementT top    = {};
		ElementT right  = {};
		ElementT bottom = {};
	};
}
