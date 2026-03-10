#pragma once

#include "element.hpp"

namespace dny::ui{
	class CheckBox final : public Element{
	public:
		CheckBox( std::string id, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_text( std::string text );
		std::string_view text() const noexcept;
		void set_checked( bool value ) noexcept;
		bool checked() const noexcept;
		bool was_toggled() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const override;

	private:
		std::string m_text;
		bool m_checked = false;
		bool m_toggled = false;
	};
}
