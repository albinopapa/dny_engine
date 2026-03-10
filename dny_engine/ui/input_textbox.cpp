#include "input_textbox.hpp"
#include <utility>

#include "../dny_colors.hpp"
#include "../dny_graphics.hpp"

namespace dny::ui{
	InputTextBox::InputTextBox( std::string id, std::string text, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_text( std::move( text ) ){}

	void InputTextBox::set_text( std::string text ){
		m_text = std::move( text );
	}

	std::string_view InputTextBox::text() const noexcept{ return m_text; }
	void InputTextBox::set_focused( bool value ) noexcept{ m_focused = value; }
	bool InputTextBox::focused() const noexcept{ return m_focused; }
	void InputTextBox::set_max_length( std::size_t value ) noexcept{ m_max_length = value; }
	std::size_t InputTextBox::max_length() const noexcept{ return m_max_length; }

	void InputTextBox::update( Mouse const& mouse, Keyboard& keyboard ){
		if( !visible() || !enabled() ){
			m_focused = false;
			return;
		}

		if( mouse.is_pressed( MouseButton::Left ) ){
			m_focused = contains( mouse.position() );
		}

		if( !m_focused ){
			return;
		}

		while( keyboard.has_char() ){
			const auto ch = keyboard.pop_char();
			if( ch == '\b' ){
				if( !m_text.empty() ){
					m_text.pop_back();
				}
				continue;
			}
			if( ch == '\r' || ch == '\n' ){
				continue;
			}
			if( m_text.size() < m_max_length ){
				m_text.push_back( ch );
			}
		}
	}

	void InputTextBox::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		dny::fill_rect( rect, dny::Color32{ 15, 15, 15 }, canvas );
		dny::draw_rect( rect, m_focused ? dny::Colors::yellow : dny::Colors::white, canvas );
		dny::draw( std::string{ m_text }, { rect.left + 4, rect.top + 4 }, font, dny::Colors::white, canvas );
	}
}
