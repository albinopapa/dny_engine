#pragma once

#include <string>
#include <system_error>

namespace dny{
#ifdef NDEBUG
	constexpr bool is_debug_build = false;
#else
	constexpr bool is_debug_build = true;
#endif

	enum class dny_Engine_Request{
		Menu,
		Editor,
		Game,
		Exit,
		None,
	};

	template<typename T>
	concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;


	// Function to convert std::string_view to std::wstring
	std::wstring string_to_wstring( std::string const& str )noexcept;

}
