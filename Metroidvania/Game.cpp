#include "Game.hpp"

Game::Game( dny::Rect<std::int32_t> const& viewport, dny::LevelDocument& document_, dny::Font const& font_ )
	: 
	m_viewport( viewport ), 
	m_document( document_ ),
	m_font( font_ ){}
void Game::update( dny::Input& input_, float dt ){
	handle_keyboard( input_.keyboard() );
	handle_mouse( input_.mouse() );
}

void Game::render( dny::renderer2d& renderer_ ) const{
	// TODO: This will eventually render the game world, but for now 
	// we can just display some placeholder text to show that the 
	// state transition worked.
	renderer_.draw_text( "Game State", { 10.f, 10.f }, m_font, dny::Colors::white );
	renderer_.draw_text( "Not yet implemented: Press Escape to return to menu", { 10.f, 30.f }, m_font, dny::Colors::white );
}

void Game::handle_keyboard( dny::Keyboard& keyboard ){
	if(keyboard.is_pressed(dny::Key::Escape)){
		m_request = dny::app_state_request::Menu;
	}
}

void Game::handle_mouse( dny::Mouse const& mouse ){}