#pragma once

#include "editor/iapp_state.hpp"
#include "editor/level_document.hpp"
#include "graphics/font.hpp"
#include "input/input.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer2d.hpp"
#include "utilities/timer.hpp"

#include <memory>

struct MetroidvaniaAppRequest{
	dny::LevelDocument document;
};

struct MetroidvaniaEditorRequest{
	dny::LevelDocument document;
};

class Metroidvania {
public:
	Metroidvania();

	void run();

private:
	void begin_frame();
	void end_frame();
	void update();
	void render();
	void handle_transition_state();
	void transition_state( std::unique_ptr<dny::IAppState> );
private:
	static constexpr dny::dims2<std::uint32_t> screen_dims{ 640, 360 };
	dny::renderer2d m_renderer;
	dny::surface<dny::Color32> m_screen_buffer{ screen_dims.width, screen_dims.height };
	std::vector<float> m_depth_buffer{ screen_dims.width * screen_dims.height, 1.f };
	dny::Timer m_timer;
	dny::LevelDocument m_document;
	dny::platform m_platform{ "Metroidvania" };
	dny::Font m_consolas;		// Used for editor UI and debug text.
	dny::Font m_ariel;			// Used for in-game text.
	std::unique_ptr<dny::IAppState> m_app_state;
	std::unique_ptr<dny::IAppState> m_next_state;
};

