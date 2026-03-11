#include "game.hpp"

Game::Game( dny::platform& platform_ )
	:
	platform( platform_ ){
	platform.set_title( L"DNY Engine - Software Renderer" );
	auto& input = platform.get_input();
	input.bind( "move_left", dny::Key::A );
	input.bind( "move_right", dny::Key::D );
	input.bind( "jump", dny::Key::Space );
	input.bind( "zoom_in", dny::Key::Q );
	input.bind( "zoom_out", dny::Key::E );
	input.bind( "fire", dny::MouseButton::Left );
	input.bind( "dash", dny::GamepadButton::A );
	init_textures();
	init_debug_vertices();
}

Game::~Game() = default;
