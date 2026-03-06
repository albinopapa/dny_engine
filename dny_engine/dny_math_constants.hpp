#pragma once

#include <bit>
#include <numbers>

namespace dny{
	static constexpr auto PI = std::numbers::pi_v<float>;
	static constexpr auto qnan = std::bit_cast< float >( -1 );
	static constexpr auto epsilon = 1e-6f;
	enum class handedness_t{ invalid, left, right };
}