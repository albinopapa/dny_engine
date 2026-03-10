#pragma once

#include <array>
#include <cstdint>

struct _XINPUT_STATE;
namespace dny{
	enum class GamepadButton : std::uint16_t{
		A,
		B,
		X,
		Y,
		LeftShoulder,
		RightShoulder,
		Back,
		Start,
		DpadUp,
		DpadDown,
		DpadLeft,
		DpadRight
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
		struct state{
			std::uint16_t button_mask;
			std::uint8_t left_trigger_value;
			std::uint8_t right_trigger_value;
			std::int16_t left_stick_X;
			std::int16_t left_stick_Y;
			std::int16_t right_stick_X;
			std::int16_t right_stick_Y;
		};
		static float normalize_trigger( std::uint8_t value ) noexcept;
		static float normalize_stick( std::int16_t value ) noexcept;
		static state to_state( _XINPUT_STATE const& state ) noexcept;
		std::uint32_t m_user_index = 0;
		bool m_connected = false;
		bool m_prev_connected = false;
		state m_current = {};
		state m_previous = {};
	};
}
