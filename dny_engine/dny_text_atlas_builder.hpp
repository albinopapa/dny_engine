#pragma once

#include "dny_d2dsdk.hpp"
#include "dny_dwritesdk.hpp"
#include "dny_image_loader.hpp"
#include "dny_rectangle.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace dny::internal{
	inline auto generate_chars() -> std::string{
		std::string charList;
		charList.resize( 127 - 32 );
		char c = ' ';
		std::generate( charList.begin(), charList.end(), [ &c ](){
			return c++;
		} );
		return charList;
	}

	inline auto make_image( IWICBitmap* bitmap ) -> image_data{
		auto [width, height] = wic::get_bitmap_size( bitmap );

		auto lock = wic::create_bitmap_lock(
			bitmap,
			WICRect{
				.Width = static_cast< int >( width ),
				.Height = static_cast< int >( height )
			},
			WICBitmapLockRead
		);

		const auto src = std::span<const Color32>(
			reinterpret_cast< Color32 const* >( wic::get_bitmap_pixels( lock.Get() ).data() ),
			width * height
		);

		const auto stride = wic::get_bitmap_stride( lock.Get() ) / 4;
		image_data img;
		img.channels_per_pixel = 4;
		img.height = height;
		img.width = width;
		img.pixels = std::make_unique<std::uint8_t[]>( width * height * 4 );
		img.stride = width * img.channels_per_pixel;
		auto dst = std::span<Color32>(
			reinterpret_cast< Color32* >( img.pixels.get() ),
			img.width * img.height
		);

		for( std::int32_t y = 0; y < static_cast< std::int32_t >( height ); ++y ){
			const auto src_row = src.subspan( y * stride, stride );
			auto dst_row = dst.subspan( y * width, width );
			
			std::copy(
				src_row.begin(),
				src_row.end(),
				dst_row.begin()
			);
		}

		return img;
	};

	inline Rect<std::int32_t> get_char_rect( const char C, std::int32_t char_width, std::int32_t char_height ){
		const int fontIndex = C - ' ';

		const auto fontCol = fontIndex % 32;
		const auto fontRow = fontIndex / 32;

		const auto left = fontCol * char_width;
		const auto top  = fontRow * char_height;

		return {
			left,
			top,
			left + char_width,
			top  + char_height
		};
	}

	inline void render_atlas(
		std::int32_t char_width_,
		std::int32_t char_height_,
		std::int32_t chars_per_row_,
		std::int32_t num_rows_,
		ID2D1RenderTarget* pRenderTarget,
		ID2D1SolidColorBrush*  pBrush,
		const std::string& charList,
		IDWriteTextFormat* text_format_ ){

		const auto rt_size = pRenderTarget->GetSize();
		const auto wCharList = win32::utf8_to_wstring( charList );

		pRenderTarget->BeginDraw();
		for( int y = 0; y < num_rows_; ++y ){
			for( int x = 0; x < chars_per_row_; ++x ){
				const auto index = x + ( y * chars_per_row_ );
				const auto rect = get_char_rect( 
					charList[ index ],
					char_width_,
					char_height_
				);
				const auto d2d_rect = D2D1_RECT_F{
					static_cast< float >( rect.left ),
					static_cast< float >( rect.top ),
					static_cast< float >( rect.right ),
					static_cast< float >( rect.bottom )
				};
				
				const auto wc = wCharList[ index ];
				pRenderTarget->DrawText(
					&wc,
					1u,
					text_format_,
					d2d_rect,
					pBrush
				);
			}
		}
		const auto hr = pRenderTarget->EndDraw();
		if( FAILED( hr ) ){
			throw win32::win32_error{ "Error rendering text atlas." };
		}
	}

	inline auto make_atlas(
		std::wstring const& font_name_,
		std::uint32_t chars_per_row_,
		std::uint32_t num_rows_,
		std::uint32_t font_size_ ){
		auto format_props = dwrite::text_format_create_properties{
			.font_name = font_name_,
			.font_height = static_cast< float >( font_size_ )
		};

		auto dwrite_factory = dwrite::create_factory();
		auto text_format = dwrite::create_text_format( 
			dwrite_factory.Get(), 
			format_props 
		);
		auto text_layout = dwrite::create_text_layout( 
			dwrite_factory.Get(),
			L"Q",
			text_format.Get() 
		);
		const auto [font_width, font_height] = dwrite::get_font_size( text_layout.Get() );
		const auto char_width  = static_cast< std::int32_t >( font_width );
		const auto char_height = static_cast< std::int32_t >( font_height );

		auto imaging_factory = wic::create_imaging_factory();
		auto wic_bitmap = wic::create_bitmap(
			imaging_factory.Get(),
			chars_per_row_ * char_width,
			num_rows_ * char_height
		);

		auto d2d_factory = d2d::create_factory();
		auto render_target = d2d::create_bitmap_render_target(
			d2d_factory.Get(),
			wic_bitmap.Get(),
			{}
		);
		auto brush = d2d::create_solid_color_brush(
			render_target.Get(),
			D2D1::ColorF{ D2D1::ColorF::White }
		);

		const auto char_list = generate_chars();
		const auto wchar_list = win32::utf8_to_wstring( char_list );
		
		render_atlas(
			char_width,
			char_height,
			chars_per_row_,
			num_rows_,
			render_target.Get(), 
			brush.Get(), 
			char_list,
			text_format.Get()
		);

		return make_image( wic_bitmap.Get() );
	}
}