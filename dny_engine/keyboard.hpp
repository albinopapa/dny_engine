#pragma once

#include <array>
#include <cstdint>

namespace dny{
	enum class Key : std::uint16_t{
		Space,
		A,
		D,
		E,
		Q,
		Escape,
		Left,
		Right,
		Up,
		Down
	};

	class Keyboard{
	public:
		void begin_frame() noexcept;
		bool handle_message( std::uint32_t msg, std::uintptr_t wparam ) noexcept;
		void clear() noexcept;

		bool is_pressed( Key key ) const noexcept;
		bool is_held( Key key ) const noexcept;
		bool is_released( Key key ) const noexcept;
		bool has_char() const noexcept;
		char pop_char() noexcept;
		Key win32_key_code_to_key( std::uint32_t key ) const noexcept;
	private:
		std::uint32_t key_to_win32_key_code( Key key_code ) const noexcept;
		void push_char( char value ) noexcept;

		static constexpr std::size_t key_count = 256;
		static constexpr std::size_t char_queue_capacity = 64;

		std::array<bool, key_count> m_held = {};
		std::array<bool, key_count> m_pressed = {};
		std::array<bool, key_count> m_released = {};

		std::array<char, char_queue_capacity> m_char_queue = {};
		std::size_t m_char_head = 0;
		std::size_t m_char_tail = 0;
		std::size_t m_char_size = 0;
	};
}
