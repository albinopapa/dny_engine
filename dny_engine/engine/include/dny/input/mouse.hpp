#pragma once

#include "math/math.hpp"

#include <array>
#include <cstdint>

namespace dny{
	enum class MouseButton : std::uint8_t{
		Left,
		Right,
		Middle,
		Count
	};

	class Mouse{
	public:
		void begin_frame() noexcept;
		bool handle_message( std::uint32_t msg, std::uintptr_t wparam, std::intptr_t lparam ) noexcept;
		void clear() noexcept;

		dny::vector2<std::int32_t> position() const noexcept;
		dny::vector2<std::int32_t> delta() const noexcept;
		std::int32_t wheel_delta() const noexcept;

		bool is_pressed( MouseButton button ) const noexcept;
		bool is_held( MouseButton button ) const noexcept;
		bool is_released( MouseButton button ) const noexcept;
	private:
		static std::size_t to_index( MouseButton button ) noexcept;

		dny::vector2<std::int32_t> m_position = { 0, 0 };
		dny::vector2<std::int32_t> m_delta = { 0, 0 };
		std::int32_t m_wheel_delta = 0;
		std::array<bool, static_cast<std::size_t>( MouseButton::Count )> m_current = {};
		std::array<bool, static_cast<std::size_t>( MouseButton::Count )> m_previous = {};
	};
}
