#pragma once

#include "dny_vector2.hpp"

namespace dny{
	template<typename ElementT>
	struct Rect{
		using element_type = ElementT;

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

		constexpr vector2<ElementT> size() const noexcept{ return { width(), height() }; }

		constexpr vector2<ElementT> center()const noexcept{
			return vector2{ left, top } + ( vector2{ width(), height() } *= 0.5f );
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
			*this= Rect{
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
