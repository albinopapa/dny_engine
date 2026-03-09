#pragma once

#include "dny_win32sdk.hpp"

#include <array>
#include <cstdint>

namespace dny{
	enum class Key : std::uint16_t{
		Space = VK_SPACE,
		A = 'A',
		D = 'D',
		E = 'E',
		Q = 'Q',
		Escape = VK_ESCAPE,
		Left = VK_LEFT,
		Right = VK_RIGHT,
		Up = VK_UP,
		Down = VK_DOWN
	};

	class Keyboard{
	public:
		void begin_frame() noexcept;
		bool handle_message( UINT msg, WPARAM wparam ) noexcept;
		void clear() noexcept;

		bool is_pressed( Key key ) const noexcept;
		bool is_held( Key key ) const noexcept;
		bool is_released( Key key ) const noexcept;
		bool is_held( std::uint8_t key_code ) const noexcept;
	private:
		static constexpr std::size_t key_count = 256;
		static std::size_t to_index( Key key ) noexcept;

		std::array<bool, key_count> m_current = {};
		std::array<bool, key_count> m_previous = {};
	};
}
