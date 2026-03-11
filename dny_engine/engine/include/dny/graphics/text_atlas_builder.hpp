#pragma once

#include "image_loader.hpp"
#include "core/rectangle.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace dny::internal{
	static constexpr char first_char = ' ';
	static constexpr char last_char = '~';
	static constexpr std::size_t glyph_count = static_cast< std::size_t >( last_char - first_char + 1 );

	struct glyph_metrics{
		Rect<std::int32_t> atlas_rect{};
		std::int32_t advance = {};
	};

	struct text_atlas{
		image_data image;
		std::array<glyph_metrics, glyph_count> glyphs{};
		std::int32_t line_height = {};
		std::int32_t max_glyph_width = {};
	};

	auto is_supported_char( char c ) -> bool;
	auto to_glyph_index( char c ) -> std::size_t;
	auto get_char_rect( text_atlas const& atlas, char c ) -> Rect<std::int32_t>;
	auto get_char_advance( text_atlas const& atlas, char c ) -> std::int32_t;

	auto make_atlas(
		std::wstring const& font_name_,
		std::uint32_t font_size_ ) -> text_atlas;
}