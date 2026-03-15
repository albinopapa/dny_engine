#pragma once

#include "input/input.hpp"

namespace dny
{
	class IMode{
	public:
		void update( Input const& input_ ){
			handle_keyboard( input_ );
			handle_mouse( input_ );
		}
		virtual void render()const = 0;

	protected:
		virtual void handle_mouse( Input const& input_ ) = 0;
		virtual void handle_keyboard( Input const& input_ ) = 0;
	};
}
