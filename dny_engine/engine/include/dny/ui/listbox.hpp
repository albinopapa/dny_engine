#pragma once

#include "element.hpp"

#include <vector>

namespace dny::ui{
	class ListBox final : public Element{
	public:
		ListBox( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_selected_item( std::string_view item ) noexcept;
		void set_items( std::vector<std::string> items );
		void add_item( std::string item );
		void clear_items();
		std::vector<std::string> const& items() const noexcept;
		void set_selected_index( std::int32_t index ) noexcept;
		std::int32_t selected_index() const noexcept;
		std::string_view selected_item() const noexcept;
		bool selection_changed() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		static constexpr std::int32_t item_height = 18;

		std::vector<std::string> m_items;
		std::int32_t m_selected = -1;
		bool m_selection_changed = false;
	};
}
