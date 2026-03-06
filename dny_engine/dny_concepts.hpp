#pragma once

#include "dny_type_traits.hpp"
#include "dny_colors.hpp"
#include "dny_vector2.hpp"

namespace dny{
	template<typename T> concept color_type = is_color<T>::value;
	template<color_type ColorT> class surface;

	template<typename T> concept vector_type = is_math_vector<T>::value;

	template<typename T> concept vertex_type = is_vertex<T>::value;

	template<typename VS>
	concept vertex_shader =
		requires( VS vs, typename VS::vertex_in vin, typename VS::vertex_out vout ){
			{ vs( vin ) } -> std::same_as<typename VS::vertex_out>;
	};

	template<typename PS>
	concept pixel_shader =
		requires( PS ps, typename PS::vertex_in vin, ColorF color ){
			{ ps( vin ) }->std::same_as<ColorF>;
	};

	template<typename S>
	concept sampler_type =
		requires( S s, surface<ColorF> const* t ){
			{ s.sample( vector2<float>{}, t ) }noexcept -> std::same_as<ColorF>;
	};

	template<typename T>
	concept math_scalar = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

}