#pragma once

#include "dny_wicsdk.hpp"

#include <memory>

namespace dny{
	struct image_data{
		std::uint32_t width = {};
		std::uint32_t height = {};
		std::uint32_t channels_per_pixel = {};
		std::uint32_t stride = {};
		std::unique_ptr<std::uint8_t[]> pixels;
	};

	inline image_data load_image_data( std::filesystem::path const& filename ){
		auto factory = wic::create_imaging_factory();
		auto decoder = wic::create_bitmap_decoder( factory.Get(), filename );
		auto bitmap_frame = wic::create_bitmap_frame( decoder.Get() );

		const auto num_color_channels =
			wic::get_num_channels( factory.Get(), bitmap_frame.Get() );

		auto bitmap = wic::create_bitmap( factory.Get(), bitmap_frame.Get() );
		auto [width, height] = wic::get_bitmap_size( bitmap.Get() );
		const auto region = WICRect{
			.Width = static_cast< std::int32_t >( width ),
			.Height = static_cast< std::int32_t >( height )
		};
		auto lock = wic::create_bitmap_lock( bitmap.Get(), region, WICBitmapLockRead );
		const auto stride = wic::get_bitmap_stride( lock.Get() );
		auto u8_pixels = wic::get_bitmap_pixels( lock.Get() );

		auto pixels = std::make_unique<std::uint8_t[]>( stride * height );

		auto pixel_span = std::span<std::uint8_t>{ pixels.get(), stride * height };
		std::copy(
			u8_pixels.begin(),
			u8_pixels.end(),
			pixel_span.begin()
		);

		auto data = image_data{
			.width = width,
			.height = height,
			.channels_per_pixel = num_color_channels,
			.stride = stride,
			.pixels = std::move( pixels )
		};

		return data;
	}


}