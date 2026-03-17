#pragma once

#include "graphics/font.hpp"
#include "input/input.hpp"
#include "renderer/renderer2d.hpp"

namespace dny
{
	class IMode{
	public:
		virtual void update( Mouse const& mouse, Keyboard& keyboard ) = 0;
		virtual void render( renderer2d& renderer, dny::Font const& font )const = 0;
	};
}
