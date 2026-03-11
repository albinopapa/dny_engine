#pragma once

#include "math/math.hpp"
#include "core/surface.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

namespace dny{
	inline Rect<std::int32_t> clip_rect( Rect<std::int32_t>const& src_, Rect<std::int32_t> const& bounds_ ){
		const auto left = std::max( src_.left, bounds_.left );
		const auto right = std::min( src_.right, bounds_.right );
		const auto top = std::max( src_.top, bounds_.top );
		const auto bottom = std::min( src_.bottom, bounds_.bottom );

		if( left >= right || top >= bottom ){
			return Rect<std::int32_t>{};
		}

		return Rect<std::int32_t>{ left, top, right, bottom };
	}

	template<typename ColorT>
	void draw_line(
		vector2<std::int32_t> p1_,
		vector2<std::int32_t> p2_,
		ColorT color_,
		surface<ColorT>& canvas_ ){
		auto delta = p2_ - p1_;
		const auto m = p2_.x != p1_.x ? delta.y / delta.x : 0.0f;

		if( p2_.x != p1_.x && m >= -1.f && m <= 1.f ){
			if( p1_.x > p2_.x ){
				std::swap( p1_.x, p2_.x );
				std::swap( p1_.y, p2_.y );
			}
			delta.x = p2_.x - p1_.x;
			delta.y = p2_.y - p1_.y;
			const auto b = p1_.y - m * p1_.x;
			for( auto x = p1_.x; x < p2_.x; x++ ){
				const auto y = m * x + b;
				canvas_.pixel( x, y ) = color_;
			}
		}
		else{
			if( p1_.y > p2_.y ){
				std::swap( p1_.y, p2_.y );
				std::swap( p1_.x, p2_.x );
			}
			delta.x = p2_.x - p1_.x;
			delta.y = p2_.y - p1_.y;
			const auto w = delta.x / delta.y;
			const auto p = p1_.x - w * p1_.y;
			for( auto y = p1_.y; y < p2_.y; y++ ){
				const auto x = w * y + p;
				canvas_.pixel( x, y ) = color_;
			}
		}
	}

	template<typename ColorT>
	void draw_rect(
		Rect<std::int32_t> const& rect_, 
		ColorT color_,
		surface<ColorT>& canvas_ ){
		const auto canvas_bounds = Rect<std::int32_t>{			
			0, 
			0, 
			static_cast<std::int32_t>( canvas_.width() ), 
			static_cast<std::int32_t>( canvas_.height() )
		};
		const auto clippped = clip_rect( rect_, canvas_bounds );
		draw_line( clippped.top_left(),    clippped.top_right(), color_, canvas_ );
		draw_line( clippped.top_right(),   clippped.bottom_right(), color_, canvas_ );
		draw_line( clippped.top_left(),    clippped.bottom_left(), color_, canvas_ );
		draw_line( clippped.bottom_left(), clippped.bottom_right(), color_, canvas_ );
	}

	template<typename ColorT>
	void fill_circle(
		dny::vector2<std::int32_t> const& center_,
		std::int32_t radius_,
		ColorT const& color_, 
		surface<ColorT>& canvas_ ){
		const auto rad_sq = radius_ * radius_;
		const auto circle_bounds = Rect<std::int32_t>{
			center_.x - radius_, center_.y - radius_,
			center_.x + radius_, center_.y + radius_
		};
		const auto canvas_bounds = Rect<std::int32_t>{
			0,
			0,
			static_cast< std::int32_t >( canvas_.width() ),
			static_cast< std::int32_t >( canvas_.height() )
		};
		const auto clipped = clip_rect( circle_bounds, canvas_bounds );

		for( auto y = clipped.top; y < clipped.bottom; ++y ){
			for( auto x = clipped.left; x < clipped.right; ++x ){
				const auto dx = x - center_.x;
				const auto dy = y - center_.y;
				if( ( dx * dx ) + ( dy * dy ) <= rad_sq ){
					canvas_.pixel( x, y ) = color_;
				}
			}
		}
	}

	template<typename ColorT>
	void fill_rect(
		Rect<std::int32_t> const& rect_, 
		ColorT color_, 
		surface<ColorT>& canvas_ ){
		const auto canvas_bounds = Rect<std::int32_t>{
			0,
			0,
			static_cast< std::int32_t >( canvas_.width() ),
			static_cast< std::int32_t >( canvas_.height() )
		};
		const auto clipped = clip_rect( rect_, canvas_bounds );

		for( auto y = clipped.top; y < clipped.bottom; ++y ){
			for( auto x = clipped.left; x < clipped.right; ++x ){
				canvas_.pixel( x, y ) = color_;
			}
		}
	}

	template<typename ColorT>
	void draw_text( 
		std::string text_, 
		vector2<std::int32_t> position_,
		Font const& font_, 
		ColorT color_, 
		surface<ColorT>& canvas_ ){
		const auto canvas_bounds = Rect<std::int32_t>{
			0,
			0,
			static_cast< std::int32_t >( canvas_.width() ),
			static_cast< std::int32_t >( canvas_.height() )
		};

		static constexpr auto trans_black = Color32{ 0 };
		auto draw_char = [ & ]( vector2<std::int32_t> const& char_pos_, Rect<std::int32_t> const& char_rect ){
			for( std::int32_t y = 0; y < char_rect.height(); ++y ){
				for( std::int32_t x = 0; x < char_rect.width(); ++x ){
					auto src = font_.pixel( x + char_rect.left, y + char_rect.top );
					if( src != trans_black ){
						canvas_.pixel( x + char_pos_.x, y + char_pos_.y ) = color_;
					}
				}
			}
		};

		auto pen_x = position_.x;
		for( auto const& ch : text_ ){
			const auto char_rect = font_.glyph_rect( ch );
			const auto rect = Rect<std::int32_t>{
				pen_x,
				position_.y,
				pen_x + char_rect.width(),
				position_.y + char_rect.height()
			};

			if( rect.right > canvas_bounds.left && rect.left < canvas_bounds.right &&
				rect.bottom > canvas_bounds.top && rect.top < canvas_bounds.bottom ){
				draw_char( { rect.left, rect.top }, char_rect );
			}

			pen_x += font_.glyph_advance( ch );
		}
	}

}
