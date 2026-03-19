#include "ui/button.hpp"
#include "graphics/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

namespace dny::ui{
	Button::Button( std::string id, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_text( std::move( label ) ){}

    void Button::set_hovered( bool value ) noexcept{
		m_hovered = value;
	}

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
		if( contains( mouse.position() ) ){
			m_hovered = true;
			if( mouse.is_pressed( MouseButton::Left ) ){
				m_clicked = true;
			}
		}
		else{
			m_hovered = false;
		}
	}

	void Button::draw( dny::renderer2d& renderer, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		const auto str_dims = Font::measure_text( m_text, font );
		const auto text_pos = vector2<std::int32_t>{
			rect.left + ( rect.width() - str_dims.width ) / 2,
			rect.top + ( rect.height() - str_dims.height ) / 2
		};
		const auto fill = enabled() ? ( m_hovered ? hover_color : enabled_color ) : disabled_color;
		renderer.fill_rect( rect, fill );
		renderer.draw_rect( rect, to_color32( dny::Colors::white ) );
		renderer.draw_text( m_text, text_pos, font, to_color32( dny::Colors::white ) );
	}
}
