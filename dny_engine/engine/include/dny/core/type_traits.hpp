#pragma once

#include <array>
#include <type_traits>

namespace dny{
	template<typename T> struct is_color : std::false_type{};
	template<typename T> struct is_math_vector : std::false_type{};
	template<typename T> struct is_vertex :std::false_type{};

	template<typename Array> struct get_array_size{};

	template<typename T, std::size_t N> struct get_array_size<std::array<T, N>>{
		static constexpr std::size_t value = N;
	};

	template<typename Array>
	constexpr std::size_t get_array_size_v = get_array_size<Array>::value;
}