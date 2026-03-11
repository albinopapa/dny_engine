#include "ui/textbox.hpp"

#include "core/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

namespace dny::ui{
	TextBox::TextBox( std::string id, std::string text, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_text( std::move( text ) ){}

	void TextBox::set_text( std::string text ){
		m_text = std::move( text );
	}

	std::string_view TextBox::text() const noexcept{ return m_text; }

	void TextBox::update( Mouse const&, Keyboard& ){}

	void TextBox::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		dny::fill_rect( rect, dny::Color32{ 20, 20, 20 }, canvas );
		dny::draw_rect( rect, dny::Color32{ dny::Colors::white }, canvas );
		dny::draw_text( m_text, { rect.left + 4, rect.top + 4 }, font, dny::Color32{ dny::Colors::white }, canvas );
	}
}
