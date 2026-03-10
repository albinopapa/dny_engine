#pragma once

#include "../dny_dims2.hpp"
#include "../dny_font.hpp"
#include "../dny_rectangle.hpp"
#include "../dny_surface.hpp"
#include "../dny_vector2.hpp"
#include "../keyboard.hpp"
#include "../mouse.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace dny::ui{
	class Element{
	public:
		Element( std::string id, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;
		virtual ~Element() = default;

		Element( Element const& ) = default;
		Element& operator=( Element const& ) = default;
		Element( Element&& ) noexcept = default;
		Element& operator=( Element&& ) noexcept = default;

		void set_position( dny::vector2<std::int32_t> position ) noexcept;
		void set_size( dny::dims2<std::int32_t> size ) noexcept;
		void set_visible( bool value ) noexcept;
		void set_enabled( bool value ) noexcept;

		dny::vector2<std::int32_t> position() const noexcept;
		dny::dims2<std::int32_t> size() const noexcept;
		bool visible() const noexcept;
		bool enabled() const noexcept;
		std::string_view id() const noexcept;
		dny::Rect<std::int32_t> bounds() const noexcept;

		bool contains( dny::vector2<std::int32_t> point ) const noexcept;

		virtual void update( Mouse const& mouse, Keyboard& keyboard ) = 0;
		virtual void draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const = 0;

	private:
		std::string m_id;
		dny::vector2<std::int32_t> m_position = {};
		dny::dims2<std::int32_t> m_size = {};
		bool m_visible = true;
		bool m_enabled = true;
	};
}
