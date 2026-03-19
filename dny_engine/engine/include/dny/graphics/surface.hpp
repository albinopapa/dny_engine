#pragma once

#include "core/concepts.hpp"
#include "graphics/colors.hpp"
#include "math/math.hpp"
#include "graphics/image_loader.hpp"

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

	template<color_type ColorT>
	surface<ColorT> load_surface_from_file( std::filesystem::path filename_ ){
		using max_channel_type = std::conditional_t<std::is_same_v<ColorT, dny::ColorF>, float, std::uint8_t>;
		auto decide_value = []( std::uint8_t value ) -> max_channel_type{
			if constexpr( std::is_same_v<ColorT, dny::ColorF> ){
				return static_cast< float >( value ) / 255.f;
			}
			else{
				return value;
			}
		};

		const auto data = dny::load_image_data( filename_ );
		assert( data.channels_per_pixel == 4 ); // We expect RGBA data from the loader

		auto result = surface<ColorT>{ data.width, data.height };

		for( std::size_t j = 0; j < data.width * data.height; ++j ){
			const auto chanel_offset = j * data.channels_per_pixel;

			result.pixels()[ j ] = ColorT( 
				decide_value( data.pixels[ chanel_offset + 2 ] ), 
				decide_value( data.pixels[ chanel_offset + 1 ] ), 
				decide_value( data.pixels[ chanel_offset + 0 ] ), 
				decide_value( data.pixels[ chanel_offset + 3 ] )
			);
		}

		return result;
	}
}