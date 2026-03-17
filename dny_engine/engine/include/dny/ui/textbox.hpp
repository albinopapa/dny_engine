#pragma once

#include "element.hpp"

namespace dny::ui{
	class TextBox final : public Element{
	public:
		TextBox( std::string id, std::string text, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_text( std::string text );
		std::string_view text() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		std::string m_text;
	};
}
