#pragma once

#include "dny_display.hpp"
#include "dny_math.hpp"
#include "dny_input.hpp"

#include <cstdint>
#include <span>

namespace dny{
	static constexpr std::int32_t screen_width = 1280;
	static constexpr std::int32_t screen_height = 720;

	class platform{
	public:
		platform()
			:
			m_input( m_keyboard, m_mouse, m_gamepad ),
			m_display( screen_width, screen_height, *this ){
			m_display.show();
		}

		void process_message_pump(){
			m_keyboard.begin_frame();
			m_mouse.begin_frame();
			m_gamepad.begin_frame();

			while( auto msg = win32::peek_message() ){
				if( msg->message == WM_KEYDOWN && msg->wParam == VK_SPACE ){
					int a = 0;
				}
				if( msg->message == WM_KEYUP && msg->wParam == VK_SPACE ){
					int a = 0;
				}
				win32::translate_message( *msg );
				win32::dispatch_message( *msg );
			}

			m_gamepad.poll();
		}
		void update_view( std::int32_t width_, std::int32_t height_, std::span<const dny::Color32> pixels_ ) const{
			m_display.present( width_, height_, pixels_ );
		}
		bool is_done() const{
			return m_done;
		}

		input& get_input() noexcept{
			return m_input;
		}
		input const& get_input() const noexcept{
			return m_input;
		}

		Keyboard const& keyboard() const noexcept{ return m_keyboard; }
		Mouse const& mouse() const noexcept{ return m_mouse; }
		Gamepad const& gamepad() const noexcept{ return m_gamepad; }

		dny::vector2<float> mouse_pos() const noexcept{
			auto const pos = m_mouse.position();
			return{ static_cast<float>( pos.x ), static_cast<float>( pos.y ) };
		}
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
			if( msg == WM_CLOSE ){
				m_done = true;
				return 0;
			}
			//if( msg == WM_KILLFOCUS ){
			//	m_keyboard.clear();
			//	m_mouse.clear();
			//	return 0;
			//}
			if( m_keyboard.handle_message( msg, wparam ) ){
				return 0;
			}
			if( m_mouse.handle_message( msg, wparam, lparam ) ){
				return 0;
			}

			return DefWindowProcW(
				reinterpret_cast< HWND >( m_display.handle() ),
				msg,
				wparam,
				lparam
			);
		}
		void set_title( std::wstring title_ ){
			SetWindowTextW(
				reinterpret_cast< HWND >( m_display.handle() ),
				title_.c_str()
			);
		}
	private:
		Keyboard m_keyboard;
		Mouse m_mouse;
		Gamepad m_gamepad;
		input m_input;
		dny::display m_display;
		bool m_done = false;
	};
}
