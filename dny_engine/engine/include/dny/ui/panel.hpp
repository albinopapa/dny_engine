#pragma once

#include "element.hpp"
#include "radiobutton.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace dny::ui{
	class Panel final : public Element{
	public:
		Panel( std::string id, std::string title, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept;

		void set_title( std::string title );
		std::string_view title() const noexcept;

		void add_child( std::shared_ptr<Element> child );
		template<typename ElementT, typename... Args>
		std::shared_ptr<ElementT> emplace_child( Args&&... args ){
			auto instance = std::make_shared<ElementT>( std::forward<Args>( args )... );
			add_child( instance );
			return instance;
		}

		Element* find_child( std::string_view id ) noexcept;
		Element const* find_child( std::string_view id ) const noexcept;
		std::vector<std::shared_ptr<Element>> const& children() const noexcept;

		void update( Mouse const& mouse, Keyboard& keyboard ) override;
		void draw( dny::renderer2d& renderer, dny::Font const& font ) const override;

	private:
		void resolve_radio_groups();

		std::string m_title;
		std::vector<std::shared_ptr<Element>> m_children;
	};
}
