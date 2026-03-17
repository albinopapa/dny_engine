#pragma once

#include "gamepad.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"

#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

namespace dny{
	class Input{
	public:
		using Binding = std::variant<Key, MouseButton, GamepadButton>;

		Input( Keyboard& keyboard, Mouse& mouse, Gamepad& gamepad ) noexcept;
		Input( Input const& ) = delete;
		Input( Input&& ) = default;

		Input& operator=( Input const& ) = delete;
		Input& operator=( Input&& ) = default;

		void bind( std::string action, Key key );
		void bind( std::string action, MouseButton button );
		void bind( std::string action, GamepadButton button );
		bool has_binding( std::string_view action ) const;

		bool is_pressed( std::string_view action ) const;
		bool is_held( std::string_view action ) const;
		bool is_released( std::string_view action ) const;

		bool is_key_down( std::uint8_t key_code ) const noexcept;

		Mouse const& mouse() const noexcept;
		Keyboard const& keyboard()const noexcept;
		Keyboard& keyboard()noexcept;
		Gamepad const& gamepad() const noexcept;
	private:
		template<typename Query>
		bool query_binding( std::string_view action, Query&& query ) const{
			const auto iter = m_bindings.find( std::string{ action } );
			if( iter == m_bindings.end() ){
				return false;
			}
			return std::visit( std::forward<Query>( query ), iter->second );
		}

		Keyboard* m_keyboard = nullptr;
		Mouse const* m_mouse = nullptr;
		Gamepad const* m_gamepad = nullptr;
		std::unordered_map<std::string, Binding, std::hash<std::string>, std::equal_to<>> m_bindings;
	};

	using input = Input;
}
