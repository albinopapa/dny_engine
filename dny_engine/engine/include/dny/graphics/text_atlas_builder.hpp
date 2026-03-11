#pragma once

#include "dny_image_loader.hpp"
#include "dny_rectangle.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace dny::internal{
	auto get_char_rect( const char C, std::int32_t char_width, std::int32_t char_height ) -> Rect<std::int32_t>;

	auto make_atlas(
		std::wstring const& font_name_,
		std::uint32_t font_size_ ) -> image_data;
}