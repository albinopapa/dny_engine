#include "element.hpp"
#include <utility>

namespace dny::ui{
	Element::Element( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: m_id( std::move( id ) ), m_position( position ), m_size( size ){}

	void Element::set_position( dny::vector2<std::int32_t> position ) noexcept{ m_position = position; }
	void Element::set_size( dny::dims2<std::int32_t> size ) noexcept{ m_size = size; }
	void Element::set_visible( bool value ) noexcept{ m_visible = value; }
	void Element::set_enabled( bool value ) noexcept{ m_enabled = value; }

	dny::vector2<std::int32_t> Element::position() const noexcept{ return m_position; }
	dny::dims2<std::int32_t> Element::size() const noexcept{ return m_size; }
	bool Element::visible() const noexcept{ return m_visible; }
	bool Element::enabled() const noexcept{ return m_enabled; }
	std::string_view Element::id() const noexcept{ return m_id; }

	dny::Rect<std::int32_t> Element::bounds() const noexcept{
		return dny::Rect<std::int32_t>{ m_position, m_size };
	}

	bool Element::contains( dny::vector2<std::int32_t> point ) const noexcept{
		const auto rect = bounds();
		return point.x >= rect.left && point.x < rect.right && point.y >= rect.top && point.y < rect.bottom;
	}
}
