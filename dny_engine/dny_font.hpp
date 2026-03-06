#pragma once

#include "dny_rectangle.hpp"
#include "dny_colors.hpp"
#include "dny_text_atlas_builder.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dny{
	class Font{
	public:
		Font( std::wstring const& font_name, std::uint32_t font_height ){
			const auto img = internal::make_atlas(
				font_name, 
				m_max_chars_per_row,
				m_max_rows,
				font_height 
			);
			m_char_width = img.width / m_max_chars_per_row;
			m_char_height = img.height / m_max_rows;
			const auto img_data = std::span<const Color32>{
				reinterpret_cast< Color32 const* >( img.pixels.get() ),
				img.width * img.height
			};

			m_pixels.resize( img_data.size() );

			std::copy( img_data.begin(), img_data.end(), m_pixels.begin() );
		}
		std::int32_t char_width()const{ return m_char_width; }
		std::int32_t char_height()const{ return m_char_height; }
		Color32 pixel( std::int32_t x, std::int32_t y )const{
			const auto sheet_width = m_char_width * m_max_chars_per_row;
			if( std::abs( x ) >= sheet_width )return Colors::magenta;
			if( std::abs( y ) >= m_char_height * m_max_rows )return Colors::magenta;
			return m_pixels[ x + y * sheet_width ];
		}

	private:
		static constexpr std::int32_t m_max_chars_per_row = 32;
		static constexpr std::int32_t m_max_rows = 3;
		std::vector<dny::Color32> m_pixels;
		std::int32_t m_char_width = {};
		std::int32_t m_char_height = {};
	};
}