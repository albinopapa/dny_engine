#include "ui/radiobutton.hpp"
#include "core/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

namespace dny::ui{
	RadioButton::RadioButton( std::string id, std::string group, std::string label, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_group( std::move( group ) ), m_text( std::move( label ) ){}

	void RadioButton::set_group( std::string group ){
		m_group = std::move( group );
	}
	std::string_view RadioButton::group() const noexcept{ return m_group; }
	void RadioButton::set_text( std::string text ){ m_text = std::move( text ); }
	std::string_view RadioButton::text() const noexcept{ return m_text; }
	void RadioButton::set_selected( bool value ) noexcept{ m_selected = value; }
	bool RadioButton::selected() const noexcept{ return m_selected; }
	bool RadioButton::was_selected_this_frame() const noexcept{ return m_selected_this_frame; }

	void RadioButton::update( Mouse const& mouse, Keyboard& ){
		m_selected_this_frame = false;
		if( !visible() || !enabled() ){
			return;
		}
		if( mouse.is_pressed( MouseButton::Left ) && contains( mouse.position() ) ){
			m_selected = true;
			m_selected_this_frame = true;
		}
	}

	void RadioButton::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		const auto radius = rect.height() / 2;
		const auto center = dny::vector2<std::int32_t>{ rect.left + radius, rect.top + radius };
		dny::fill_circle( center, radius, dny::Color32{ 20, 20, 20 }, canvas );
		dny::fill_circle( center, radius - 1, dny::Color32{ 20, 20, 20 }, canvas );
		if( m_selected ){
			dny::fill_circle( center, radius / 2, dny::Color32{ dny::Colors::green }, canvas );
		}
		dny::draw_text( std::string{ m_text }, { rect.left + rect.height() + 6, rect.top + 2 }, font, dny::Color32{ dny::Colors::white }, canvas );
	}
}
