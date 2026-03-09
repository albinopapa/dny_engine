#pragma once

#include <filesystem>
#include <memory>

namespace dny{
	struct image_data{
		std::uint32_t width = {};
		std::uint32_t height = {};
		std::uint32_t channels_per_pixel = {};
		std::uint32_t stride = {};
		std::unique_ptr<std::uint8_t[]> pixels;
	};

	image_data load_image_data( std::filesystem::path const& filename );
}