#include "checkbox.hpp"
#include <utility>

#include "../dny_colors.hpp"
#include "../dny_graphics.hpp"

namespace dny::ui{
	CheckBox::CheckBox( std::string id, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_text( std::move( label ) ){}

	void CheckBox::set_text( std::string text ){ m_text = std::move( text ); }
	std::string_view CheckBox::text() const noexcept{ return m_text; }
	void CheckBox::set_checked( bool value ) noexcept{ m_checked = value; }
	bool CheckBox::checked() const noexcept{ return m_checked; }
	bool CheckBox::was_toggled() const noexcept{ return m_toggled; }

	void CheckBox::update( Mouse const& mouse, Keyboard& ){
		m_toggled = false;
		if( !visible() || !enabled() ){
			return;
		}
		if( mouse.is_pressed( MouseButton::Left ) && contains( mouse.position() ) ){
			m_checked = !m_checked;
			m_toggled = true;
		}
	}

	void CheckBox::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		const auto box_rect = dny::Rect<std::int32_t>{
			rect.left,
			rect.top,
			rect.left + rect.height(),
			rect.bottom
		};
		dny::fill_rect( box_rect, dny::Color32{ 20, 20, 20 }, canvas );
		dny::draw_rect( box_rect, dny::Colors::white, canvas );
		if( m_checked ){
			dny::fill_rect( dny::Rect<std::int32_t>{ box_rect.left + 4, box_rect.top + 4, box_rect.right - 4, box_rect.bottom - 4 }, dny::Colors::green, canvas );
		}
		dny::draw( std::string{ m_text }, { box_rect.right + 6, rect.top + 2 }, font, dny::Colors::white, canvas );
	}
}
