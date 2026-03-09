#include "gamepad.hpp"

#include <algorithm>
#include <cmath>

#pragma comment( lib, "xinput9_1_0.lib" )

namespace dny{
	Gamepad::Gamepad( std::uint32_t user_index ) noexcept
		: m_user_index( user_index ){
	}

	void Gamepad::begin_frame() noexcept{
		m_previous = m_current;
		m_prev_connected = m_connected;
	}

	void Gamepad::poll() noexcept{
		XINPUT_STATE state = {};
		const auto result = XInputGetState( m_user_index, &state );
		if( result == ERROR_SUCCESS ){
			m_connected = true;
			m_current = state;
		}
		else{
			m_connected = false;
			m_current = {};
		}
	}

	bool Gamepad::is_connected() const noexcept{
		return m_connected;
	}

	bool Gamepad::is_pressed( GamepadButton button ) const noexcept{
		if( !m_connected ){
			return false;
		}
		const auto mask = static_cast< WORD >( button );
		const auto current = ( m_current.Gamepad.wButtons & mask ) != 0;
		const auto previous = m_prev_connected && ( m_previous.Gamepad.wButtons & mask ) != 0;
		return current && !previous;
	}

	bool Gamepad::is_held( GamepadButton button ) const noexcept{
		if( !m_connected ){
			return false;
		}
		return ( m_current.Gamepad.wButtons & static_cast< WORD >( button ) ) != 0;
	}

	bool Gamepad::is_released( GamepadButton button ) const noexcept{
		const auto mask = static_cast< WORD >( button );
		const auto current = m_connected && ( m_current.Gamepad.wButtons & mask ) != 0;
		const auto previous = m_prev_connected && ( m_previous.Gamepad.wButtons & mask ) != 0;
		return !current && previous;
	}

	float Gamepad::left_trigger() const noexcept{ return normalize_trigger( m_current.Gamepad.bLeftTrigger ); }
	float Gamepad::right_trigger() const noexcept{ return normalize_trigger( m_current.Gamepad.bRightTrigger ); }
	float Gamepad::left_stick_x() const noexcept{ return normalize_stick( m_current.Gamepad.sThumbLX ); }
	float Gamepad::left_stick_y() const noexcept{ return normalize_stick( m_current.Gamepad.sThumbLY ); }
	float Gamepad::right_stick_x() const noexcept{ return normalize_stick( m_current.Gamepad.sThumbRX ); }
	float Gamepad::right_stick_y() const noexcept{ return normalize_stick( m_current.Gamepad.sThumbRY ); }

	float Gamepad::normalize_trigger( BYTE value ) noexcept{
		return static_cast< float >( value ) / 255.0f;
	}

	float Gamepad::normalize_stick( SHORT value ) noexcept{
		if( value >= 0 ){
			return std::clamp( static_cast< float >( value ) / 32767.0f, 0.0f, 1.0f );
		}
		return std::clamp( static_cast< float >( value ) / 32768.0f, -1.0f, 0.0f );
	}
}
