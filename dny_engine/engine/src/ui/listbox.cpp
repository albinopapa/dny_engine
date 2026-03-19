#include "ui/listbox.hpp"
#include "graphics/graphics.hpp"

#include <algorithm>
#include <utility>

namespace dny::ui{
	ListBox::ListBox( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( id, position, size )
		, m_scroll_bar( id + "_scrollbar", position, dny::dims2<std::int32_t>{ scroll_bar_width, size.height } ){
		sync_scrollbar();
	}

	void ListBox::set_selected_item( std::string_view item ) noexcept{
		for( std::int32_t i = 0; i < static_cast<std::int32_t>( m_items.size() ); ++i ){
			if( m_items[i] == item ){
				m_selected = i;
				sync_scrollbar();
				return;
			}
		}
		m_selected = -1;
		sync_scrollbar();
	}

	void ListBox::set_items( std::vector<std::string> items ){
		m_items = std::move( items );
		if( m_selected >= static_cast<std::int32_t>( m_items.size() ) ){
			m_selected = -1;
		}
		sync_scrollbar();
	}

	void ListBox::add_item( std::string item ){
		m_items.emplace_back( std::move( item ) );
		sync_scrollbar();
	}

	void ListBox::clear_items(){
		m_items.clear();
		m_selected = -1;
		m_scroll_value = 0;
		sync_scrollbar();
	}

	std::vector<std::string> const& ListBox::items() const noexcept{ return m_items; }
	
	void ListBox::set_selected_index( std::int32_t index ) noexcept{
		if( index >= 0 && index < static_cast<std::int32_t>( m_items.size() ) ){
			m_selected = index;
		}
		else{
			m_selected = -1;
		}
		sync_scrollbar();
	}
	
	std::int32_t ListBox::selected_index() const noexcept{ return m_selected; }
	
	std::string_view ListBox::selected_item() const noexcept{
		if( m_selected < 0 || m_selected >= static_cast<std::int32_t>( m_items.size() ) ){
			return {};
		}
		return m_items[m_selected];
	}
	
	bool ListBox::selection_changed() const noexcept{ return m_selection_changed; }

	void ListBox::update( Mouse const& mouse, Keyboard& keyboard ){
		m_selection_changed = false;
		if( !visible() || !enabled() ){
			return;
		}

		sync_scrollbar();

		if( shows_scrollbar() ){
			m_scroll_bar.update( mouse, keyboard );
			if( m_scroll_bar.value_changed() ){
				m_scroll_value = m_scroll_bar.value();
			}
		}

		if( contains( mouse.position() ) && shows_scrollbar() ){
			const auto wheel_delta = mouse.wheel_delta();
			if( wheel_delta != 0 ){
				m_scroll_value += ( wheel_delta > 0 ) ? -1 : 1;
				m_scroll_value = std::max( 0, std::min( m_scroll_value, max_scroll_value() ) );
				m_scroll_bar.set_value( m_scroll_value );
			}
		}

		const auto item_area = item_area_bounds();
		const auto mouse_pos = mouse.position();
		const auto inside_item_area =
			mouse_pos.x >= item_area.left && mouse_pos.x <= item_area.right &&
			mouse_pos.y >= item_area.top && mouse_pos.y <= item_area.bottom;

		if( !mouse.is_pressed( MouseButton::Left ) || !inside_item_area ){
			return;
		}

		const auto rel_y = mouse.position().y - item_area.top;
		const auto index = ( rel_y / item_height ) + m_scroll_value;
		if( index >= 0 && index < static_cast<std::int32_t>( m_items.size() ) ){
			m_selection_changed = ( m_selected != index );
			m_selected = index;
		}
	}

	void ListBox::draw( dny::renderer2d& renderer, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		const auto item_area = item_area_bounds();
		renderer.fill_rect( rect, dny::Color32{ 15, 15, 15 } );
		renderer.draw_rect( rect, dny::Color32{ dny::Colors::white } );

		for( std::int32_t row = 0; row < visible_items; ++row ){
			const auto item_index = row + m_scroll_value;
			if( item_index >= static_cast<std::int32_t>( m_items.size() ) ){
				break;
			}

			const auto item_rect = dny::Rect<std::int32_t>{
				item_area.left + 1,
				item_area.top + row * item_height,
				item_area.right - 1,
				item_area.top + ( row + 1 ) * item_height
			};
			if( item_rect.bottom > item_area.bottom ){
				break;
			}

			if( item_index == m_selected ){
				renderer.fill_rect( item_rect, dny::Color32{ 45, 70, 120 } );
			}
			renderer.draw_text( m_items[item_index], { item_rect.left + 4, item_rect.top + 2 }, font, dny::Color32{ dny::Colors::white } );
		}

		if( shows_scrollbar() ){
			m_scroll_bar.draw( renderer, font );
		}
	}

	void ListBox::sync_scrollbar() noexcept{
		m_scroll_value = std::max( 0, std::min( m_scroll_value, max_scroll_value() ) );

		const auto rect = bounds();
		m_scroll_bar.set_position( { rect.right - scroll_bar_width, rect.top } );
		m_scroll_bar.set_size( { scroll_bar_width, rect.height() } );
		m_scroll_bar.set_visible( shows_scrollbar() );
		m_scroll_bar.set_enabled( enabled() );
		m_scroll_bar.set_page_size( visible_items );
		m_scroll_bar.set_range( 0, max_scroll_value() );
		m_scroll_bar.set_value( m_scroll_value );
	}

	std::int32_t ListBox::max_scroll_value() const noexcept{
		const auto overflow = static_cast<std::int32_t>( m_items.size() ) - visible_items;
		return std::max( 0, overflow );
	}

	bool ListBox::shows_scrollbar() const noexcept{
		return static_cast<std::int32_t>( m_items.size() ) > visible_items;
	}

	dny::Rect<std::int32_t> ListBox::item_area_bounds() const noexcept{
		const auto rect = bounds();
		if( !shows_scrollbar() ){
			return rect;
		}

		return dny::Rect<std::int32_t>{
			rect.left,
			rect.top,
			rect.right - scroll_bar_width,
			rect.bottom
		};
	}
}
