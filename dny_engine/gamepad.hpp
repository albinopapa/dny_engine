#pragma once

#include <Windows.h>
#include <Xinput.h>

#include <array>
#include <cstdint>

namespace dny{
	enum class GamepadButton : std::uint16_t{
		A = XINPUT_GAMEPAD_A,
		B = XINPUT_GAMEPAD_B,
		X = XINPUT_GAMEPAD_X,
		Y = XINPUT_GAMEPAD_Y,
		LeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
		RightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
		Back = XINPUT_GAMEPAD_BACK,
		Start = XINPUT_GAMEPAD_START,
		DpadUp = XINPUT_GAMEPAD_DPAD_UP,
		DpadDown = XINPUT_GAMEPAD_DPAD_DOWN,
		DpadLeft = XINPUT_GAMEPAD_DPAD_LEFT,
		DpadRight = XINPUT_GAMEPAD_DPAD_RIGHT
	};

	class Gamepad{
	public:
		explicit Gamepad( std::uint32_t user_index = 0 ) noexcept;

		void begin_frame() noexcept;
		void poll() noexcept;

		bool is_connected() const noexcept;
		bool is_pressed( GamepadButton button ) const noexcept;
		bool is_held( GamepadButton button ) const noexcept;
		bool is_released( GamepadButton button ) const noexcept;
		float left_trigger() const noexcept;
		float right_trigger() const noexcept;
		float left_stick_x() const noexcept;
		float left_stick_y() const noexcept;
		float right_stick_x() const noexcept;
		float right_stick_y() const noexcept;
	private:
		static float normalize_trigger( BYTE value ) noexcept;
		static float normalize_stick( SHORT value ) noexcept;

		std::uint32_t m_user_index = 0;
		bool m_connected = false;
		bool m_prev_connected = false;
		XINPUT_STATE m_current = {};
		XINPUT_STATE m_previous = {};
	};
}
