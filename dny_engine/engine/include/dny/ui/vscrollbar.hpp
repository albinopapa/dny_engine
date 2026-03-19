#pragma once

#include "element.hpp"

namespace dny::ui{
	class VScrollBar final : public Element{
	public:
		VScrollBar( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_range( std::int32_t min_value, std::int32_t max_value ) noexcept;
		void set_value( std::int32_t value ) noexcept;
		void set_page_size( std::int32_t value ) noexcept;

		std::int32_t min_value() const noexcept;
		std::int32_t max_value() const noexcept;
		std::int32_t value() const noexcept;
		std::int32_t page_size() const noexcept;
		bool value_changed() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		Rect<std::int32_t> up_button_rect() const noexcept;
		Rect<std::int32_t> down_button_rect() const noexcept;

	private:
		std::int32_t m_min_value = 0;
		std::int32_t m_max_value = 0;
		std::int32_t m_value = 0;
		std::int32_t m_page_size = 1;
		bool m_value_changed = false;
	};
}
