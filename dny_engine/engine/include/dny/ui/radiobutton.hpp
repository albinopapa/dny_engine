#pragma once

#include "element.hpp"

namespace dny::ui{
	class RadioButton final : public Element{
	public:
		RadioButton( std::string id, std::string group, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_group( std::string group );
		std::string_view group() const noexcept;
		void set_text( std::string text );
		std::string_view text() const noexcept;
		void set_selected( bool value ) noexcept;
		bool selected() const noexcept;
		bool was_selected_this_frame() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		std::string m_group;
		std::string m_text;
		bool m_selected = false;
		bool m_selected_this_frame = false;
	};
}
