#pragma once

#include "math/simd.hpp"
#include "type_traits.hpp"

#include <array>
#include <tuple>

namespace dny{
	//template<typename Vertex, typename Arr, std::size_t I = 0>
	//void tuple_to_simd_array_helper( Vertex const& v, Arr& arr ){
	//	if constexpr( I < get_array_size_v<Arr> ){
	//		arr[ I ] = simd::load( std::get<I>( v.m_fields ) );
	//		tuple_to_simd_array_helper<Vertex, Arr, I + 1>( v, arr );
	//	}
	//}
	//template<typename Arr, typename Vertex, std::size_t I>
	//void simd_array_to_tuple_helper( Arr const& arr, Vertex& v ){
	//	if constexpr( I < get_array_size_v<Arr> ){
	//		simd::store( arr[ I ], std::get<I>( v.m_fields ) );
	//		simd_array_to_tuple_helper<Arr, Vertex, I + 1>( arr, v );
	//	}
	//}

	template<typename Vertex, typename Arr>
		requires ( std::tuple_size_v<decltype( Vertex::m_fields )> == get_array_size_v<Arr> )
	void tuple_to_simd_array( Vertex const& v, Arr& arr ){
		constexpr size_t N = std::tuple_size_v<decltype( Vertex::m_fields )>;
		[&] <std::size_t... I>( std::index_sequence<I...> ){
			( ( arr[ I ] = simd::load( std::get<I>( v.m_fields ) ) ), ... );
		}( std::make_index_sequence<N>{} );
	}

	template<typename Arr, typename Vertex>
		requires ( std::tuple_size_v<decltype( Vertex::m_fields )> == get_array_size_v<Arr> )
	void simd_array_to_tuple( Arr const& arr, Vertex& v ){
		constexpr size_t N = std::tuple_size_v<decltype( Vertex::m_fields )>;
		[&] <std::size_t... I>( std::index_sequence<I...> ){
			( ( simd::store( arr[ I ], std::get<I>( v.m_fields ) ) ), ... );
		}( std::make_index_sequence<N>{} );
	}
}