#pragma once

#include "element.hpp"

namespace dny::ui{
	class Button final : public Element{
	public:
		Button( std::string id, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_text( std::string text );
		std::string_view text() const noexcept;
		bool was_clicked() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		std::string m_text;
		bool m_clicked = false;
	};
}
