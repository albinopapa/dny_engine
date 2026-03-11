#pragma once

#include "dny_win32sdk.hpp"

#include <dwrite.h>

#pragma comment(lib, "dwrite.lib")

namespace dwrite{
	struct text_format_create_properties{
		std::wstring font_name				   = L"Consolas";
		float font_height					   = 32.f;
		DWRITE_FONT_WEIGHT font_weight		   = DWRITE_FONT_WEIGHT_NORMAL;
		DWRITE_FONT_STYLE font_style		   = DWRITE_FONT_STYLE_NORMAL;
		DWRITE_FONT_STRETCH font_stretch	   = DWRITE_FONT_STRETCH_NORMAL;
		std::wstring locale					   = L"en_us";
		IDWriteFontCollection* font_collection = nullptr;
	};

	inline auto create_factory(){
		auto factory = wrl::ComPtr<IDWriteFactory>{};
		auto hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof( IDWriteFactory ),
			reinterpret_cast< IUnknown** >( factory.GetAddressOf() )
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create DirectWrite factory." };
		}

		return factory;
	}
	inline auto create_text_format(
		IDWriteFactory* factory, 
		text_format_create_properties const& props){
		auto format = wrl::ComPtr<IDWriteTextFormat>{};
		const auto hr = factory->CreateTextFormat(
			props.font_name.c_str(), 
			props.font_collection,
			props.font_weight,
			props.font_style,
			props.font_stretch,
			props.font_height,
			props.locale.c_str(),
			format.GetAddressOf()
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create DWrite text format." };
		}

		return format;
	}
	inline auto create_text_layout( 
		IDWriteFactory* factory, 
		std::wstring const& string, 
		IDWriteTextFormat* format ){
		auto layout = wrl::ComPtr<IDWriteTextLayout>{};
		const auto hr = factory->CreateTextLayout(
			string.c_str(),
			static_cast< std::uint32_t >( string.size() ),
			format,
			FLT_MAX,
			FLT_MAX,
			layout.GetAddressOf()
		);

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create DWrite text layout." };
		}

		return layout;
	}
	inline auto get_font_size( IDWriteTextLayout* layout ){
		auto metrics = DWRITE_TEXT_METRICS{};
		layout->GetMetrics( &metrics );
		
		return std::pair{ metrics.width, metrics.height };
	}
}