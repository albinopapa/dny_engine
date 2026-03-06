#pragma once

#include "dny_win32sdk.hpp"

#include <cstdint>
#include <filesystem>
#include <span>

#include "wincodec.h"

#pragma comment(lib, "windowscodecs.lib")

namespace wic{
	inline auto create_imaging_factory(){
		auto factory = wrl::ComPtr<IWICImagingFactory>{};
		auto hr = CoInitialize( nullptr );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to initialize COM." };
		}

		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS( factory.GetAddressOf() )
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create WIC imaging factory." };
		}

		return factory;
	}
	inline auto create_bitmap_decoder(
		IWICImagingFactory* factory,
		std::filesystem::path const& file_path,
		std::uint32_t rw_access = GENERIC_READ,
		WICDecodeOptions decode_option = WICDecodeMetadataCacheOnDemand,
		GUID* vendor_guid = nullptr ){
		auto decoder = wrl::ComPtr<IWICBitmapDecoder>{};
		const auto hr = factory->CreateDecoderFromFilename(
			file_path.wstring().c_str(),
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			decoder.GetAddressOf() );

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create bitmap decoder." };
		}

		return decoder;
	}
	inline auto create_bitmap_frame( IWICBitmapDecoder* decoder, std::uint32_t frame_index = 0u ){
		auto frame = wrl::ComPtr<IWICBitmapFrameDecode>{};
		const auto hr = decoder->GetFrame( frame_index, frame.GetAddressOf() );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create frame decode." };
		}
		
		return frame;
	}
	inline auto create_image_converter(
		IWICImagingFactory* factory,
		IWICBitmapSource* source,
		REFWICPixelFormatGUID format,
		double alpha_threshhold = 1.0,
		WICBitmapDitherType dither_type = WICBitmapDitherType::WICBitmapDitherTypeNone,
		WICBitmapPaletteType palette_type = WICBitmapPaletteTypeCustom,
		IWICPalette* palette = nullptr ){
		auto converter = wrl::ComPtr<IWICFormatConverter>{};
		auto hr = factory->CreateFormatConverter( converter.GetAddressOf() );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create format converter." };
		}

		hr = converter->Initialize( source, format, dither_type, palette, alpha_threshhold, palette_type );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to initialize format converter." };
		}

		return converter;
	}
	inline auto create_bitmap(
		IWICImagingFactory* factory,
		IWICBitmapSource* bitmap_source,
		WICBitmapCreateCacheOption cache_option = WICBitmapCacheOnDemand ){
		auto bitmap = wrl::ComPtr<IWICBitmap>{};
		const auto hr = factory->CreateBitmapFromSource( bitmap_source, cache_option, bitmap.GetAddressOf() );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create bitmap from bitmap source." };
		}
		
		return bitmap;
	}
	inline auto create_bitmap(
		IWICImagingFactory* factory, 
		std::uint32_t width, 
		std::uint32_t height,
		REFWICPixelFormatGUID const& format = GUID_WICPixelFormat32bppPBGRA,
		WICBitmapCreateCacheOption cache_option = WICBitmapCacheOnDemand ){
		auto bitmap = wrl::ComPtr<IWICBitmap>{};
		const auto hr = factory->CreateBitmap(
			width,
			height,
			format,
			cache_option,
			bitmap.GetAddressOf()
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create bitmap." };
		}
		
		return bitmap;
	}

	inline auto get_bitmap_size( IWICBitmap* bitmap ){
		auto size = std::pair{ 0u, 0u };
		bitmap->GetSize( &size.first, &size.second );
		return size;
	}
	inline auto create_bitmap_lock( IWICBitmap* bitmap, WICRect const& region, std::uint32_t flags ){
		auto lock = wrl::ComPtr<IWICBitmapLock>{};
		const auto hr = bitmap->Lock( &region, flags, lock.GetAddressOf() );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create bitmap lock." };
		}
		
		return lock;
	}
	inline auto get_bitmap_pixels( IWICBitmapLock* lock ){
		std::uint8_t* buffer = nullptr;
		std::uint32_t buffer_size = {};
		const auto hr = lock->GetDataPointer( &buffer_size, &buffer );

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to get bitmap pixels." };
		}

		return std::span{ buffer, buffer_size };
	}
	inline auto get_bitmap_stride( IWICBitmapLock* lock ){
		std::uint32_t stride = {};
		lock->GetStride( &stride );
		return stride;
	}
	inline auto get_num_channels(IWICImagingFactory* factory, IWICBitmapFrameDecode* frame ){
		WICPixelFormatGUID pixelFormat{};
		auto hr = frame->GetPixelFormat( &pixelFormat );

		auto componentInfo = wrl::ComPtr<IWICComponentInfo>{};
		factory->CreateComponentInfo( pixelFormat, &componentInfo );

		auto formatInfo = wrl::ComPtr<IWICPixelFormatInfo>{};
		componentInfo.As( &formatInfo );
		
		UINT channelCount = 0;
		formatInfo->GetChannelCount( &channelCount );

		return channelCount;
	}
}