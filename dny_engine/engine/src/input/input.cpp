#include "input/input.hpp"

namespace dny{
	Input::Input( Keyboard const& keyboard, Mouse const& mouse, Gamepad const& gamepad ) noexcept
		: m_keyboard( &keyboard ), m_mouse( &mouse ), m_gamepad( &gamepad ){
	}

	void Input::bind( std::string action, Key key ){
		m_bindings[ std::move( action ) ] = key;
	}

	void Input::bind( std::string action, MouseButton button ){
		m_bindings[ std::move( action ) ] = button;
	}

	void Input::bind( std::string action, GamepadButton button ){
		m_bindings[ std::move( action ) ] = button;
	}

	bool Input::has_binding( std::string_view action ) const{
		return m_bindings.contains( std::string{ action } );
	}

	bool Input::is_pressed( std::string_view action ) const{
		return query_binding( action, [this]( auto binding ){
			using T = decltype( binding );
			if constexpr( std::is_same_v<T, Key> ) 
				return m_keyboard->is_pressed( binding );
			if constexpr( std::is_same_v<T, MouseButton> ) 
				return m_mouse->is_pressed( binding );
			if constexpr( std::is_same_v<T, GamepadButton> ) 
				return m_gamepad->is_pressed( binding );
		} );
	}

	bool Input::is_held( std::string_view action ) const{
		return query_binding( action, [this]( auto binding ){
			using T = decltype( binding );
			if constexpr( std::is_same_v<T, Key> ) 
				return m_keyboard->is_held( binding );
			if constexpr( std::is_same_v<T, MouseButton> ) 
				return m_mouse->is_held( binding );
			if constexpr( std::is_same_v<T, GamepadButton> )
				return m_gamepad->is_held( binding );
		} );
	}

	bool Input::is_released( std::string_view action ) const{
		return query_binding( action, [this]( auto binding ){
			using T = decltype( binding );
			if constexpr( std::is_same_v<T, Key> ) 
				return m_keyboard->is_released( binding );
			if constexpr( std::is_same_v<T, MouseButton> ) 
				return m_mouse->is_released( binding );
			if constexpr( std::is_same_v<T, GamepadButton> )
				return m_gamepad->is_released( binding );
		} );
	}

	bool Input::is_key_down( std::uint8_t key_code ) const noexcept{
		return m_keyboard->is_held( m_keyboard->win32_key_code_to_key( key_code ) );
	}

	Mouse const& Input::mouse() const noexcept{
		return *m_mouse;
	}

	Keyboard const& Input::keyboard()const noexcept{
		return *m_keyboard;
	}

	Gamepad const& Input::gamepad() const noexcept{
		return *m_gamepad;
	}

}