#pragma once

#include <array>
#include <cstdint>

namespace dny{
	class input{
	public:
		void on_key_down( std::uint8_t key_code ) noexcept{
			m_key_states[ key_code ] = true;
		}
		void on_key_up( std::uint8_t key_code ) noexcept{
			m_key_states[ key_code ] = false;
		}
		void clear() noexcept{
			m_key_states.fill( false );
		}
		bool is_key_down( std::uint8_t key_code )const noexcept{
			return m_key_states[ key_code ];
		}
	private:
		std::array<bool, 256> m_key_states = {};
	};
}
