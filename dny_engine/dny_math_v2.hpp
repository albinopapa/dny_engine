#pragma once

#include "dny_simd.hpp"
#include "dny_math.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <intrin.h>
#include <format>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>


#define MAKE_KERNEL( Kernel ) (std::string_view{ #Kernel } )

namespace dny{
	namespace gpu{

		enum class backend_kind : std::uint8_t{ cpu, directcompute };

		enum class access_mode : std::uint8_t{
			read,
			write,
			read_write
		};

		// Forward-declare an internal implementation handle
		namespace detail{ struct memory_impl; }

		template<class T>
		class memory final{
			static_assert( std::is_trivially_copyable_v<T> );

		public:
			using value_type = T;
			using size_type = std::size_t;

			memory() noexcept = default;
			explicit memory( size_type count );

			memory( memory&& ) noexcept;
			memory& operator=( memory&& ) noexcept;

			memory( memory const& ) = delete;
			memory& operator=( memory const& ) = delete;

			~memory();

			[[nodiscard]] bool      valid() const noexcept;
			[[nodiscard]] size_type size()  const noexcept;
			[[nodiscard]] size_type bytes() const noexcept;

			// Explicit data movement
			void set( std::span<T const> src );  // host -> memory
			void get( std::span<T> dst ) const;  // memory -> host

			// Explicit domain sync
			void flush();        // host -> device (no-op on CPU)
			void synchronize();  // device -> host (no-op on CPU)

			[[nodiscard]] std::uint64_t debug_id() const noexcept;

		private:
			detail::memory_impl* m_impl = nullptr;
		};

	} // namespace gpu


	template<typename T> struct get_typename;
	template<> struct get_typename<float>{
		static constexpr std::string_view value = "float";
	};
	template<> struct get_typename<dny::vector2<float>>{
		inline static const std::string value = std::string{ "float2" };
	};
	template<> struct get_typename<dny::vector3<float>>{
		inline static const std::string value = std::string{ "float3" };
	};
	template<> struct get_typename<dny::vector4<float>>{
		inline static const std::string value = std::string{ "float4" };
	};
	template<> struct get_typename<dny::matrix_4x4<float>>{
		inline static const std::string value = std::string{ "float4x4" };
	}
} // namespace dny

namespace dny::gpu::kernels{

	void add( std::vector<float> const& lhs, std::vector<float> const& rhs, std::vector<float>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = std::addressof( lhs[ i ] );
			auto const* rhs_it = std::addressof( rhs[ i ] );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a + b;

			auto* out_it = std::addressof( result[ i ] );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] + rhs[ i ];
		}
	}
	void sub( std::vector<float> const& lhs, std::vector<float> const& rhs, std::vector<float>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = std::addressof( lhs[ i ] );
			auto const* rhs_it = std::addressof( rhs[ i ] );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a - b;

			auto* out_it = std::addressof( result[ i ] );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] - rhs[ i ];
		}
	}
	void mul( std::vector<float> const& lhs, std::vector<float> const& rhs, std::vector<float>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = std::addressof( lhs[ i ] );
			auto const* rhs_it = std::addressof( rhs[ i ] );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a * b;

			auto* out_it = std::addressof( result[ i ] );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] * rhs[ i ];
		}
	}
	void div( std::vector<float> const& lhs, std::vector<float> const& rhs, std::vector<float>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = std::addressof( lhs[ i ] );
			auto const* rhs_it = std::addressof( rhs[ i ] );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a / b;

			auto* out_it = std::addressof( result[ i ] );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] / rhs[ i ];
		}
	}

	void add( std::vector<dny::vector2<float>> const& lhs, std::vector<dny::vector2<float>> const& rhs, std::vector<dny::vector2<float>>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = reinterpret_cast<float const*>( std::addressof( lhs[ i ] ) );
			auto const* rhs_it = reinterpret_cast<float const*>( std::addressof( rhs[ i ] ) );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a + b;

			auto* out_it = reinterpret_cast< float* >( std::addressof( result[ i ] ) );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] + rhs[ i ];
		}
	}
	void sub( std::vector<dny::vector2<float>> const& lhs, std::vector<dny::vector2<float>> const& rhs, std::vector<dny::vector2<float>>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = reinterpret_cast< float const* >( std::addressof( lhs[ i ] ) );
			auto const* rhs_it = reinterpret_cast< float const* >( std::addressof( rhs[ i ] ) );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a - b;

			auto* out_it = reinterpret_cast< float* >( std::addressof( result[ i ] ) );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] - rhs[ i ];
		}
	}
	void mul( std::vector<dny::vector2<float>> const& lhs, std::vector<dny::vector2<float>> const& rhs, std::vector<dny::vector2<float>>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = reinterpret_cast< float const* >( std::addressof( lhs[ i ] ) );
			auto const* rhs_it = reinterpret_cast< float const* >( std::addressof( rhs[ i ] ) );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a * b;

			auto* out_it = reinterpret_cast< float* >( std::addressof( result[ i ] ) );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] * rhs[ i ];
		}
	}
	void div( std::vector<dny::vector2<float>> const& lhs, std::vector<dny::vector2<float>> const& rhs, std::vector<dny::vector2<float>>& result ){
		if( lhs.size() != rhs.size() )
			throw std::invalid_argument( "Input buffer sizes do not match." );

		if( result.size() != lhs.size() ){
			result.resize( lhs.size() );
		}

		static constexpr std::size_t bitmod = std::size_t( -3 );

		const std::size_t start = std::size_t{};
		const std::size_t end = lhs.size() & bitmod;
		const std::size_t slack_start = end;
		const std::size_t slack_end = lhs.size();

		for( auto i = start; i < end; i += 4 ){
			auto const* lhs_it = reinterpret_cast< float const* >( std::addressof( lhs[ i ] ) );
			auto const* rhs_it = reinterpret_cast< float const* >( std::addressof( rhs[ i ] ) );

			const auto a = simd::load( lhs_it );
			const auto b = simd::load( rhs_it );
			const auto c = a / b;

			auto* out_it = reinterpret_cast< float* >( std::addressof( result[ i ] ) );
			simd::store( c, std::span<float, 4>{ out_it, 4 } );
		}

		for( auto i = slack_start; i < slack_end; ++i ){
			result[ i ] = lhs[ i ] / rhs[ i ];
		}
	}
	
	template <vector_type VecT>
	std::string make_dot_hlsl(){
		using S = typename VecT::scalar_t;
		std::string vec_ty = get_typename<VecT>::value;
		std::string scalar_ty = get_typename<typename VecT::element_type>;

		return std::format( R"(
cbuffer Params : register(b0) {{
    uint count;
}};

StructuredBuffer<{}> A : register(t0);
StructuredBuffer<{}> B : register(t1);
RWStructuredBuffer<{}> Out : register(u0);

[numthreads(256,1,1)]
void main(uint3 dtid : SV_DispatchThreadID) {{
    uint i = dtid.x;
    if (i >= count) return;

    {} va = A[i];
    {} vb = B[i];

    {} sum = dot(va, vb);   // HLSL has built-in dot() for float[2-4]
    Out[i] = sum;
}}
)", vec_ty, vec_ty, scalar_ty, vec_ty, vec_ty, scalar_ty );
	}
}