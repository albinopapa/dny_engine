#include "ui/button.hpp"
#include "graphics/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

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

	void Button::draw( dny::renderer2d& renderer, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		static constexpr auto enabled_color = Color32{ 61, 61, 61 };
		static constexpr auto disabled_color = Color32{ 36, 36, 36 };
		const auto rect = bounds();
		const auto fill = enabled() ? enabled_color : disabled_color;
		renderer.fill_rect( rect, fill );
		renderer.draw_rect( rect, to_color32( dny::Colors::white ) );
		renderer.draw_text( m_text, { rect.left + 4, rect.top + 4 }, font, to_color32( dny::Colors::white ) );
	}
}
