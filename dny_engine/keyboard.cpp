#include "keyboard.hpp"
#include "dny_win32sdk.hpp"

namespace dny{
	void Keyboard::begin_frame() noexcept{
		m_previous = m_current;
	}

	bool Keyboard::handle_message( std::uint32_t msg, std::uintptr_t wparam ) noexcept{
		switch( msg ){
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				m_current[ static_cast< std::uint8_t >( wparam ) ] = true;
				return true;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				m_current[ static_cast< std::uint8_t >( wparam ) ] = false;
				return true;
			default:
				return false;
		}
	}

	void Keyboard::clear() noexcept{
		m_current.fill( false );
		m_previous.fill( false );
	}

	bool Keyboard::is_pressed( Key key ) const noexcept{
		const auto idx = key_to_win32_key_code( key );
		return m_current[ idx ] && !m_previous[ idx ];
	}

	bool Keyboard::is_held( Key key ) const noexcept{
		return m_current[ key_to_win32_key_code( key ) ];
	}

	bool Keyboard::is_released( Key key ) const noexcept{
		const auto idx = key_to_win32_key_code( key );
		return !m_current[ idx ] && m_previous[ idx ];
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
		switch( key_code){
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
