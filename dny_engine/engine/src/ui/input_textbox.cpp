#include "ui/input_textbox.hpp"

#include "graphics/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

namespace dny::ui{
	InputTextBox::InputTextBox( std::string id, std::string text, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_placeholder( std::move( text ) ){}

	void InputTextBox::set_text( std::string text ){
		m_text = std::move( text );
	}

	std::string_view InputTextBox::text() const noexcept{ return m_text; }
    void InputTextBox::set_placeholder( std::string text ) noexcept{ m_placeholder = std::move( text ); }
	std::string_view InputTextBox::placeholder() const noexcept{ return m_placeholder; }
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
			// Ignore control characters - we only want to allow printable characters 
			// in the text box.
			if( ch == '\r' || ch == '\n' || ch == '\t' ){
				continue;
			}
			if( m_text.size() < m_max_length ){
				m_text.push_back( ch );
			}
		}
	}

	void InputTextBox::draw( dny::renderer2d& renderer, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto& visible_text = m_text.empty() ? m_placeholder : m_text;
		const auto rect = bounds();
		renderer.fill_rect( rect, dny::Color32{ 16, 16, 16 } );
		renderer.draw_rect( rect, m_focused ? dny::Color32( dny::Colors::yellow ) : dny::Color32( dny::Colors::white ) );
		renderer.draw_text( visible_text, { rect.left + 4, rect.top + 4 }, font, dny::Color32{ dny::Colors::white } );
	}
}
