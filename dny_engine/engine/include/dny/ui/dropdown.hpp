#pragma once

#include "element.hpp"
#include "listbox.hpp"

#include "graphics/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>
#include <vector>

namespace dny::ui{
	class Dropdown final : public Element{
	public:
		Dropdown( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_items( std::vector<std::string> items );
		void add_item( std::string item );
		void clear_items();
		std::vector<std::string> const& items() const noexcept;

		void set_selected_index( std::int32_t index ) noexcept;
		std::int32_t selected_index() const noexcept;
		std::string_view selected_item() const noexcept;

		bool selection_changed() const noexcept;

		void set_open( bool value ) noexcept;
		bool open() const noexcept;

		void set_placeholder( std::string text );
		std::string_view placeholder() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;

		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		void sync_popup_layout() noexcept;

	private:
		static constexpr std::int32_t popup_visible_items = 5;
		static constexpr std::int32_t popup_item_height = 18;

		ListBox m_list_box;
		std::string m_placeholder = "<select>";
		bool m_open = false;
		bool m_selection_changed = false;
	};
}
