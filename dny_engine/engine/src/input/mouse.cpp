#include "input/mouse.hpp"
#include "platform/win32sdk.hpp"

namespace dny{
	void Mouse::begin_frame() noexcept{
		m_previous = m_current;
		m_delta = { 0, 0 };
		m_wheel_delta = 0;
	}

	bool Mouse::handle_message( std::uint32_t msg, std::uintptr_t wparam, std::intptr_t lparam ) noexcept{
		switch( msg ){
			case WM_MOUSEMOVE:{
				const auto x = static_cast< std::int32_t >( GET_X_LPARAM( lparam ) );
				const auto y = static_cast< std::int32_t >( GET_Y_LPARAM( lparam ) );
				m_delta.x += ( x - m_position.x );
				m_delta.y += ( y - m_position.y );
				m_position = { x, y };
				return true;
			}
			case WM_LBUTTONDOWN:
				m_current[ to_index( MouseButton::Left ) ] = true;
				return true;
			case WM_LBUTTONUP:
				m_current[ to_index( MouseButton::Left ) ] = false;
				return true;
			case WM_RBUTTONDOWN:
				m_current[ to_index( MouseButton::Right ) ] = true;
				return true;
			case WM_RBUTTONUP:
				m_current[ to_index( MouseButton::Right ) ] = false;
				return true;
			case WM_MBUTTONDOWN:
				m_current[ to_index( MouseButton::Middle ) ] = true;
				return true;
			case WM_MBUTTONUP:
				m_current[ to_index( MouseButton::Middle ) ] = false;
				return true;
			case WM_MOUSEWHEEL:
				m_wheel_delta += GET_WHEEL_DELTA_WPARAM( wparam );
				return true;
			default:
				return false;
		}
	}

	void Mouse::clear() noexcept{
		m_current.fill( false );
		m_previous.fill( false );
		m_delta = { 0, 0 };
		m_wheel_delta = 0;
	}

	dny::vector2<std::int32_t> Mouse::position() const noexcept{
		return m_position / 2;
	}

	dny::vector2<std::int32_t> Mouse::delta() const noexcept{
		return m_delta;
	}

	std::int32_t Mouse::wheel_delta() const noexcept{
		return m_wheel_delta;
	}

	bool Mouse::is_pressed( MouseButton button ) const noexcept{
		const auto idx = to_index( button );
		return m_current[ idx ] && !m_previous[ idx ];
	}

	bool Mouse::is_held( MouseButton button ) const noexcept{
		return m_current[ to_index( button ) ];
	}

	bool Mouse::is_released( MouseButton button ) const noexcept{
		const auto idx = to_index( button );
		return !m_current[ idx ] && m_previous[ idx ];
	}

	std::size_t Mouse::to_index( MouseButton button ) noexcept{
		return static_cast< std::size_t >( button );
	}
}
