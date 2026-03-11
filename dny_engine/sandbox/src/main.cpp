#include "game.hpp"

#include "dny_platform.hpp"

#include <cstdint>

std::int32_t main() {
	auto platform = dny::platform{};
	auto game = Game{ platform };
	
	while( !platform.is_done() ){
		platform.process_message_pump();
		game.run();
	}

	return 0;
}
