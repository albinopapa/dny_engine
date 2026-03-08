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

		input const& get_input()const noexcept{
			return m_input;
		}

		dny::vector2<float> mouse_pos()const noexcept{
			auto mp = POINT{};
			GetCursorPos( &mp );
			return{
				static_cast< float >( mp.x ),
				static_cast< float >( mp.y )
			};
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
			switch( msg ){
				case WM_CLOSE:
					m_done = true;
					return 0;
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					m_input.on_key_down( static_cast<std::uint8_t>( wparam ) );
					return 0;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					m_input.on_key_up( static_cast<std::uint8_t>( wparam ) );
					return 0;
				case WM_KILLFOCUS:
					m_input.clear();
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
		input m_input;
		bool m_done = false;
	};
}
