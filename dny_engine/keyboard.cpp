#include "keyboard.hpp"

namespace dny{
	void Keyboard::begin_frame() noexcept{
		m_previous = m_current;
	}

	bool Keyboard::handle_message( UINT msg, WPARAM wparam ) noexcept{
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
		const auto idx = to_index( key );
		return m_current[ idx ] && !m_previous[ idx ];
	}

	bool Keyboard::is_held( Key key ) const noexcept{
		return m_current[ to_index( key ) ];
	}

	bool Keyboard::is_released( Key key ) const noexcept{
		const auto idx = to_index( key );
		return !m_current[ idx ] && m_previous[ idx ];
	}

	bool Keyboard::is_held( std::uint8_t key_code ) const noexcept{
		return m_current[ key_code ];
	}

	std::size_t Keyboard::to_index( Key key ) noexcept{
		return static_cast< std::size_t >( key );
	}
}
