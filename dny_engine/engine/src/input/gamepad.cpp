#include "input/gamepad.hpp"
#include "platform/win32sdk.hpp"

#include <Xinput.h>

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
			m_current = to_state( state );
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
		const auto current = ( m_current.button_mask & mask ) != 0;
		const auto previous = m_prev_connected && ( m_previous.button_mask & mask ) != 0;
		return current && !previous;
	}

	bool Gamepad::is_held( GamepadButton button ) const noexcept{
		if( !m_connected ){
			return false;
		}
		return ( m_current.button_mask & static_cast< WORD >( button ) ) != 0;
	}

	bool Gamepad::is_released( GamepadButton button ) const noexcept{
		const auto mask = static_cast< WORD >( button );
		const auto current = m_connected && ( m_current.button_mask & mask ) != 0;
		const auto previous = m_prev_connected && ( m_previous.button_mask & mask ) != 0;
		return !current && previous;
	}

	float Gamepad::left_trigger() const noexcept{ return normalize_trigger( m_current.left_trigger_value ); }
	float Gamepad::right_trigger() const noexcept{ return normalize_trigger( m_current.right_trigger_value ); }
	float Gamepad::left_stick_x() const noexcept{ return normalize_stick( m_current.left_stick_X ); }
	float Gamepad::left_stick_y() const noexcept{ return normalize_stick( m_current.left_stick_Y ); }
	float Gamepad::right_stick_x() const noexcept{ return normalize_stick( m_current.right_stick_X ); }
	float Gamepad::right_stick_y() const noexcept{ return normalize_stick( m_current.right_stick_Y ); }

	float Gamepad::normalize_trigger( std::uint8_t value ) noexcept{
		return static_cast< float >( value ) / 255.0f;
	}

	float Gamepad::normalize_stick( std::int16_t value ) noexcept{
		if( value >= 0 ){
			return std::clamp( static_cast< float >( value ) / 32767.0f, 0.0f, 1.0f );
		}
		return std::clamp( static_cast< float >( value ) / 32768.0f, -1.0f, 0.0f );
	}
	Gamepad::state Gamepad::to_state( XINPUT_STATE const& xinput_state ) noexcept{
		return state{
			.button_mask         = xinput_state.Gamepad.wButtons,
			.left_trigger_value  = xinput_state.Gamepad.bLeftTrigger,
			.right_trigger_value = xinput_state.Gamepad.bRightTrigger,
			.left_stick_X        = xinput_state.Gamepad.sThumbLX,
			.left_stick_Y        = xinput_state.Gamepad.sThumbLY,
			.right_stick_X       = xinput_state.Gamepad.sThumbRX,
			.right_stick_Y       = xinput_state.Gamepad.sThumbRY
		};
	}
}
