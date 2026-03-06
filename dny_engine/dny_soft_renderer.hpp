#pragma once

#include "dny_colors.hpp"
#include "dny_concepts.hpp"
#include "dny_math.hpp"
#include "dny_utilities.hpp"
#include "dny_simd.hpp"
#include "dny_surface.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#undef far
#undef near

#if defined( _MSC_VER )
#include <intrin.h>
#elif defined( __SSE__ ) || defined( __x86_64__ ) || defined( __i386__ )
#include <xmmintrin.h>
#else
#error "dny::simd headers require SSE-capable x86 architecture"
#endif

namespace dny{
	template<typename... Fields>
	struct basic_vertex{
		constexpr basic_vertex& operator+=( basic_vertex const& other ) noexcept{
			add_impl( other, std::index_sequence_for<Fields...>{} );
			return *this;
		}
		constexpr basic_vertex& operator-=( basic_vertex const& other ) noexcept{
			sub_impl( other, std::index_sequence_for<Fields...>{} );
			return *this;
		}
		constexpr basic_vertex& operator*=( float factor )noexcept{
			mul_impl( factor, std::index_sequence_for<Fields...>{} );
			return *this;
		}
		constexpr basic_vertex& operator/=( float factor )noexcept{
			div_impl( factor, std::index_sequence_for<Fields...>{} );
			return *this;
		}
		std::tuple<Fields...> m_fields;

	private:
		template<std::size_t... I>
		constexpr void add_impl( basic_vertex const& other, std::index_sequence<I...> ){
			( ( std::get<I>( m_fields ) += std::get<I>( other.m_fields ) ), ... );
		}
		template<std::size_t... I>
		constexpr void sub_impl( basic_vertex const& other, std::index_sequence<I...> ){
			( ( std::get<I>( m_fields ) -= std::get<I>( other.m_fields ) ), ... );
		}
		template<std::size_t... I>
		constexpr void mul_impl( float factor, std::index_sequence<I...> ){
			( ( std::get<I>( m_fields ) *= factor ), ... );
		}
		template<std::size_t... I>
		constexpr void div_impl( float factor, std::index_sequence<I...> ){
			( ( std::get<I>( m_fields ) /= factor ), ... );
		}
	};

	template<typename...Fields> struct is_vertex<basic_vertex<Fields...>> : std::true_type{};

	constexpr auto operator+( vertex_type auto const& lhs, vertex_type auto const& rhs )noexcept{
		auto result = lhs;
		result += rhs;
		return result;
	}
	constexpr auto operator-( vertex_type auto const& lhs, vertex_type auto const& rhs )noexcept{
		auto result = lhs;
		result -= rhs;
		return result;
	}
	constexpr auto operator*( vertex_type auto const& lhs, float rhs )noexcept{
		auto result = lhs;
		result *= rhs;
		return result;
	}
	constexpr auto operator/( vertex_type auto const& lhs, float rhs )noexcept{
		auto result = lhs;
		result /= rhs;
		return result;
	}

	struct null_sampler{
		ColorF sample( vector2<float> const& uv, surface<ColorF> const* texture )const noexcept{
			return Colors::black;
		}
	};

	struct point_sampler{
		ColorF sample( vector2<float> const& uv, surface<ColorF> const* texture )const noexcept{
			if( texture->width() == 0 || texture->height() == 0 )return Colors::black;

			const auto tx = static_cast< std::uint32_t >(
				static_cast< float >( texture->width() - 1 ) * uv.x
				);
			const auto ty = static_cast< std::uint32_t >( 
				static_cast< float >( texture->height() - 1 ) * uv.y
				);
			
			return texture->pixel(
				static_cast< std::uint32_t >( tx ),
				static_cast< std::uint32_t >( ty )
			);
		}
	};

	struct bilinear_sampler{
		ColorF sample( vector2<float> const& uv, surface<ColorF> const* texture )const noexcept{
			if( !texture )return Colors::black;
			const auto f_width = static_cast< float >( texture->width() );
			const auto f_height = static_cast< float >( texture->height() );
			const auto tx = f_width * uv.x;  
			const auto ty = f_height * uv.y; 

			const auto ix = static_cast< std::uint32_t >( std::clamp( tx, 0.f, f_width - 2.f ) );
			const auto iy = static_cast< std::uint32_t >( std::clamp( ty, 0.f, f_height - 2.f ) );
			{
				const auto mc00 = simd::load( texture->pixel( ix + 0, iy + 0 ) );
				const auto mc01 = simd::load( texture->pixel( ix + 1, iy + 0 ) );
				const auto mc10 = simd::load( texture->pixel( ix + 0, iy + 1 ) );
				const auto mc11 = simd::load( texture->pixel( ix + 1, iy + 1 ) );

				const auto mtx = simd::load( tx );
				const auto mty = simd::load( ty );

				const auto mt = mtx - simd::truncate( mtx );
				const auto mu = mty - simd::truncate( mty );

				const auto l1 =   mc00 + ( mc01 - mc00 ) * mt;
				const auto l2 =   mc10 + ( mc11 - mc10 ) * mt;
				const auto result = l1 + ( l2   - l1 )   * mu;

				ColorF final_color;
				dny::simd::store( result, final_color );

				return final_color;
			}
		}
	};

	struct null_vs_constant_buffer{};
	struct null_ps_constant_buffer{};

	template<
		typename VertexIn,
		typename VertexOut,
		typename ConstantBuffer>
	struct basic_vertex_shader{
		using vertex_in = VertexIn;
		using vertex_out = VertexOut;
		using constant_buffer_t = ConstantBuffer;

		void update_cbuffer( constant_buffer_t const& cbuffer_ ){
			cbuffer = cbuffer_;
		}
		constant_buffer_t cbuffer;
		std::size_t vertex_id = {};
	};

	template<
		typename VertexIn,
		typename ConstantBuffer,
		sampler_type Sampler = null_sampler>
	struct basic_pixel_shader{
		using vertex_in = VertexIn;
		using sampler_type_t = Sampler;
		using constant_buffer_t = ConstantBuffer;

		void update_cbuffer( constant_buffer_t const& cbuffer_ ){
			cbuffer = cbuffer_;
		}
		constant_buffer_t cbuffer;
		sampler_type_t sampler;
		surface<ColorF> const* texture[ 8 ] = {};
	};

	template<vertex_shader VShader, pixel_shader PShader>
		requires std::is_same_v<typename VShader::vertex_out, typename PShader::vertex_in>
	class basic_effect{
	public:
		using vshader_type = VShader;
		using pshader_type = PShader;
		using buffer_type = std::vector<typename VShader::vertex_in>;

		vshader_type vshader;
		pshader_type pshader;
		std::vector<buffer_type const*> vertex_buffer;
	};

	template<typename Effect>
	class pipeline{
	public:
		using vshader_type = Effect::vshader_type;
		using vshader_in = vshader_type::vertex_in;
		using vshader_out = vshader_type::vertex_out;
		using vertex_buffer_type = std::vector<vshader_in>;
		using vshader_cbuffer_type = vshader_type::constant_buffer_t;

		using pshader_type = Effect::pshader_type;
		using pshader_in = vshader_out;
		using pshader_cbuffer_type = pshader_type::constant_buffer_t;
		using pshader_sampler_type = pshader_type::sampler_type_t;

		static constexpr std::int32_t Position_ID = vshader_type::Position_ID;
		static constexpr std::int32_t SV_Position_ID = pshader_type::Position_ID;


	public:
		void render( vertex_buffer_type const& mesh ){
			if( mesh.size() < 3 ) return;
			if( ( mesh.size() % 3 ) != 0 ) return;

			auto vert_id = 0;
			auto run_vshader = [ & ]( auto const& v ){
				m_effect.vshader.vertex_id = vert_id++;
				return m_effect.vshader( v ); // MUST return clip-space
			};

			for( std::size_t j = 0; j < mesh.size(); j += 3 ){
				clip_triangle(
					run_vshader( mesh[ j + 0 ] ),
					run_vshader( mesh[ j + 1 ] ),
					run_vshader( mesh[ j + 2 ] )
				);
			}
		}
		void update_cbuffer( vshader_cbuffer_type const& cb_ ){
			m_effect.vshader.cbuffer = cb_;
		}
		void update_cbuffer( pshader_cbuffer_type const& cb_ ){
			m_effect.pshader.cbuffer = cb_;
		}
		void set_render_target( surface<Color32>& rt_ ){
			m_target = std::addressof( rt_ );
		}
		void set_depth_buffer( std::vector<float>& db_ ){
			m_depth_buffer = std::addressof( db_ );
		}
		void set_pixel_shader_textures( std::size_t slot_, surface<ColorF> const& texture_ ){
			m_effect.pshader.texture[ slot_ ] = std::addressof( texture_ );
		}

	private:
		enum class clip_plane{
			left, right, top, bottom, near, far
		};

		constexpr bool inside( vector4<float> const& p, clip_plane plane ){
			switch( plane ){
				case clip_plane::left:   return p.x >= -p.w;
				case clip_plane::right:  return p.x <= p.w;
				case clip_plane::bottom: return p.y >= -p.w;
				case clip_plane::top:    return p.y <= p.w;
				case clip_plane::near:   return p.z >= 0.0f;
				case clip_plane::far:    return p.z <= p.w;
			}
			return false;
		}

		constexpr float intersect_t(
			dny::vector4<float> const& a,
			dny::vector4<float> const& b,
			clip_plane plane ){
			float da = 0.f, db = 0.f;

			switch( plane ){
				case clip_plane::left:   da = a.x + a.w; db = b.x + b.w; break;
				case clip_plane::right:  da = a.w - a.x; db = b.w - b.x; break;
				case clip_plane::bottom: da = a.y + a.w; db = b.y + b.w; break;
				case clip_plane::top:    da = a.w - a.y; db = b.w - b.y; break;
				case clip_plane::near:   da = a.z; db = b.z;			 break;
				case clip_plane::far:	 da = a.w - a.z; db = b.w - b.z; break;
			}

			const auto inv_range = 1.f / ( da - db );
			return da * inv_range;
		}

		void clip_triangle(
			pshader_in const& va_in,
			pshader_in const& vb_in,
			pshader_in const& vc_in ){
			using vert = pshader_in;

			//constexpr const clip_plane planes[] = {
			//	clip_plane::left,
			//	clip_plane::right,
			//	clip_plane::bottom,
			//	clip_plane::top,
			//	clip_plane::near,
			//	clip_plane::far
			//};

			auto const& va = std::get<SV_Position_ID>( va_in.m_fields );
			auto const& vb = std::get<SV_Position_ID>( vb_in.m_fields );
			auto const& vc = std::get<SV_Position_ID>( vc_in.m_fields );

			constexpr auto plane = clip_plane::near;

			const auto ia = inside( va, plane );
			const auto ib = inside( vb, plane );
			const auto ic = inside( vc, plane );
			const auto num_in =
				static_cast< int >( ia ) +
				static_cast< int >( ib ) +
				static_cast< int >( ic );

			switch( num_in ){
				case 3:
					rasterize( va_in, vb_in, vc_in );
					break;
				case 2:
					if( ia && ib ){
						const auto temp0 = lerp( vc_in, va_in, intersect_t( vc, va, plane ) );
						const auto temp1 = lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) );
						rasterize( va_in, vb_in, temp1 );
						rasterize( temp1, temp0, va_in );
					}
					else if( ia && ic ){
						const auto temp0 = lerp( va_in, vb_in, intersect_t( va, vb, plane ) );
						const auto temp1 = lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) );
						rasterize( va_in, temp0, vc_in );
						rasterize( vc_in, temp0, temp1 );
					}
					else{
						const auto temp0 = lerp( va_in, vb_in, intersect_t( va, vb, plane ) );
						const auto temp1 = lerp( vc_in, va_in, intersect_t( vc, va, plane ) );
						rasterize( temp0, vb_in, vc_in );
						rasterize( vc_in, temp1, temp0 );
					}
					break;
				case 1:
					if( ia ){
						rasterize(
							va_in,
							lerp( va_in, vb_in, intersect_t( va, vb, plane ) ),
							lerp( vc_in, va_in, intersect_t( vc, va, plane ) )
						);
					}
					else if( ib ){
						rasterize(
							lerp( va_in, vb_in, intersect_t( va, vb, plane ) ),
							vb_in,
							lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) )
						);
					}
					else{
						rasterize(
							lerp( vc_in, va_in, intersect_t( vc, va, plane ) ),
							lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) ),
							vc_in
						);
					}
					break;
				default:
					return; // fully clipped
			}
		}

		void rasterize(
			pshader_in const& va_in,
			pshader_in const& vb_in,
			pshader_in const& vc_in ){
			rasterize_simd( va_in, vb_in, vc_in );
		}

		auto rasterize_simd(
			pshader_in const& va_in,
			pshader_in const& vb_in,
			pshader_in const& vc_in
		){
			const auto zero = _mm_set1_ps( epsilon );
			const auto half = simd::load( 0.5f );
			const auto one = simd::load( 1.f );

			const float fw = static_cast< float >( m_target->width() );
			const float fh = static_cast< float >( m_target->height() );

			const auto fw_simd = simd::load( fw );
			const auto fh_simd = simd::load( fh );

			auto screen_transform = [ & ]( simd::float4 v ){
				auto pos_x = simd::shuffle<0, 0, 0, 0>( v );
				auto pos_y = simd::shuffle<1, 1, 1, 1>( v );

				pos_x = ( pos_x * half + half ) * fw_simd;
				pos_y = ( one - ( pos_y * half + half ) ) * fh_simd;

				return simd::mask_merge<1, 1, 0, 0>(
					simd::mask_merge<1, 0, 0, 0>( pos_x, pos_y ),
					v
				);
			};

			auto simd_signed_area = [](
				simd::float4 va_pos,
				simd::float4 vb_pos,
				simd::float4 vc_pos ){
				// const auto e1 = b - a;
				// const auto e2 = c - a;
				// ( e1.x * e2.y ) - ( e1.y * e2.x )

				const auto ba = vb_pos - va_pos;
				const auto ca = vc_pos - va_pos;

				const auto e1x = simd::shuffle<0, 0, 0, 0>( ba );
				const auto e1y = simd::shuffle<1, 1, 1, 1>( ba );
				const auto e2x = simd::shuffle<0, 0, 0, 0>( ca );
				const auto e2y = simd::shuffle<1, 1, 1, 1>( ca );
				return ( e1x * e2y ) - ( e1y * e2x );
			};

			// ---- Top-left rule
			auto simd_is_top_left = [&](
				simd::float4 v0,
				simd::float4 v1 ){
				// 		const auto dist = v1 - v0;
				// 		return ( dist.y < 0.f ) || ( dist.y == 0.f && dist.x > 0.f );
				const auto dist = v1 - v0;
				const auto dx = simd::shuffle<0, 0, 0, 0>( dist );
				const auto dy = simd::shuffle<1, 1, 1, 1>( dist );
				
				const auto result = 
					( dy < zero ) || ( ( dy <= zero ) && ( dx > zero ) );
				return simd::all( result );
			};

			static constexpr auto num_elements = std::tuple_size_v<decltype( va_in.m_fields )>;
			std::array<__m128, num_elements> va;
			std::array<__m128, num_elements> vb;
			std::array<__m128, num_elements> vc;
			tuple_to_simd_array( va_in, va );
			tuple_to_simd_array( vb_in, vb );
			tuple_to_simd_array( vc_in, vc );

			const auto inv_wa =
				one / simd::shuffle<3, 3, 3, 3>( va[ Position_ID ] );
			const auto inv_wb =
				one / simd::shuffle<3, 3, 3, 3>( vb[ Position_ID ] );
			const auto inv_wc =
				one / simd::shuffle<3, 3, 3, 3>( vc[ Position_ID ] );

			const auto position_va = 
				screen_transform( va[ Position_ID ] * inv_wa );
			const auto position_vb = 
				screen_transform( vb[ Position_ID ] * inv_wb );
			const auto position_vc = 
				screen_transform( vc[ Position_ID ] * inv_wc );

			const auto area = simd_signed_area( position_va, position_vb, position_vc );

			// Backface culling test (CW front facing)
			const auto is_backface = area < _mm_setzero_ps();
			if( simd::any( is_backface ) ) return;

			const auto inv_area = one / area;

			const auto tl0 = simd_is_top_left( position_vb, position_vc );
			const auto tl1 = simd_is_top_left( position_vc, position_va );
			const auto tl2 = simd_is_top_left( position_va, position_vb );

			// ---- Bounding box
			const auto limits = simd::load( fw, fh, 0.f, 0.f );
			const auto min_xy = simd::max(
				_mm_setzero_ps(),
				simd::floor( simd::min( simd::min(
					position_va, 
					position_vb ), 
					position_vc ) )
			);
			const auto max_xy = _mm_min_ps(
				limits,
				simd::ceil( simd::max( simd::max( 
					position_va, 
					position_vb ), 
					position_vc ) )
			);
			const auto min_x = static_cast< int >( simd::extract<0>( min_xy ) );
			const auto min_y = static_cast< int >( simd::extract<1>( min_xy ) );
			const auto max_x = static_cast< int >( simd::extract<0>( max_xy ) );
			const auto max_y = static_cast< int >( simd::extract<1>( max_xy ) );
			if( min_x >= max_x || min_y >= max_y ) return;

			// Reincorporate the 2D position
			va[ Position_ID ] = simd::mask_merge<1, 1, 0, 0>( position_va, va[ Position_ID ] );
			vb[ Position_ID ] = simd::mask_merge<1, 1, 0, 0>( position_vb, vb[ Position_ID ] );
			vc[ Position_ID ] = simd::mask_merge<1, 1, 0, 0>( position_vc, vc[ Position_ID ] );

			// ---- Raster loop
			for( int y = min_y; y < max_y; ++y ){
				const auto fy = static_cast< float >( y ) + 0.5f;

				for( int x = min_x; x < max_x; ++x ){
					const auto fx = static_cast< float >( x ) + 0.5f;

					const auto p = simd::load( dny::vector2{ fx, fy } );

					// Barycentric coordinates
					const auto w0_vec = simd_signed_area( p, position_vb, position_vc );
					const auto w1_vec = simd_signed_area( position_va, p, position_vc );
					const auto w2_vec = simd_signed_area( position_va, position_vb, p );

					{
						const auto w0 = simd::extract<0>( w0_vec );
						const auto w1 = simd::extract<0>( w1_vec );
						const auto w2 = simd::extract<0>( w2_vec );

						if( w0 < 0.0f || w1 < 0.0f || w2 < 0.0f )continue;
						if( ( w0 < epsilon && !tl0 ) ||
							( w1 < epsilon && !tl1 ) ||
							( w2 < epsilon && !tl2 ) ) continue;
					}

					// ---- Standard interpolation
					const auto t_vec = w0_vec * inv_area;
					const auto u_vec = w1_vec * inv_area;
					const auto v_vec = w2_vec * inv_area;

					// ---- Perspective-correct interpolation
					const auto inv_w =
						( t_vec * inv_wa ) +
						( u_vec * inv_wb ) +
						( v_vec * inv_wc );

					const auto w = one / inv_w;

					std::array<__m128, num_elements> frag;
					// Loop for AoS style
					for( std::size_t i = {}; i < num_elements; ++i ){
						frag[ i ] = (
							( va[ i ] * ( t_vec * inv_wa ) ) +
							( vb[ i ] * ( u_vec * inv_wb ) ) +
							( vc[ i ] * ( v_vec * inv_wc ) )
							) * w;
					}

					// ---- Depth test (z already post-divide)
					const auto depth = simd::extract<2>( frag[ Position_ID ] );

					if( !m_target->depth_test( x, y, depth ) )
						continue;

					pshader_in frag_pshader;
					simd_array_to_tuple( frag, frag_pshader );

					m_target->pixel( x, y ) = m_effect.pshader( frag_pshader );
				}
			}
		}

	private:
		Effect m_effect;
		surface<Color32>* m_target = nullptr;
		std::vector<std::float_t>* m_depth_buffer = nullptr;
	};
}