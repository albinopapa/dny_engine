#include "keyboard.hpp"
#include "dny_win32sdk.hpp"

namespace dny{
	void Keyboard::begin_frame() noexcept{
		m_pressed.fill( false );
		m_released.fill( false );
	}

	bool Keyboard::handle_message( std::uint32_t msg, std::uintptr_t wparam ) noexcept{
		switch( msg ){
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:{
				const auto idx = static_cast< std::uint8_t >( wparam );
				if( !m_held[ idx ] ){
					m_held[ idx ] = true;
					m_pressed[ idx ] = true;
				}
				return true;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:{
				const auto idx = static_cast< std::uint8_t >( wparam );
				if( m_held[ idx ] ){
					m_held[ idx ] = false;
					m_released[ idx ] = true;
				}
				return true;
			}
			case WM_CHAR:
				push_char( static_cast<char>( wparam ) );
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
			case Key::A: return 'A';
			case Key::D: return 'D';
			case Key::E: return 'E';
			case Key::Q: return 'Q';
			case Key::Escape: return VK_ESCAPE;
			case Key::Left: return VK_LEFT;
			case Key::Right: return VK_RIGHT;
			case Key::Up: return VK_UP;
			case Key::Down: return VK_DOWN;
			default: return 0;
		}
    }

	Key Keyboard::win32_key_code_to_key( std::uint32_t key_code ) const noexcept{
		switch( key_code ){
			case VK_SPACE: return Key::Space;
			case 'A': return Key::A;
			case 'D': return Key::D;
			case 'E': return Key::E;
			case 'Q': return Key::Q;
			case VK_ESCAPE: return Key::Escape;
			case VK_LEFT: return Key::Left;
			case VK_RIGHT: return Key::Right;
			case VK_UP: return Key::Up;
			case VK_DOWN: return Key::Down;
			default: return static_cast<Key>( 0 );
		}
	}
}
