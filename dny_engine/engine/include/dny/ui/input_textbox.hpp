#pragma once

#include "element.hpp"

#include <cstddef>

namespace dny::ui{
	class InputTextBox final : public Element{
	public:
		InputTextBox( std::string id, std::string text, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_text( std::string text );
		std::string_view text() const noexcept;
		void set_focused( bool value ) noexcept;
		bool focused() const noexcept;
		void set_max_length( std::size_t value ) noexcept;
		std::size_t max_length() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		std::string m_text;
		bool m_focused = false;
		std::size_t m_max_length = 64;
	};
}
