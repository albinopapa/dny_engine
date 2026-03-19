#include "Metroidvania.hpp"
#include "Game.hpp"
#include "Menu.hpp"

#include "editor/level_editor.hpp"

#include <span>

Metroidvania::Metroidvania()
	: 
	m_consolas( L"Consolas", 16 ),
	m_ariel( L"Ariel", 16 ),
	m_app_state( std::make_unique<dny::Menu>() )
{}

void Metroidvania::run() {
	while( !m_platform.is_done() ){
		begin_frame();
		update();
		render();
		end_frame();
	}
}

void Metroidvania::begin_frame(){
	m_platform.process_message_pump();

	auto screen_span = std::span<dny::Color32>{ m_screen_buffer.pixels(), screen_dims.width * screen_dims.height };
	std::fill( screen_span.begin(), screen_span.end(), dny::to_color32( dny::Colors::black ) );
	std::fill( m_depth_buffer.begin(), m_depth_buffer.end(), 1.f );
	m_renderer.begin( m_screen_buffer, m_depth_buffer );
}

void Metroidvania::end_frame(){
	auto viewspan = std::span<const dny::Color32>{ m_screen_buffer.pixels(), screen_dims.width * screen_dims.height };
	m_platform.update_view( screen_dims.width, screen_dims.height, viewspan );
}

void Metroidvania::update( ){
	const auto dt = m_timer.mark();
	m_app_state->update( m_platform.get_input(), dt);
	handle_transition_state();
}

void Metroidvania::render(){
	m_app_state->render( m_renderer );
}

void Metroidvania::handle_transition_state(){
	const auto request = m_app_state->request();
	switch( request ){
		case dny::app_state_request::Editor:
			m_app_state = std::make_unique<dny::LevelEditor>( dny::Rect<std::int32_t>{
				0, 0, screen_dims.width, screen_dims.height
			}, m_document, m_consolas );
			break;
		case dny::app_state_request::Game:
			m_app_state = std::make_unique<Game>( dny::Rect<std::int32_t>{
				0, 0, screen_dims.width, screen_dims.height
			}, m_document, m_ariel );
			break;
		case dny::app_state_request::Menu:
			m_app_state = std::make_unique<dny::Menu>();
			break;
		case dny::app_state_request::Exit:
			m_platform.shutdown();
			break;
	}
}
