#pragma once

#include "editor/iapp_state.hpp"
#include "editor/level_document.hpp"

class Game : public dny::IAppState{
	public:
	Game( dny::Rect<std::int32_t> const& viewport, dny::LevelDocument& document_ );

	Game( Game const& ) = delete;
	Game& operator=( Game const& ) = delete;

	void update( dny::Input& input_, float dt )override;
	void render( dny::renderer2d& renderer_, dny::Font const& font_ ) const override;

private:
	void handle_keyboard( dny::Keyboard& keyboard );
	void handle_mouse( dny::Mouse const& mouse );
private:
	dny::Rect<std::int32_t> m_viewport;
	dny::LevelDocument& m_document;
	// Later, Player, Enemy, etc.
};