#include "graphics/text_atlas_builder.hpp"
#include "core/colors.hpp"
#include "platform/d2dsdk.hpp"
#include "platform/dwritesdk.hpp"
#include "platform/wicsdk.hpp"

#include <algorithm>
#include <cmath>
#include <tuple>

namespace dny::internal{
	auto is_supported_char( char c ) -> bool{
		return c >= first_char && c <= last_char;
	}

	auto to_glyph_index( char c ) -> std::size_t{
		if( !is_supported_char( c ) ){
			return static_cast< std::size_t >( '?' - first_char );
		}

		return static_cast< std::size_t >( c - first_char );
	}

	auto generate_chars() -> std::string{
		std::string char_list;
		char_list.resize( glyph_count );
		char c = first_char;
		std::generate( char_list.begin(), char_list.end(), [ &c ](){ return c++; } );
		return char_list;
	}

	auto make_image( IWICBitmap* bitmap ) -> image_data{
		auto size = wic::get_bitmap_size( bitmap );
		const auto width = size.first;
		const auto height = size.second;

		auto lock = wic::create_bitmap_lock(
			bitmap,
			WICRect{
				.Width = static_cast< int >( width ),
				.Height = static_cast< int >( height )
			},
			WICBitmapLockRead
		);

		const auto src_pixels = reinterpret_cast< Color32 const* >( wic::get_bitmap_pixels( lock.Get() ).data() );
		const auto src_stride = wic::get_bitmap_stride( lock.Get() ) / 4;

		image_data img;
		img.channels_per_pixel = 4;
		img.height = height;
		img.width = width;
		img.pixels = std::make_unique<std::uint8_t[]>( width * height * 4 );
		img.stride = width * img.channels_per_pixel;

		auto dst_pixels = reinterpret_cast< Color32* >( img.pixels.get() );
		for( std::int32_t y = 0; y < static_cast< std::int32_t >( height ); ++y ){
			const auto* src_row = src_pixels + ( y * src_stride );
			auto* dst_row = dst_pixels + ( y * width );
			std::copy( src_row, src_row + width, dst_row );
		}

		return img;
	};

	auto measure_glyph( IDWriteFactory* dwrite_factory, IDWriteTextFormat* text_format, wchar_t wc ){
		auto layout = dwrite::create_text_layout(
			dwrite_factory,
			std::wstring{ 1, wc },
			text_format
		);

		auto metrics = DWRITE_TEXT_METRICS{};
		layout->GetMetrics( &metrics );

		const auto draw_width = std::max(
			1,
			static_cast< std::int32_t >( std::ceil( metrics.width ) )
		);
		const auto advance = std::max(
			draw_width,
			static_cast< std::int32_t >( std::ceil( metrics.widthIncludingTrailingWhitespace ) )
		);
		const auto height = std::max(
			1,
			static_cast< std::int32_t >( std::ceil( metrics.height ) )
		);

		return std::tuple{ draw_width, advance, height };
	}

	void render_atlas(
		text_atlas const& atlas,
		ID2D1RenderTarget* render_target,
		ID2D1SolidColorBrush* brush,
		IDWriteTextFormat* text_format ){
		const auto char_list = generate_chars();
		const auto wchar_list = win32::utf8_to_wstring( char_list );

		render_target->BeginDraw();
		render_target->Clear( D2D1::ColorF( 0.f, 0.f, 0.f, 0.f ) );

		for( std::size_t i = 0; i < glyph_count; ++i ){
			const auto& glyph = atlas.glyphs[ i ];
			const auto& rect = glyph.atlas_rect;
			if( rect.width() <= 0 || rect.height() <= 0 ){
				continue;
			}

			const auto d2d_rect = D2D1_RECT_F{
				static_cast< float >( rect.left ),
				static_cast< float >( rect.top ),
				static_cast< float >( rect.right ),
				static_cast< float >( rect.bottom )
			};
			const auto wc = wchar_list[ i ];

			render_target->DrawText(
				&wc,
				1u,
				text_format,
				d2d_rect,
				brush
			);
		}

		const auto hr = render_target->EndDraw();
		if( FAILED( hr ) ){
			throw win32::win32_error{ "Error rendering text atlas." };
		}
	}

	auto get_char_rect( text_atlas const& atlas, char c ) -> Rect<std::int32_t>{
		return atlas.glyphs[ to_glyph_index( c ) ].atlas_rect;
	}

	auto get_char_advance( text_atlas const& atlas, char c ) -> std::int32_t{
		return atlas.glyphs[ to_glyph_index( c ) ].advance;
	}

	auto make_atlas(
		std::wstring const& font_name_,
		std::uint32_t font_size_ ) -> text_atlas{
		auto format_props = dwrite::text_format_create_properties{
			.font_name = font_name_,
			.font_height = static_cast< float >( font_size_ )
		};

		auto dwrite_factory = dwrite::create_factory();
		auto text_format = dwrite::create_text_format(
			dwrite_factory.Get(),
			format_props
		);

		text_atlas atlas;
		const auto char_list = generate_chars();
		static constexpr std::int32_t padding = 1;

		auto cursor_x = padding;
		auto max_height = 1;
		for( std::size_t i = 0; i < glyph_count; ++i ){
			const auto wc = static_cast< wchar_t >( static_cast< unsigned char >( char_list[ i ] ) );
			auto [draw_width, advance, height] = measure_glyph(
				dwrite_factory.Get(),
				text_format.Get(),
				wc
			);

			atlas.max_glyph_width = std::max( atlas.max_glyph_width, draw_width );
			max_height = std::max( max_height, height );

			auto& glyph = atlas.glyphs[ i ];
			glyph.atlas_rect = {
				cursor_x,
				padding,
				cursor_x + draw_width,
				padding + height
			};
			glyph.advance = advance;
			cursor_x += draw_width + padding;
		}

		atlas.line_height = max_height;

		auto imaging_factory = wic::create_imaging_factory();
		auto wic_bitmap = wic::create_bitmap(
			imaging_factory.Get(),
			static_cast< std::uint32_t >( cursor_x + padding ),
			static_cast< std::uint32_t >( max_height + ( padding * 2 ) )
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

		render_atlas(
			atlas,
			render_target.Get(),
			brush.Get(),
			text_format.Get()
		);

		atlas.image = make_image( wic_bitmap.Get() );
		return atlas;
	}
}