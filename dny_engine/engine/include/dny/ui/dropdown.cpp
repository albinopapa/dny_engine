#include "dropdown.hpp"

namespace dny::ui{
	Dropdown::Dropdown( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( id, position, size )
		, m_list_box(
			id + "_listbox",
			{ position.x, position.y + size.height },
			{ size.width, popup_visible_items * popup_item_height }
		){
		m_list_box.set_visible( false );
		sync_popup_layout();
	}

	void Dropdown::set_items( std::vector<std::string> items ){
		m_list_box.set_items( std::move( items ) );
	}

	void Dropdown::add_item( std::string item ){
		m_list_box.add_item( std::move( item ) );
	}

	void Dropdown::clear_items(){
		m_list_box.clear_items();
		m_open = false;
	}

	std::vector<std::string> const& Dropdown::items() const noexcept{ return m_list_box.items(); }

	void Dropdown::set_selected_index( std::int32_t index ) noexcept{
		m_list_box.set_selected_index( index );
	}

	std::int32_t Dropdown::selected_index() const noexcept{ return m_list_box.selected_index(); }

	std::string_view Dropdown::selected_item() const noexcept{ return m_list_box.selected_item(); }

	bool Dropdown::selection_changed() const noexcept{ return m_selection_changed; }

	void Dropdown::set_open( bool value ) noexcept{
		m_open = value && enabled() && visible();
		m_list_box.set_visible( m_open );
	}

	bool Dropdown::open() const noexcept{ return m_open; }

	void Dropdown::set_placeholder( std::string text ){
		m_placeholder = std::move( text );
	}

	std::string_view Dropdown::placeholder() const noexcept{ return m_placeholder; }

	void Dropdown::update( Mouse const& mouse, Keyboard& keyboard ) {
		m_selection_changed = false;
		sync_popup_layout();

		if( !visible() || !enabled() ){
			m_open = false;
			m_list_box.set_visible( false );
			return;
		}

		if( mouse.is_pressed( MouseButton::Left ) && contains( mouse.position() ) ){
			m_open = !m_open;
		}

		m_list_box.set_visible( m_open );
		m_list_box.set_enabled( enabled() );
		if( m_open ){
			m_list_box.update( mouse, keyboard );
			if( m_list_box.selection_changed() ){
				m_selection_changed = true;
				m_open = false;
				m_list_box.set_visible( false );
			}

			if( mouse.is_pressed( MouseButton::Left ) &&
				!contains( mouse.position() ) &&
				!m_list_box.contains( mouse.position() ) ){
				m_open = false;
				m_list_box.set_visible( false );
			}
		}
	}

	void Dropdown::draw( dny::renderer2d& renderer, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		renderer.fill_rect( rect, enabled() ? dny::Color32{ 24, 24, 24 } : dny::Color32{ 16, 16, 16 } );
		renderer.draw_rect( rect, dny::Color32{ dny::Colors::white } );

		const auto display_text = selected_item().empty() ? placeholder() : selected_item();
		renderer.draw_text( display_text, { rect.left + 4, rect.top + 2 }, font, dny::Color32{ dny::Colors::white } );

		const auto center = vector2<std::int32_t>{ rect.right - 10, rect.top + rect.height() / 2 };
		renderer.draw_line( center + vector2<std::int32_t>{ -4, -2 }, center + vector2<std::int32_t>{ 0, 2 }, dny::Color32{ dny::Colors::white }, 1.f );
		renderer.draw_line( center + vector2<std::int32_t>{ 0, 2 }, center + vector2<std::int32_t>{ 4, -2 }, dny::Color32{ dny::Colors::white }, 1.f );

		if( m_open ){
			m_list_box.draw( renderer, font );
		}
	}

	void Dropdown::sync_popup_layout() noexcept{
		const auto rect = bounds();
		m_list_box.set_position( { rect.left, rect.bottom } );
		m_list_box.set_size( { rect.width(), popup_visible_items * popup_item_height } );
	}

}