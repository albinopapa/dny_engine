#include "button.hpp"
#include <utility>

#include "../dny_colors.hpp"
#include "../dny_graphics.hpp"

namespace dny::ui{
	Button::Button( std::string id, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_text( std::move( label ) ){}

	void Button::set_text( std::string text ){
		m_text = std::move( text );
	}

	std::string_view Button::text() const noexcept{ return m_text; }
	bool Button::was_clicked() const noexcept{ return m_clicked; }

	void Button::update( Mouse const& mouse, Keyboard& ){
		m_clicked = false;
		if( !visible() || !enabled() ){
			return;
		}
		if( mouse.is_pressed( MouseButton::Left ) && contains( mouse.position() ) ){
			m_clicked = true;
		}
	}

	void Button::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		static constexpr auto white = dny::Color32{ dny::Colors::white };
		const auto rect = bounds();
		const auto fill = enabled() ? dny::Color32{ 60, 60, 60 } : dny::Color32{ 35, 35, 35 };
		dny::fill_rect( rect, fill, canvas );
		dny::draw_rect( rect, white, canvas );
		dny::draw_text( m_text, { rect.left + 4, rect.top + 4 }, font, white, canvas );
	}
}
