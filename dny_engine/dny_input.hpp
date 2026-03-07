#pragma once

#include "dny_vector2.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dny{
	enum class mouse_button : std::uint8_t{
		left = 0,
		right,
		middle,
		x1,
		x2,
		count
	};

	class keyboard{
	public:
		static constexpr std::size_t max_keys = 256;

		void begin_frame() noexcept{
			m_previous = m_current;
		}
		void clear() noexcept{
			m_current.fill( false );
			m_previous.fill( false );
		}

		void on_key_down( std::uint32_t virtual_key_code ) noexcept{
			if( virtual_key_code < max_keys ){
				m_current[ virtual_key_code ] = true;
			}
		}
		void on_key_up( std::uint32_t virtual_key_code ) noexcept{
			if( virtual_key_code < max_keys ){
				m_current[ virtual_key_code ] = false;
			}
		}

		bool is_down( std::uint32_t virtual_key_code )const noexcept{
			return virtual_key_code < max_keys ? m_current[ virtual_key_code ] : false;
		}
		bool was_pressed( std::uint32_t virtual_key_code )const noexcept{
			if( virtual_key_code >= max_keys ){
				return false;
			}

			return m_current[ virtual_key_code ] && !m_previous[ virtual_key_code ];
		}
		bool was_released( std::uint32_t virtual_key_code )const noexcept{
			if( virtual_key_code >= max_keys ){
				return false;
			}

			return !m_current[ virtual_key_code ] && m_previous[ virtual_key_code ];
		}

	private:
		std::array<bool, max_keys> m_current = {};
		std::array<bool, max_keys> m_previous = {};
	};

	class mouse{
	public:
		static constexpr std::size_t button_count = static_cast< std::size_t >( mouse_button::count );

		void begin_frame() noexcept{
			m_previous_buttons = m_current_buttons;
			m_delta = { 0.f, 0.f };
			m_wheel_delta = 0.f;
		}
		void clear() noexcept{
			m_current_buttons.fill( false );
			m_previous_buttons.fill( false );
			m_position = { 0.f, 0.f };
			m_delta = { 0.f, 0.f };
			m_wheel_delta = 0.f;
		}

		void on_move( std::int32_t x_, std::int32_t y_ ) noexcept{
			auto next_pos = dny::vector2<float>{ static_cast<float>( x_ ), static_cast<float>( y_ ) };
			m_delta = next_pos - m_position;
			m_position = next_pos;
		}
		void on_button_down( mouse_button button_ ) noexcept{
			m_current_buttons[ to_index( button_ ) ] = true;
		}
		void on_button_up( mouse_button button_ ) noexcept{
			m_current_buttons[ to_index( button_ ) ] = false;
		}
		void on_wheel( float delta_ ) noexcept{
			m_wheel_delta += delta_;
		}

		dny::vector2<float> const& position()const noexcept{ return m_position; }
		dny::vector2<float> const& delta()const noexcept{ return m_delta; }
		float wheel_delta()const noexcept{ return m_wheel_delta; }

		bool is_down( mouse_button button_ )const noexcept{
			return m_current_buttons[ to_index( button_ ) ];
		}
		bool was_pressed( mouse_button button_ )const noexcept{
			auto idx = to_index( button_ );
			return m_current_buttons[ idx ] && !m_previous_buttons[ idx ];
		}
		bool was_released( mouse_button button_ )const noexcept{
			auto idx = to_index( button_ );
			return !m_current_buttons[ idx ] && m_previous_buttons[ idx ];
		}

	private:
		static constexpr std::size_t to_index( mouse_button button_ ) noexcept{
			return static_cast< std::size_t >( button_ );
		}

		std::array<bool, button_count> m_current_buttons = {};
		std::array<bool, button_count> m_previous_buttons = {};
		dny::vector2<float> m_position = {};
		dny::vector2<float> m_delta = {};
		float m_wheel_delta = 0.f;
	};

	class editor_input{
	public:
		editor_input( keyboard const& keyboard_, mouse const& mouse_ )
			:
			m_keyboard( keyboard_ ),
			m_mouse( mouse_ ) {
		}

		keyboard const& get_keyboard()const noexcept{ return m_keyboard; }
		mouse const& get_mouse()const noexcept{ return m_mouse; }

	private:
		keyboard const& m_keyboard;
		mouse const& m_mouse;
	};

	class Input{
	public:
		struct binding{
			enum class kind : std::uint8_t{
				keyboard,
				mouse
			};

			kind type = kind::keyboard;
			std::uint32_t code = 0;

			static binding key( std::uint32_t virtual_key_code ) noexcept{
				return { kind::keyboard, virtual_key_code };
			}
			static binding mouse_btn( mouse_button button_ ) noexcept{
				return { kind::mouse, static_cast< std::uint32_t >( button_ ) };
			}
		};

		Input( keyboard const& keyboard_, mouse const& mouse_ )
			:
			m_keyboard( keyboard_ ),
			m_mouse( mouse_ ) {
		}

		void bind_action( std::string action_name_, binding binding_ ){
			auto& bindings = m_action_bindings[ std::move( action_name_ ) ];
			auto duplicate = std::find_if( bindings.begin(), bindings.end(), [ & ]( auto const& existing_ ){
				return existing_.type == binding_.type && existing_.code == binding_.code;
			} );
			if( duplicate == bindings.end() ){
				bindings.push_back( binding_ );
			}
		}

		void clear_action_bindings( std::string_view action_name_ ){
			m_action_bindings.erase( std::string{ action_name_ } );
		}

		bool is_action_down( std::string_view action_name_ )const noexcept{
			return any_binding_matches( action_name_, [ & ]( binding const& binding_ ){
				if( binding_.type == binding::kind::keyboard ){
					return m_keyboard.is_down( binding_.code );
				}

				return m_mouse.is_down( static_cast< mouse_button >( binding_.code ) );
			} );
		}
		bool was_action_pressed( std::string_view action_name_ )const noexcept{
			return any_binding_matches( action_name_, [ & ]( binding const& binding_ ){
				if( binding_.type == binding::kind::keyboard ){
					return m_keyboard.was_pressed( binding_.code );
				}

				return m_mouse.was_pressed( static_cast< mouse_button >( binding_.code ) );
			} );
		}
		bool was_action_released( std::string_view action_name_ )const noexcept{
			return any_binding_matches( action_name_, [ & ]( binding const& binding_ ){
				if( binding_.type == binding::kind::keyboard ){
					return m_keyboard.was_released( binding_.code );
				}

				return m_mouse.was_released( static_cast< mouse_button >( binding_.code ) );
			} );
		}

	private:
		template<typename Predicate>
		bool any_binding_matches( std::string_view action_name_, Predicate&& predicate_ )const noexcept{
			auto action = m_action_bindings.find( std::string{ action_name_ } );
			if( action == m_action_bindings.end() ){
				return false;
			}

			for( auto const& binding : action->second ){
				if( predicate_( binding ) ){
					return true;
				}
			}

			return false;
		}

		keyboard const& m_keyboard;
		mouse const& m_mouse;
		std::unordered_map<std::string, std::vector<binding>> m_action_bindings;
	};
}
