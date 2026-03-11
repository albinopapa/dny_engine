#include "ui/panel.hpp"
#include "core/colors.hpp"
#include "graphics/graphics.hpp"

#include <utility>

#include <unordered_map>

namespace dny::ui{
	Panel::Panel( std::string id, std::string title, dny::vector2<std::int32_t> position, dny::dims2<std::int32_t> size ) noexcept
		: Element( std::move( id ), position, size ), m_title( std::move( title ) ){}

	void Panel::set_title( std::string title ){
		m_title = std::move( title );
	}

	std::string_view Panel::title() const noexcept{ return m_title; }

	void Panel::add_child( std::shared_ptr<Element> child ){
		m_children.push_back( std::move( child ) );
	}

	Element* Panel::find_child( std::string_view id ) noexcept{
		for( auto& child : m_children ){
			if( child && child->id() == id ){
				return child.get();
			}
		}
		return nullptr;
	}

	Element const* Panel::find_child( std::string_view id ) const noexcept{
		for( auto const& child : m_children ){
			if( child && child->id() == id ){
				return child.get();
			}
		}
		return nullptr;
	}

	std::vector<std::shared_ptr<Element>> const& Panel::children() const noexcept{ return m_children; }

	void Panel::update( Mouse const& mouse, Keyboard& keyboard ){
		if( !visible() || !enabled() ){
			return;
		}

		for( auto& child : m_children ){
			if( child ){
				child->update( mouse, keyboard );
			}
		}
		resolve_radio_groups();
	}

	void Panel::draw( dny::surface<dny::Color32>& canvas, dny::Font const& font ) const{
		if( !visible() ){
			return;
		}

		const auto rect = bounds();
		dny::fill_rect( rect, dny::Color32{ 30, 30, 30 }, canvas );
		dny::draw_rect( rect, dny::Color32{ dny::Colors::gray }, canvas );
		dny::draw_text( std::string{ m_title }, { rect.left + 4, rect.top + 4 }, font, dny::Color32{ dny::Colors::white }, canvas );

		for( auto const& child : m_children ){
			if( child ){
				child->draw( canvas, font );
			}
		}
	}

	void Panel::resolve_radio_groups(){
		std::unordered_map<std::string_view, RadioButton*> last_clicked;
		for( auto& child : m_children ){
			auto* radio = dynamic_cast<RadioButton*>( child.get() );
			if( radio && radio->was_selected_this_frame() ){
				last_clicked[radio->group()] = radio;
			}
		}

		for( auto const& [ group, selected ] : last_clicked ){
			for( auto& child : m_children ){
				auto* radio = dynamic_cast<RadioButton*>( child.get() );
				if( radio && radio->group() == group ){
					radio->set_selected( radio == selected );
				}
			}
		}
	}
}
