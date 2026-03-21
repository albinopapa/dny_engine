#pragma once

#include "utilities/dims2.hpp"
#include "utilities/rectangle.hpp"
#include "graphics/colors.hpp"
#include "text_atlas_builder.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dny{
	class Font{
	public:
		Font( std::wstring const& font_name,
			std::uint32_t font_height,
			std::int32_t scale = 1 ){
			m_scale = scale;

			const auto atlas = internal::make_atlas(
				font_name,
				font_height * scale   // ← render bigger
			);

			m_atlas_width = static_cast< std::int32_t >( atlas.image.width );
			m_atlas_height = static_cast< std::int32_t >( atlas.image.height );
			m_char_width = atlas.max_glyph_width / scale;
			m_char_height = atlas.line_height / scale;
			m_glyphs = atlas.glyphs;

			const auto img_data = std::span<const Color32>{
				reinterpret_cast< Color32 const* >( atlas.image.pixels.get() ),
				atlas.image.width * atlas.image.height
			};

			m_pixels.resize( img_data.size() );
			std::copy( img_data.begin(), img_data.end(), m_pixels.begin() );
		}

		static dims2<std::int32_t> measure_text( std::string_view text, Font const& font ) noexcept{
			if( text.empty() ){
				return { 0, 0 };
			}

			auto current_line_width = std::int32_t{ 0 };
			auto max_line_width = std::int32_t{ 0 };
			auto line_count = std::int32_t{ 1 };

			const auto scale = font.scale();
			for( const auto ch : text ){
				if( ch == '\n' ){
					max_line_width = std::max( max_line_width, current_line_width );
					current_line_width = 0;
					++line_count;
					continue;
				}

				current_line_width += font.glyph_advance( ch ) / scale;
			}

			max_line_width = std::max( max_line_width, current_line_width );
			return {
				max_line_width,
				line_count * font.char_height()
			};
		}

		std::int32_t char_width()const{ return m_char_width; }
		std::int32_t char_height()const{ return m_char_height; }
		std::int32_t atlas_width()const{ return m_atlas_width; }
		std::int32_t atlas_height()const{ return m_atlas_height; }

		Rect<std::int32_t> glyph_rect( char c )const{
			return m_glyphs[ internal::to_glyph_index( c ) ].atlas_rect;
		}
		std::int32_t glyph_advance( char c )const{
			return m_glyphs[ internal::to_glyph_index( c ) ].advance;
		}

		Color32 pixel( std::int32_t x, std::int32_t y )const{
			if( x < 0 || x >= m_atlas_width ) return Colors::magenta;
			if( y < 0 || y >= m_atlas_height ) return Colors::magenta;
			return m_pixels[ static_cast< std::size_t >( x + y * m_atlas_width ) ];
		}
		Color32 const* pixels()const noexcept{ return m_pixels.data(); }
		std::vector<Color32> const& pixel_data()const noexcept{ return m_pixels; }
		std::int32_t scale() const{ return m_scale; }
	private:
		std::vector<dny::Color32> m_pixels;
		std::array<internal::glyph_metrics, internal::glyph_count> m_glyphs{};
		std::int32_t m_atlas_width = {};
		std::int32_t m_atlas_height = {};
		std::int32_t m_char_width = {};
		std::int32_t m_char_height = {};
		std::int32_t m_scale = 1;
	};
}