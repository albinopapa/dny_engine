#include "ui/checkbox.hpp"
#include "graphics/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

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

	void CheckBox::draw( dny::renderer2d& renderer, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();

		static constexpr auto white = dny::Color32{ dny::Colors::white };
		static constexpr auto green = dny::Color32{ dny::Colors::green };
		static constexpr auto background = dny::Color32{ 20, 20, 20 };
		renderer.fill_rect( rect, background );
		renderer.draw_rect( rect, white );
		if( m_checked ){
			const auto expanded_rect = dny::Rect<std::int32_t>{
				rect.left + 4,
				rect.top + 4,
				rect.right - 4,
				rect.bottom - 4 
			};
			renderer.fill_rect( expanded_rect, green );
		}
		renderer.draw_text( std::string{ m_text }, { rect.right + 6, rect.top + 2 }, font, white );
	}
}
