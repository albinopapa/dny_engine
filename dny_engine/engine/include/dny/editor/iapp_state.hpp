#pragma once

#include "input/input.hpp"
#include "renderer/renderer2d.hpp"

namespace dny
{
	enum class app_state_request{
		Menu,
		Editor,
		Game,
		Exit,
		None,
	};

	// RequestT must be an 'enum class'
	class IAppState{
	public:
		virtual ~IAppState() = default;

		virtual void update( Input& input_, float dt ) = 0;
		virtual void render( renderer2d& renderer_ )const = 0;

		app_state_request request()const{ return m_request; }

	protected:
		app_state_request m_request = app_state_request::None;
	};
}
