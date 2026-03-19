#pragma once

#include "graphics/font.hpp"
#include "input/input.hpp"
#include "renderer/renderer2d.hpp"
#include "ui/panel.hpp"

namespace dny
{
	class basic_mode{
	public:
		enum class State{ Working, Done, };

	public:
		basic_mode() = default;
		basic_mode( std::string const& name_, std::string const& label_, Rect<std::int32_t> const& area_ ) noexcept
			:
			m_panel{ name_, label_, area_.top_left(), area_.size() }{}

		virtual void update( Mouse const& mouse_, Keyboard& keyboard_ ) = 0;
		virtual void render( renderer2d& renderer_ )const = 0;
		State state() const noexcept{ return m_state; }
		virtual ~basic_mode() = default;

	protected:
	protected:
		ui::Panel m_panel = { "base_panel", "", { 0, 0 }, { 0, 0 } };
		State m_state = State::Working;
	};
}
