#pragma once

#include "dny_font.hpp"
#include "dny_colors.hpp"
#include "dny_soft_renderer.hpp"
#include "dny_math.hpp"
#include "dny_simd.hpp"
#include "dny_surface.hpp"
#include "dny_utilities.hpp"

#include <utility>

namespace dny{
	Rect<std::int32_t> clip_rect( Rect<std::int32_t>const& src_, Rect<std::int32_t> const& bounds_ ){
		return Rect<std::int32_t>{
			std::max( -src_.left, 0 ),
				std::max( -src_.top, 0 ),
				std::min<std::int32_t>( bounds_.width() - src_.left, src_.width()),
				std::min<std::int32_t>( bounds_.height() - src_.top, src_.height())
		};
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
		draw_line( rect_.top_left(), rect_.top_right(), color_, canvas_ );
		draw_line( rect_.top_right(), rect_.bottom_right(), color_, canvas_ );
		draw_line( rect_.top_left(), rect_.bottom_left(), color_, canvas_ );
		draw_line( rect_.bottom_left(), rect_.bottom_right(), color_, canvas_ );
	}
	template<typename ColorT>
	void fill_circle(
		dny::vector2<std::int32_t> const& center_,
		std::int32_t radius_,
		ColorT const& color_, 
		surface<ColorT>& canvas_ ){
		const auto rad_sq = radius_ * radius_;
		const auto clipped = clip_rect( {
			center_.x - radius_,
			center_.y - radius_,
			center_.x + radius_,
			center_.y + radius_,
			} );

		for( auto y = clipped.top - radius_; y < clipped.bottom - radius_; ++y ){
			for( auto x = clipped.left - radius_; x < clipped.right - radius_; ++x ){
				const auto dist_sq = ( x * x ) + ( y * y );
				if( dist_sq <= rad_sq ){
					canvas_.pixel(
						clipped.left + x + center_.x,
						clipped.top + y + center_.y
					) = color_;
				}
			}
		}
	}
	template<typename ColorT>
	void fill_rect(
		Rect<std::int32_t> const& rect_, 
		ColorT color_, 
		surface<ColorT>& canvas_ ){
		const auto clipped = clip_rect( rect_ );

		for( auto y = clipped.top; y < clipped.bottom; ++y ){
			for( auto x = clipped.left; x < clipped.right; ++x ){
				canvas_.pixel( x, y ) = color_;
			}
		}
	}
	template<typename ColorT>
	void draw( 
		std::string text, 
		vector2<std::int32_t> position, 
		Font const& font, 
		ColorT color, 
		surface<ColorT>& canvas_ ){
		for( std::int32_t i = 0; auto const& ch : text ){
			const auto char_rect = internal::get_char_rect(
				ch,
				font.char_width(),
				font.char_height()
			);
			const auto spx = position.x + ( i * char_rect.width() );

			const auto surf_width = static_cast< std::int32_t >( m_width );
			const auto surf_height = static_cast< std::int32_t >( m_height );

			if( position.x + char_rect.right < 0 || position.x + char_rect.left >= surf_width )
				continue;
			if( position.y + char_rect.bottom < 0 || position.y + char_rect.top >= surf_height )
				continue;

			for( std::int32_t y = 0; y < char_rect.height(); ++y ){
				for( std::int32_t x = 0; x < char_rect.width(); ++x ){
					auto src = font.pixel( x + char_rect.left, y + char_rect.top );
					if( src != Color32{ 0 } ){
						canvas_.pixel( x + spx, y + position.y ) = color;
					}
				}
			}

			++i;
		}
	}

}
