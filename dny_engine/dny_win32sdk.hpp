#pragma once

#include <string>
#include <string_view>
#include <system_error>
#include <optional>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <wrl/client.h>

namespace win32{
	class win32_error :public std::system_error{
	public:
		win32_error( HRESULT hr, std::string const& message )
			:
			std::system_error( hr, std::system_category(), message ){}
		win32_error( std::string const& message )
			:
			std::system_error( GetLastError(), std::system_category(), message ){}
	};

	class ComApartment{
	public:
		explicit ComApartment( DWORD flags = COINIT_MULTITHREADED ) noexcept
			: _hr( CoInitializeEx( nullptr, flags ) ){
		}

		~ComApartment(){
			if( SUCCEEDED( _hr ) ){
				CoUninitialize();
			}
		}

		// Non-copyable
		ComApartment( const ComApartment& ) = delete;
		ComApartment& operator=( const ComApartment& ) = delete;

		// Movable (optional but useful)
		ComApartment( ComApartment&& other ) noexcept
			: _hr( other._hr ){
			other._hr = E_FAIL;
		}

		ComApartment& operator=( ComApartment&& other ) noexcept{
			if( this != &other ){
				if( SUCCEEDED( _hr ) )
					CoUninitialize();

				_hr = other._hr;
				other._hr = E_FAIL;
			}
			return *this;
		}

		HRESULT result() const noexcept{ return _hr; }
		bool ok() const noexcept{ return SUCCEEDED( _hr ); }

	private:
		HRESULT _hr;
	};

	inline auto get_desktop(){
		return GetDesktopWindow();
	}
	inline auto get_module_handle(){
		return reinterpret_cast< HINSTANCE >( GetModuleHandle( nullptr ) );
	}
	inline auto adjust_window_rect( std::int32_t width, std::int32_t height, std::uint32_t style, std::uint32_t ex_style, bool has_menu ){
		RECT result = { 0, 0, static_cast< long >( width ), static_cast< long >( height ) };
		AdjustWindowRectEx( &result, style, has_menu ? TRUE : FALSE, ex_style );
		return result;
	}

	inline auto register_window_class( WNDCLASSEX const& wc ){
		SetLastError( 0u );
		const auto atom = RegisterClassEx( &wc );
		if( atom == 0 ){
			throw win32_error( "Failed to register window class." );
		}

		return atom;
	}
	inline void unregister_window_class( std::wstring const& classname, HINSTANCE instance ){
		UnregisterClass( classname.c_str(), instance );
	}

	inline auto get_client_rect( HWND handle ){
		RECT rect;
		GetClientRect( handle, &rect );
		return rect;
	}
	inline auto create_window( CREATESTRUCT const& params ){
		SetLastError( 0 );
		auto hwnd = CreateWindowEx(
			params.dwExStyle,
			params.lpszClass,
			params.lpszName,
			params.style,
			params.x,
			params.y,
			params.cx,
			params.cy,
			params.hwndParent,
			params.hMenu,
			params.hInstance,
			params.lpCreateParams
		);
		if( !hwnd ){
			throw win32_error{ "Failed to create the window instance." };
		}

		return hwnd;
	}
	inline void show_window( HWND handle, std::int32_t flag = SW_SHOWDEFAULT ){
		ShowWindow( handle, flag );
	}
	inline void destroy_window( HWND handle ){
		DestroyWindow( handle );
	}

	template<typename T>
	void set_window_long_ptr(HWND handle_,std::int32_t index_, T& t ){
		SetWindowLongPtr(
			handle_,
			index_,
			reinterpret_cast< LONG_PTR >( std::addressof( t ) )
		);
	}
	template<typename T>
	T* get_window_long_ptr( HWND handle_, std::int32_t index_ ){
		T* ptr = reinterpret_cast< T* >( GetWindowLongPtr( handle_, index_ ) );
		return ptr;
	}

	template<typename T>
	void set_userdata( HWND handle_, T& t ){
		set_window_long_ptr( handle_, GWLP_USERDATA, t );
	}
	template<typename T>
	T* get_user_data( HWND handle_ ){
		return get_window_long_ptr<T>( handle_, GWLP_USERDATA );
	}


	inline std::optional<MSG> peek_message(
		HWND handle = nullptr,
		std::uint32_t filter_min = 0u,
		std::uint32_t filter_max = 0u,
		std::int32_t flag = PM_REMOVE ){
		auto msg = MSG{};
		if( PeekMessage( &msg, handle, filter_min, filter_max, flag ) == 0 ){
			return std::nullopt;
		}

		return msg;
	}
	inline void translate_message( MSG const& msg ){
		TranslateMessage( &msg );
	}
	inline void dispatch_message( MSG const& msg ){
		DispatchMessage( &msg );
	}

	inline auto utf8_to_wstring( const std::string& str ){
		if( str.empty() )
			return std::wstring{};

		// First pass: get required size (includes null terminator)
		const auto size = MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			nullptr,
			0
		);

		if( size == 0 ){
			throw win32_error( "Failed to convert UTF-8 string to UTF-16" );
		}

		auto result = std::wstring( 
			static_cast< std::size_t >( size - 1 ), // exclude null terminator
			L'\0' 
		); 

		// Second pass: perform conversion
		MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			result.data(),
			size
		);

		return result;
	}


}

namespace wrl = Microsoft::WRL;
