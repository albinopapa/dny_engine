#include "input/keyboard.hpp"
#include "platform/win32sdk.hpp"

namespace dny{
	void Keyboard::begin_frame() noexcept{
		m_pressed.fill( false );
		m_released.fill( false );
	}

	bool Keyboard::handle_message( std::uint32_t msg, std::uintptr_t wparam ) noexcept{
		switch( msg ){
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				const auto idx = static_cast< std::uint8_t >( wparam );
				if( !m_held[ idx ] ){
					m_held[ idx ] = true;
					m_pressed[ idx ] = true;
				}
				return true;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				const auto idx = static_cast< std::uint8_t >( wparam );
				if( m_held[ idx ] ){
					m_held[ idx ] = false;
					m_released[ idx ] = true;
				}
				return true;
			}
			case WM_CHAR:
				push_char( static_cast< char >( wparam ) );
				return true;
			default:
				return false;
		}
	}

	void Keyboard::clear() noexcept{
		m_held.fill( false );
		m_pressed.fill( false );
		m_released.fill( false );

		m_char_head = 0;
		m_char_tail = 0;
		m_char_size = 0;
	}

	bool Keyboard::is_pressed( Key key ) const noexcept{
		return m_pressed[ key_to_win32_key_code( key ) ];
	}

	bool Keyboard::is_held( Key key ) const noexcept{
		return m_held[ key_to_win32_key_code( key ) ];
	}

	bool Keyboard::is_released( Key key ) const noexcept{
		return m_released[ key_to_win32_key_code( key ) ];
	}

	bool Keyboard::has_char() const noexcept{
		return m_char_size > 0;
	}

	char Keyboard::pop_char() noexcept{
		if( m_char_size == 0 ){
			return '\0';
		}

		const auto ch = m_char_queue[ m_char_head ];
		m_char_head = ( m_char_head + 1 ) % char_queue_capacity;
		--m_char_size;
		return ch;
	}

	void Keyboard::push_char( char value ) noexcept{
		if( m_char_size == char_queue_capacity ){
			m_char_head = ( m_char_head + 1 ) % char_queue_capacity;
			--m_char_size;
		}

		m_char_queue[ m_char_tail ] = value;
		m_char_tail = ( m_char_tail + 1 ) % char_queue_capacity;
		++m_char_size;
	}

	std::uint32_t Keyboard::key_to_win32_key_code( Key key ) const noexcept{
		switch( key ){
			case Key::Space: return VK_SPACE;
			case Key::_1: return '1';
			case Key::_2: return '2';
			case Key::_3: return '3';
			case Key::_4: return '4';
			case Key::_5: return '5';
			case Key::_6: return '6';
			case Key::_7: return '7';
			case Key::_8: return '8';
			case Key::_9: return '9';
			case Key::_0: return '0';
			case Key::A: return 'A';
			case Key::B: return 'B';
			case Key::C: return 'C';
			case Key::D: return 'D';
			case Key::E: return 'E';
			case Key::Q: return 'Q';
			case Key::Escape: return VK_ESCAPE;
			case Key::Left: return VK_LEFT;
			case Key::Right: return VK_RIGHT;
			case Key::Up: return VK_UP;
			case Key::Down: return VK_DOWN;
			case Key::F1: return VK_F1;
			case Key::F2: return VK_F2;
			case Key::F3: return VK_F3;
			case Key::F4: return VK_F4;
			case Key::Control: return VK_CONTROL;
			case Key::Shift: return VK_SHIFT;
			case Key::Enter: return VK_RETURN;

			default: return 0;
		}
	}

	Key Keyboard::win32_key_code_to_key( std::uint32_t key_code ) const noexcept{
		/*
		Space,
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		_0,
		A,
		B,
		C,
		D,
		E,
		Q,
		Escape,
		Left,
		Right,
		Up,
		Down,
		F1,
		F2,
		F3,
		F4,
		Control,
		Shift,
		Enter		*/
		switch( key_code ){
			case VK_RETURN: return Key::Enter;
			case '1': return Key::_1;
			case '2': return Key::_2;
			case '3': return Key::_3;
			case '4': return Key::_4;
			case '5': return Key::_5;
			case '6': return Key::_6;
			case '7': return Key::_7;
			case '8': return Key::_8;
			case '9': return Key::_9;
			case '0': return Key::_0;
			case VK_SPACE: return Key::Space;
			case 'A': return Key::A;
			case 'B': return Key::B;
			case 'C': return Key::C;
			case 'D': return Key::D;
			case 'E': return Key::E;
			case 'Q': return Key::Q;
			case VK_ESCAPE: return Key::Escape;
			case VK_LEFT: return Key::Left;
			case VK_RIGHT: return Key::Right;
			case VK_UP: return Key::Up;
			case VK_DOWN: return Key::Down;
			default: return static_cast< Key >( 0 );
		}
	}
}
