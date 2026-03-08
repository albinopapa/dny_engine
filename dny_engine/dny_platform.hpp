#pragma once

#include "dny_math.hpp"
#include "dny_display.hpp"
#include "dny_input.hpp"

#include <cstdint>

namespace dny{
	static constexpr std::int32_t screen_width = 1280;
	static constexpr std::int32_t screen_height = 720;

	class platform{
	public:
		platform()
			:
			m_display( screen_width, screen_height, *this ) {
			m_display.show();
		}

		void process_message_pump(){
			m_keyboard.begin_frame();
			m_mouse.begin_frame();
			while( auto msg = win32::peek_message() ){
				win32::translate_message( *msg );
				win32::dispatch_message( *msg );
			}
		}
		void update_view( std::int32_t width_, std::int32_t height_, std::span<const dny::Color32> pixels_ )const{
			m_display.present( width_, height_, pixels_ );
		}
		bool is_done()const{
			return m_done;
		}

		keyboard const& keyboard_state()const noexcept{ return m_keyboard; }
		mouse const& mouse_state()const noexcept{ return m_mouse; }

		void hide_mouse(){
			ShowCursor( FALSE );
		}
		void show_mouse(){
			ShowCursor( TRUE );
		}
		void clamp_mouse_to_window(){
			m_display.clamp_cursor();
		}
		void free_mouse(){
			m_display.unclamp_cursor();
		}
		void recenter_mouse(){
			RECT rect = {};
			GetClientRect( reinterpret_cast< HWND >( m_display.handle() ), &rect );
			MapWindowPoints( reinterpret_cast< HWND >( m_display.handle() ), nullptr, reinterpret_cast< POINT* >( &rect ), 2 );
			auto center_x = ( rect.left + rect.right ) / 2;
			auto center_y = ( rect.top + rect.bottom ) / 2;
			SetCursorPos( center_x, center_y );
		}

		LRESULT message_proc( UINT msg, WPARAM wparam, LPARAM lparam ){
			switch( msg ){
				case WM_CLOSE:
					m_done = true;
					return 0;
				case WM_KILLFOCUS:
					m_keyboard.clear();
					m_mouse.clear();
					m_display.unclamp_cursor();
					return 0;
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					m_keyboard.on_key_down( static_cast< std::uint32_t >( wparam ) );
					return 0;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					m_keyboard.on_key_up( static_cast< std::uint32_t >( wparam ) );
					return 0;
				case WM_MOUSEMOVE:
					m_mouse.on_move( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
					return 0;
				case WM_LBUTTONDOWN:
					m_mouse.on_button_down( mouse_button::left );
					return 0;
				case WM_LBUTTONUP:
					m_mouse.on_button_up( mouse_button::left );
					return 0;
				case WM_RBUTTONDOWN:
					m_mouse.on_button_down( mouse_button::right );
					return 0;
				case WM_RBUTTONUP:
					m_mouse.on_button_up( mouse_button::right );
					return 0;
				case WM_MBUTTONDOWN:
					m_mouse.on_button_down( mouse_button::middle );
					return 0;
				case WM_MBUTTONUP:
					m_mouse.on_button_up( mouse_button::middle );
					return 0;
				case WM_XBUTTONDOWN:
					if( GET_XBUTTON_WPARAM( wparam ) == XBUTTON1 ){
						m_mouse.on_button_down( mouse_button::x1 );
					}else if( GET_XBUTTON_WPARAM( wparam ) == XBUTTON2 ){
						m_mouse.on_button_down( mouse_button::x2 );
					}
					return TRUE;
				case WM_XBUTTONUP:
					if( GET_XBUTTON_WPARAM( wparam ) == XBUTTON1 ){
						m_mouse.on_button_up( mouse_button::x1 );
					}else if( GET_XBUTTON_WPARAM( wparam ) == XBUTTON2 ){
						m_mouse.on_button_up( mouse_button::x2 );
					}
					return TRUE;
				case WM_MOUSEWHEEL:
					m_mouse.on_wheel( static_cast<float>( GET_WHEEL_DELTA_WPARAM( wparam ) ) / static_cast<float>( WHEEL_DELTA ) );
					return 0;
				default:
					return DefWindowProcW(
						reinterpret_cast< HWND >( m_display.handle() ),
						msg,
						wparam,
						lparam
					);
			}
		}
		void set_title( std::wstring title_ ){
			SetWindowTextW(
				reinterpret_cast< HWND >( m_display.handle() ),
				title_.c_str()
			);
		}
	private:
		dny::display m_display;
		dny::keyboard m_keyboard;
		dny::mouse m_mouse;
		bool m_done = false;
	};
}
