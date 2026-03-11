#pragma once

#include "win32sdk.hpp"

#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

namespace d2d{
	inline auto create_factory( D2D1_FACTORY_TYPE type = D2D1_FACTORY_TYPE_SINGLE_THREADED ){
		auto factory = wrl::ComPtr<ID2D1Factory>{};
		auto hr = 
			D2D1CreateFactory( type, IID_PPV_ARGS( factory.GetAddressOf() ) );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create Direct2D factory." };
		}

		return factory;
	}
	inline auto create_bitmap_render_target(
		ID2D1Factory* factory, 
		IWICBitmap* source, 
		std::optional<D2D1_RENDER_TARGET_PROPERTIES> props ){
		auto target = wrl::ComPtr<ID2D1RenderTarget>{};
		if( !props ){
			props = std::optional<D2D1_RENDER_TARGET_PROPERTIES>{
				D2D1::RenderTargetProperties()
			};
		}
		auto hr = factory->CreateWicBitmapRenderTarget( source, *props, target.GetAddressOf() );

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create bitmap render target." };
		}

		return target;
	}
	inline auto create_solid_color_brush( ID2D1RenderTarget* target, D2D1_COLOR_F color ){
		auto brush = wrl::ComPtr<ID2D1SolidColorBrush>{};
		const auto hr = 
			target->CreateSolidColorBrush( color, brush.GetAddressOf() );

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create solid color brush." };
		}

		return brush;
	}
}