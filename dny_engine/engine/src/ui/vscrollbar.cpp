#include "ui/vscrollbar.hpp"

#include "graphics/colors.hpp"

namespace dny::ui{
	VScrollBar::VScrollBar( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ){}

	void VScrollBar::set_range( std::int32_t min_value, std::int32_t max_value ) noexcept{
		m_min_value = std::min( min_value, max_value );
		m_max_value = std::max( min_value, max_value );
		set_value( m_value );
	}

	void VScrollBar::set_value( std::int32_t value ) noexcept{
		m_value = std::clamp( value, m_min_value, m_max_value );
	}

	void VScrollBar::set_page_size( std::int32_t value ) noexcept{
		m_page_size = std::max( 1, value );
	}

	std::int32_t VScrollBar::min_value() const noexcept{ return m_min_value; }
	std::int32_t VScrollBar::max_value() const noexcept{ return m_max_value; }
	std::int32_t VScrollBar::value() const noexcept{ return m_value; }
	std::int32_t VScrollBar::page_size() const noexcept{ return m_page_size; }
	bool VScrollBar::value_changed() const noexcept{ return m_value_changed; }

	void VScrollBar::update( Mouse const& mouse, Keyboard& ){
		m_value_changed = false;
		if( !visible() || !enabled() ){
			return;
		}

		const auto old_value = m_value;
		if( contains( mouse.position() ) ){
			if( const auto delta = mouse.wheel_delta(); delta != 0 ){
				m_value -= ( delta > 0 ) ? 1 : -1;
			}

			if( mouse.is_pressed( MouseButton::Left ) ){
				if( dny::contains( up_button_rect(), mouse.position() ) ){
					--m_value;
				}
				else if( dny::contains( down_button_rect(), mouse.position() ) ){
					++m_value;
				}
			}
		}

		m_value = std::clamp( m_value, m_min_value, m_max_value );
		m_value_changed = old_value != m_value;
	}

	void VScrollBar::draw( dny::renderer2d& renderer, dny::Font const& ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		const auto track_rect = Rect<std::int32_t>{
			rect.left + rect.width() / 2 - 1,
			rect.top + 10,
			rect.left + rect.width() / 2 + 1,
			rect.bottom - 10
		};
		renderer.fill_rect( rect, Color32{ 32, 32, 32, 180 } );
		renderer.draw_rect( rect, Color32{ 90, 90, 90, 255 } );
		renderer.fill_rect( track_rect, Color32{ 70, 70, 70, 255 } );

		const auto up_center = up_button_rect().center();
		const auto down_center = down_button_rect().center();
		const auto up_enabled = m_value > m_min_value;
		const auto down_enabled = m_value < m_max_value;
		const auto button_fill = Color32{ 220, 220, 220, 255 };
		const auto disabled_fill = Color32{ 96, 96, 96, 255 };
		const auto glyph_color = Color32{ 25, 25, 25, 255 };
		const auto disabled_glyph = Color32{ 60, 60, 60, 255 };

		renderer.fill_circle( up_center, 6, up_enabled ? button_fill : disabled_fill );
		renderer.draw_line(
			up_center + vector2<std::int32_t>{ -4, 2 },
			up_center + vector2<std::int32_t>{ 0, -3 },
			up_enabled ? glyph_color : disabled_glyph,
			1.f
		);
		renderer.draw_line(
			up_center + vector2<std::int32_t>{ 0, -3 },
			up_center + vector2<std::int32_t>{ 4, 2 },
			up_enabled ? glyph_color : disabled_glyph,
			1.f
		);

		renderer.fill_circle( down_center, 6, down_enabled ? button_fill : disabled_fill );
		renderer.draw_line(
			down_center + vector2<std::int32_t>{ -4, -2 },
			down_center + vector2<std::int32_t>{ 0, 3 },
			down_enabled ? glyph_color : disabled_glyph,
			1.f
		);
		renderer.draw_line(
			down_center + vector2<std::int32_t>{ 0, 3 },
			down_center + vector2<std::int32_t>{ 4, -2 },
			down_enabled ? glyph_color : disabled_glyph,
			1.f
		);

		if( m_max_value > m_min_value ){
			const auto thumb_area_top = rect.top + 20;
			const auto thumb_area_bottom = rect.bottom - 20;
			const auto thumb_range = std::max( 1, thumb_area_bottom - thumb_area_top - 8 );
			const auto normalized = static_cast<float>( m_value - m_min_value ) /
				static_cast<float>( m_max_value - m_min_value );
			const auto thumb_top = thumb_area_top + static_cast<std::int32_t>( normalized * thumb_range );
			const auto thumb_rect = Rect<std::int32_t>{
				rect.left + 3,
				thumb_top,
				rect.right - 3,
				thumb_top + 8
			};
			renderer.fill_rect( thumb_rect, Color32{ 180, 180, 180, 255 } );
			renderer.draw_rect( thumb_rect, Color32{ 25, 25, 25, 255 } );
		}
	}

	Rect<std::int32_t> VScrollBar::up_button_rect() const noexcept{
		const auto rect = bounds();
		return Rect<std::int32_t>{
			rect.left,
			rect.top,
			rect.right,
			rect.top + 20
		};
	}

	Rect<std::int32_t> VScrollBar::down_button_rect() const noexcept{
		const auto rect = bounds();
		return Rect<std::int32_t>{
			rect.left,
			rect.bottom - 20,
			rect.right,
			rect.bottom
		};
	}
}
