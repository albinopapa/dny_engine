#include "listbox.hpp"
#include <utility>

#include "../dny_colors.hpp"
#include "../dny_graphics.hpp"

namespace dny::ui{
	ListBox::ListBox( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ){}

	void ListBox::set_items( std::vector<std::string> items ){
		m_items = std::move( items );
		if( m_selected >= static_cast<std::int32_t>( m_items.size() ) ){
			m_selected = -1;
		}
	}

	void ListBox::add_item( std::string item ){
		m_items.emplace_back( std::move( item ) );
	}

	void ListBox::clear_items(){
		m_items.clear();
		m_selected = -1;
	}

	std::vector<std::string> const& ListBox::items() const noexcept{ return m_items; }
	void ListBox::set_selected_index( std::int32_t index ) noexcept{
		if( index >= 0 && index < static_cast<std::int32_t>( m_items.size() ) ){
			m_selected = index;
		}
		else{
			m_selected = -1;
		}
	}
	std::int32_t ListBox::selected_index() const noexcept{ return m_selected; }
	std::string_view ListBox::selected_item() const noexcept{
		if( m_selected < 0 || m_selected >= static_cast<std::int32_t>( m_items.size() ) ){
			return {};
		}
		return m_items[m_selected];
	}
	bool ListBox::selection_changed() const noexcept{ return m_selection_changed; }

	void ListBox::update( Mouse const& mouse, Keyboard& ){
		m_selection_changed = false;
		if( !visible() || !enabled() ){
			return;
		}
		if( !mouse.is_pressed( MouseButton::Left ) || !contains( mouse.position() ) ){
			return;
		}

		const auto rect = bounds();
		const auto rel_y = mouse.position().y - rect.top;
		const auto index = rel_y / item_height;
		if( index >= 0 && index < static_cast<std::int32_t>( m_items.size() ) ){
			m_selection_changed = ( m_selected != index );
			m_selected = index;
		}
	}

	void ListBox::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		dny::fill_rect( rect, dny::Color32{ 15, 15, 15 }, canvas );
		dny::draw_rect( rect, dny::Colors::white, canvas );

		for( std::int32_t i = 0; i < static_cast<std::int32_t>( m_items.size() ); ++i ){
			const auto item_rect = dny::Rect<std::int32_t>{
				rect.left + 1,
				rect.top + i * item_height,
				rect.right - 1,
				rect.top + ( i + 1 ) * item_height
			};
			if( item_rect.bottom > rect.bottom ){
				break;
			}
			if( i == m_selected ){
				dny::fill_rect( item_rect, dny::Color32{ 45, 70, 120 }, canvas );
			}
			dny::draw( m_items[i], { item_rect.left + 4, item_rect.top + 2 }, font, dny::Colors::white, canvas );
		}
	}
}
