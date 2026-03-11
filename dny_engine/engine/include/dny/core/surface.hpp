#pragma once

#include "concepts.hpp"
#include "colors.hpp"
#include "math/math.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>

namespace dny{
	template<color_type ColorT>
	class surface{
	public:
		using color_type_t = ColorT;

	public:
		surface()noexcept = default;
		surface( std::uint32_t width_, std::uint32_t height_ )
			:
			m_width( width_ ), m_height( height_ ),
			m_pixels( std::make_unique<color_type_t[]>( width_* height_ ) ){}

		color_type_t pixel( std::uint32_t x, std::uint32_t y )const{
			assert( x < m_width );
			assert( y < m_height );
			const auto i = x + y * m_width;
			return m_pixels[ i ];
		}
		color_type_t& pixel( std::uint32_t x, std::uint32_t y ){
			assert( x < m_width );
			assert( y < m_height );
			const auto i = x + y * m_width;
			return m_pixels[ i ];
		}

		std::uint32_t width()const noexcept{
			return m_width;
		}
		std::uint32_t height()const noexcept{
			return m_height;
		}

		color_type_t const* pixels()const noexcept{
			return m_pixels.get();
		}
		color_type_t* pixels()noexcept{
			return m_pixels.get();
		}

	private:
		std::unique_ptr<color_type_t[]> m_pixels;
		std::uint32_t m_width = 0u;
		std::uint32_t m_height = 0u;
	};
}