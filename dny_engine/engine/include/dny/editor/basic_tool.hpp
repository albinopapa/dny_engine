#pragma once

#include "input/input.hpp"

namespace dny{
	class basic_tool{
	public:
		virtual void update( Mouse const& mouse_, Keyboard& keyboard_ ) = 0;
		//virtual void render( renderer2d& renderer_ )const = 0;
		virtual ~basic_tool() = default;

	};

}