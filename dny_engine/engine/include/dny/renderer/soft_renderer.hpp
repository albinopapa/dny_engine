#pragma once

#include "core/colors.hpp"
#include "core/concepts.hpp"
#include "core/surface.hpp"
#include "core/utilities.hpp"
#include "math/math.hpp"
#include "math/simd.hpp"
#include "raster_state.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#undef far
#undef near

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

				const auto l1 = mc00 + ( mc01 - mc00 ) * mt;
				const auto l2 = mc10 + ( mc11 - mc10 ) * mt;
				const auto result = l1 + ( l2 - l1 ) * mu;

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

	template<vertex_shader VShader, pixel_shader PShader, typename RasterState = default_raster_state>
		requires std::is_same_v<typename VShader::vertex_out, typename PShader::vertex_in>
	class basic_effect{
	public:
		using vshader_type = VShader;
		using pshader_type = PShader;
		using raster_state_type = RasterState;
		using buffer_type = std::vector<typename VShader::vertex_in>;

		static_assert( raster_state_policy<RasterState>, "RasterState must satisfy raster_state_policy concept" );

		vshader_type vshader;
		pshader_type pshader;
		std::vector<buffer_type const*> vertex_buffer;
	};

	template<typename Effect>
	class pipeline{
	public:
		using vshader_type = typename Effect::vshader_type;
		using vshader_in = typename vshader_type::vertex_in;
		using vshader_out = typename vshader_type::vertex_out;
		using vertex_buffer_type = std::vector<vshader_in>;
		using vshader_cbuffer_type = typename vshader_type::constant_buffer_t;

		using pshader_type = typename Effect::pshader_type;
		using pshader_in = vshader_out;
		using pshader_cbuffer_type = typename pshader_type::constant_buffer_t;
		using pshader_sampler_type = typename pshader_type::sampler_type_t;
		using raster_state_type = typename Effect::raster_state_type;

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
		void set_cbuffer( vshader_cbuffer_type const& cb_ ){
			m_effect.vshader.cbuffer = cb_;
		}
		void set_cbuffer( pshader_cbuffer_type const& cb_ ){
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
			auto simd_is_top_left = [ & ](
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

			// Backface culling test using compile-time policy
			if constexpr( raster_state_type::culling_mode != cull_mode::none ){
				constexpr bool front_is_negative = ( raster_state_type::front_face_winding == front_face::cw );
				const auto area_scalar = simd::extract<0>( area );
				const bool tri_is_front = front_is_negative ? ( area_scalar < 0.f ) : ( area_scalar > 0.f );

				if constexpr( raster_state_type::culling_mode == cull_mode::back ){
					if( !tri_is_front ) return;
				}
				else if constexpr( raster_state_type::culling_mode == cull_mode::front ){
					if( tri_is_front ) return;
				}
			}

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

						//if( w0 < 0.0f || w1 < 0.0f || w2 < 0.0f )continue;
						//if( ( w0 < epsilon && !tl0 ) ||
						//	( w1 < epsilon && !tl1 ) ||
						//	( w2 < epsilon && !tl2 ) ) continue;
						if( w0 > 0.0f || w1 > 0.0f || w2 > 0.0f )continue;
						if( ( w0 > epsilon && !tl0 ) ||
							( w1 > epsilon && !tl1 ) ||
							( w2 > epsilon && !tl2 ) ) continue;
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

					// ---- Depth test
					const auto depth = simd::extract<2>(
						( simd::shuffle<2, 2, 2, 2>( va[ Position_ID ] ) * ( t_vec * inv_wa ) ) +
						( simd::shuffle<2, 2, 2, 2>( vb[ Position_ID ] ) * ( u_vec * inv_wb ) ) +
						( simd::shuffle<2, 2, 2, 2>( vc[ Position_ID ] ) * ( v_vec * inv_wc ) )
					);

					const auto idx = x + y * m_target->width();
					
					// Loop for AoS style
					std::array<__m128, num_elements> frag;
					for( std::size_t i = {}; i < num_elements; ++i ){
						frag[ i ] = (
							( va[ i ] * ( t_vec * inv_wa ) ) +
							( vb[ i ] * ( u_vec * inv_wb ) ) +
							( vc[ i ] * ( v_vec * inv_wc ) )
							) * w;
					}

					pshader_in frag_pshader;
					simd_array_to_tuple( frag, frag_pshader );

					// Run pixel shader
					auto src_optional = m_effect.pshader( frag_pshader );
					if( !src_optional )
						continue;

					// Compile-time depth test policy
					if constexpr( raster_state_type::depth_test_enabled ){
						const bool depth_passed = detail::depth_test(
							raster_state_type::depth_function,
							depth,
							m_depth_buffer->at( idx )
						);
						if( !depth_passed ){
							continue;
						}
					}

					// Compile-time depth write policy
					if constexpr( raster_state_type::depth_write_enabled ){
						m_depth_buffer->at( idx ) = depth;
					}

					const ColorF src_color = to_colorf( *src_optional );
					
					// Compile-time blending and color mask policies
					if constexpr( !raster_state_type::blend_enabled && 
								  raster_state_type::write_r && 
								  raster_state_type::write_g && 
								  raster_state_type::write_b && 
								  raster_state_type::write_a ){
						// Fast path: no blending, full color write
						m_target->pixel( x, y ) = to_color32( src_color );
					}
					else{
						const Color32 dst_color = m_target->pixel( x, y );
						const ColorF dst_colorf = to_colorf( dst_color );
						
						// Apply blending if enabled
						ColorF blended_color;
						if constexpr( raster_state_type::blend_enabled ){
							blended_color = blend<raster_state_type>( src_color, dst_colorf );
						}
						else{
							blended_color = src_color;
						}
						
						// Apply color write mask
						Color32 final_color = to_color32( blended_color );
						final_color = apply_color_mask<raster_state_type>( final_color, dst_color );
						
						m_target->pixel( x, y ) = final_color;
					}
				}
			}
		}

	private:
		Effect m_effect;
		surface<Color32>* m_target = nullptr;
		std::vector<std::float_t>* m_depth_buffer = nullptr;
	};

	//template<typename Effect>
	//class tiled_pipeline{
	//public:
	//	using vshader_type = Effect::vshader_type;
	//	using vshader_in = vshader_type::vertex_in;
	//	using vshader_out = vshader_type::vertex_out;
	//	using vertex_buffer_type = std::vector<vshader_in>;
	//	using vshader_cbuffer_type = vshader_type::constant_buffer_t;

	//	using pshader_type = Effect::pshader_type;
	//	using pshader_in = vshader_out;
	//	using pshader_cbuffer_type = pshader_type::constant_buffer_t;
	//	using pshader_sampler_type = pshader_type::sampler_type_t;

	//	static constexpr std::int32_t Position_ID = vshader_type::Position_ID;
	//	static constexpr std::int32_t SV_Position_ID = pshader_type::Position_ID;

	//private:
	//	static constexpr std::size_t tile_extent = 16;
	//	static constexpr std::size_t simd_width = 4;
	//	static constexpr std::size_t num_elements =
	//		std::tuple_size_v<decltype( std::declval<pshader_in>().m_fields )>;

	//	struct binned_triangle{
	//		std::array<__m128, num_elements> va;
	//		std::array<__m128, num_elements> vb;
	//		std::array<__m128, num_elements> vc;

	//		float ax = 0.f, ay = 0.f;
	//		float bx = 0.f, by = 0.f;
	//		float cx = 0.f, cy = 0.f;

	//		float edge_a[ 3 ] = {};
	//		float edge_b[ 3 ] = {};
	//		float edge_c[ 3 ] = {};
	//		bool top_left[ 3 ] = {};

	//		int min_x = 0;
	//		int min_y = 0;
	//		int max_x = 0;
	//		int max_y = 0;

	//		int tile_min_x = 0;
	//		int tile_min_y = 0;
	//		int tile_max_x = 0;
	//		int tile_max_y = 0;

	//		float inv_area_scalar = 0.f;
	//		__m128 inv_wa = _mm_setzero_ps();
	//		__m128 inv_wb = _mm_setzero_ps();
	//		__m128 inv_wc = _mm_setzero_ps();
	//	};

	//public:
	//	void render( vertex_buffer_type const& mesh ){
	//		if( !m_target ) return;
	//		if( !m_depth_buffer ) return;
	//		if( mesh.size() < 3 ) return;
	//		if( ( mesh.size() % 3 ) != 0 ) return;

	//		prepare_bins();
	//		m_binned_triangles.reserve( mesh.size() / 3 );

	//		auto vert_id = std::size_t{};
	//		auto run_vshader = [ & ]( auto const& v ){
	//			m_effect.vshader.vertex_id = vert_id++;
	//			return m_effect.vshader( v ); // MUST return clip-space
	//		};

	//		for( std::size_t j = 0; j < mesh.size(); j += 3 ){
	//			clip_triangle(
	//				run_vshader( mesh[ j + 0 ] ),
	//				run_vshader( mesh[ j + 1 ] ),
	//				run_vshader( mesh[ j + 2 ] )
	//			);
	//		}

	//		rasterize_bins();
	//	}

	//	void update_cbuffer( vshader_cbuffer_type const& cb_ ){
	//		m_effect.vshader.cbuffer = cb_;
	//	}
	//	void update_cbuffer( pshader_cbuffer_type const& cb_ ){
	//		m_effect.pshader.cbuffer = cb_;
	//	}
	//	void set_render_target( surface<Color32>& rt_ ){
	//		m_target = std::addressof( rt_ );
	//	}
	//	void set_depth_buffer( std::vector<float>& db_ ){
	//		m_depth_buffer = std::addressof( db_ );
	//	}
	//	void set_pixel_shader_textures( std::size_t slot_, surface<ColorF> const& texture_ ){
	//		m_effect.pshader.texture[ slot_ ] = std::addressof( texture_ );
	//	}

	//private:
	//	enum class clip_plane{
	//		left, right, top, bottom, near, far
	//	};

	//	constexpr bool inside( vector4<float> const& p, clip_plane plane ){
	//		switch( plane ){
	//			case clip_plane::left:   return p.x >= -p.w;
	//			case clip_plane::right:  return p.x <= p.w;
	//			case clip_plane::bottom: return p.y >= -p.w;
	//			case clip_plane::top:    return p.y <= p.w;
	//			case clip_plane::near:   return p.z >= 0.0f;
	//			case clip_plane::far:    return p.z <= p.w;
	//		}
	//		return false;
	//	}

	//	constexpr float intersect_t(
	//		dny::vector4<float> const& a,
	//		dny::vector4<float> const& b,
	//		clip_plane plane ){
	//		float da = 0.f, db = 0.f;

	//		switch( plane ){
	//			case clip_plane::left:   da = a.x + a.w; db = b.x + b.w; break;
	//			case clip_plane::right:  da = a.w - a.x; db = b.w - b.x; break;
	//			case clip_plane::bottom: da = a.y + a.w; db = b.y + b.w; break;
	//			case clip_plane::top:    da = a.w - a.y; db = b.w - b.y; break;
	//			case clip_plane::near:   da = a.z; db = b.z;             break;
	//			case clip_plane::far:    da = a.w - a.z; db = b.w - b.z; break;
	//		}

	//		const auto inv_range = 1.f / ( da - db );
	//		return da * inv_range;
	//	}

	//	void clip_triangle(
	//		pshader_in const& va_in,
	//		pshader_in const& vb_in,
	//		pshader_in const& vc_in ){
	//		auto const& va = std::get<SV_Position_ID>( va_in.m_fields );
	//		auto const& vb = std::get<SV_Position_ID>( vb_in.m_fields );
	//		auto const& vc = std::get<SV_Position_ID>( vc_in.m_fields );

	//		constexpr auto plane = clip_plane::near;

	//		const auto ia = inside( va, plane );
	//		const auto ib = inside( vb, plane );
	//		const auto ic = inside( vc, plane );
	//		const auto num_in =
	//			static_cast< int >( ia ) +
	//			static_cast< int >( ib ) +
	//			static_cast< int >( ic );

	//		switch( num_in ){
	//			case 3:
	//				bin_triangle( va_in, vb_in, vc_in );
	//				break;
	//			case 2:
	//				if( ia && ib ){
	//					const auto temp0 = lerp( vc_in, va_in, intersect_t( vc, va, plane ) );
	//					const auto temp1 = lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) );
	//					bin_triangle( va_in, vb_in, temp1 );
	//					bin_triangle( temp1, temp0, va_in );
	//				}
	//				else if( ia && ic ){
	//					const auto temp0 = lerp( va_in, vb_in, intersect_t( va, vb, plane ) );
	//					const auto temp1 = lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) );
	//					bin_triangle( va_in, temp0, vc_in );
	//					bin_triangle( vc_in, temp0, temp1 );
	//				}
	//				else{
	//					const auto temp0 = lerp( va_in, vb_in, intersect_t( va, vb, plane ) );
	//					const auto temp1 = lerp( vc_in, va_in, intersect_t( vc, va, plane ) );
	//					bin_triangle( temp0, vb_in, vc_in );
	//					bin_triangle( vc_in, temp1, temp0 );
	//				}
	//				break;
	//			case 1:
	//				if( ia ){
	//					bin_triangle(
	//						va_in,
	//						lerp( va_in, vb_in, intersect_t( va, vb, plane ) ),
	//						lerp( vc_in, va_in, intersect_t( vc, va, plane ) )
	//					);
	//				}
	//				else if( ib ){
	//					bin_triangle(
	//						lerp( va_in, vb_in, intersect_t( va, vb, plane ) ),
	//						vb_in,
	//						lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) )
	//					);
	//				}
	//				else{
	//					bin_triangle(
	//						lerp( vc_in, va_in, intersect_t( vc, va, plane ) ),
	//						lerp( vb_in, vc_in, intersect_t( vb, vc, plane ) ),
	//						vc_in
	//					);
	//				}
	//				break;
	//			default:
	//				return;
	//		}
	//	}

	//	void prepare_bins(){
	//		const auto width = static_cast< int >( m_target->width() );
	//		const auto height = static_cast< int >( m_target->height() );

	//		m_tile_count_x = std::max( 1, ( width + static_cast< int >( tile_extent ) - 1 ) / static_cast< int >( tile_extent ) );
	//		m_tile_count_y = std::max( 1, ( height + static_cast< int >( tile_extent ) - 1 ) / static_cast< int >( tile_extent ) );

	//		m_tile_bins.clear();
	//		m_tile_bins.resize( static_cast< std::size_t >( m_tile_count_x * m_tile_count_y ) );
	//		m_binned_triangles.clear();
	//	}

	//	static float signed_area(
	//		float ax, float ay,
	//		float bx, float by,
	//		float cx, float cy ){
	//		return ( ( bx - ax ) * ( cy - ay ) ) - ( ( by - ay ) * ( cx - ax ) );
	//	}

	//	static bool is_top_left(
	//		float x0, float y0,
	//		float x1, float y1 ){
	//		const auto dx = x1 - x0;
	//		const auto dy = y1 - y0;
	//		return ( dy < 0.f ) || ( dy == 0.f && dx > 0.f );
	//	}

	//	bool prepare_triangle(
	//		pshader_in const& va_in,
	//		pshader_in const& vb_in,
	//		pshader_in const& vc_in,
	//		binned_triangle& out_tri ){
	//		const auto half = simd::load( 0.5f );
	//		const auto one = simd::load( 1.f );

	//		const float fw = static_cast< float >( m_target->width() );
	//		const float fh = static_cast< float >( m_target->height() );

	//		const auto fw_simd = simd::load( fw );
	//		const auto fh_simd = simd::load( fh );

	//		auto screen_transform = [ & ]( simd::float4 v ){
	//			auto pos_x = simd::shuffle<0, 0, 0, 0>( v );
	//			auto pos_y = simd::shuffle<1, 1, 1, 1>( v );

	//			pos_x = ( pos_x * half + half ) * fw_simd;
	//			pos_y = ( one - ( pos_y * half + half ) ) * fh_simd;

	//			return simd::mask_merge<1, 1, 0, 0>(
	//				simd::mask_merge<1, 0, 0, 0>( pos_x, pos_y ),
	//				v
	//			);
	//		};

	//		tuple_to_simd_array( va_in, out_tri.va );
	//		tuple_to_simd_array( vb_in, out_tri.vb );
	//		tuple_to_simd_array( vc_in, out_tri.vc );

	//		out_tri.inv_wa =
	//			one / simd::shuffle<3, 3, 3, 3>( out_tri.va[ Position_ID ] );
	//		out_tri.inv_wb =
	//			one / simd::shuffle<3, 3, 3, 3>( out_tri.vb[ Position_ID ] );
	//		out_tri.inv_wc =
	//			one / simd::shuffle<3, 3, 3, 3>( out_tri.vc[ Position_ID ] );

	//		const auto position_va =
	//			screen_transform( out_tri.va[ Position_ID ] * out_tri.inv_wa );
	//		const auto position_vb =
	//			screen_transform( out_tri.vb[ Position_ID ] * out_tri.inv_wb );
	//		const auto position_vc =
	//			screen_transform( out_tri.vc[ Position_ID ] * out_tri.inv_wc );

	//		out_tri.ax = simd::extract<0>( position_va );
	//		out_tri.ay = simd::extract<1>( position_va );
	//		out_tri.bx = simd::extract<0>( position_vb );
	//		out_tri.by = simd::extract<1>( position_vb );
	//		out_tri.cx = simd::extract<0>( position_vc );
	//		out_tri.cy = simd::extract<1>( position_vc );

	//		const auto area = signed_area(
	//			out_tri.ax, out_tri.ay,
	//			out_tri.bx, out_tri.by,
	//			out_tri.cx, out_tri.cy );

	//		if( area <= epsilon ) return false;

	//		out_tri.inv_area_scalar = 1.f / area;

	//		out_tri.top_left[ 0 ] = is_top_left( out_tri.bx, out_tri.by, out_tri.cx, out_tri.cy );
	//		out_tri.top_left[ 1 ] = is_top_left( out_tri.cx, out_tri.cy, out_tri.ax, out_tri.ay );
	//		out_tri.top_left[ 2 ] = is_top_left( out_tri.ax, out_tri.ay, out_tri.bx, out_tri.by );

	//		out_tri.edge_a[ 0 ] = out_tri.by - out_tri.cy;
	//		out_tri.edge_b[ 0 ] = out_tri.cx - out_tri.bx;
	//		out_tri.edge_c[ 0 ] = ( out_tri.bx * out_tri.cy ) - ( out_tri.by * out_tri.cx );

	//		out_tri.edge_a[ 1 ] = out_tri.cy - out_tri.ay;
	//		out_tri.edge_b[ 1 ] = out_tri.ax - out_tri.cx;
	//		out_tri.edge_c[ 1 ] = ( out_tri.ay * out_tri.cx ) - ( out_tri.ax * out_tri.cy );

	//		out_tri.edge_a[ 2 ] = out_tri.ay - out_tri.by;
	//		out_tri.edge_b[ 2 ] = out_tri.bx - out_tri.ax;
	//		out_tri.edge_c[ 2 ] = ( out_tri.ax * out_tri.by ) - ( out_tri.ay * out_tri.bx );

	//		const auto width = static_cast< int >( m_target->width() );
	//		const auto height = static_cast< int >( m_target->height() );

	//		out_tri.min_x = std::max( 0, static_cast< int >( std::floor( std::min( { out_tri.ax, out_tri.bx, out_tri.cx } ) ) ) );
	//		out_tri.min_y = std::max( 0, static_cast< int >( std::floor( std::min( { out_tri.ay, out_tri.by, out_tri.cy } ) ) ) );
	//		out_tri.max_x = std::min( width, static_cast< int >( std::ceil( std::max( { out_tri.ax, out_tri.bx, out_tri.cx } ) ) ) );
	//		out_tri.max_y = std::min( height, static_cast< int >( std::ceil( std::max( { out_tri.ay, out_tri.by, out_tri.cy } ) ) ) );

	//		if( out_tri.min_x >= out_tri.max_x || out_tri.min_y >= out_tri.max_y ) return false;

	//		out_tri.tile_min_x = out_tri.min_x / static_cast< int >( tile_extent );
	//		out_tri.tile_min_y = out_tri.min_y / static_cast< int >( tile_extent );
	//	 out_tri.tile_max_x = ( out_tri.max_x - 1 ) / static_cast< int >( tile_extent );
	//		out_tri.tile_max_y = ( out_tri.max_y - 1 ) / static_cast< int >( tile_extent );

	//		out_tri.va[ Position_ID ] = simd::mask_merge<1, 1, 0, 0>( position_va, out_tri.va[ Position_ID ] );
	//		out_tri.vb[ Position_ID ] = simd::mask_merge<1, 1, 0, 0>( position_vb, out_tri.vb[ Position_ID ] );
	//		out_tri.vc[ Position_ID ] = simd::mask_merge<1, 1, 0, 0>( position_vc, out_tri.vc[ Position_ID ] );

	//		return true;
	//	}

	//	void bin_triangle(
	//		pshader_in const& va_in,
	//		pshader_in const& vb_in,
	//		pshader_in const& vc_in ){
	//		binned_triangle tri;
	//		if( !prepare_triangle( va_in, vb_in, vc_in, tri ) ) return;

	//		const auto tri_index = m_binned_triangles.size();
	//		m_binned_triangles.push_back( tri );

	//		for( int tile_y = tri.tile_min_y; tile_y <= tri.tile_max_y; ++tile_y ){
	//			for( int tile_x = tri.tile_min_x; tile_x <= tri.tile_max_x; ++tile_x ){
	//				m_tile_bins[ tile_index( tile_x, tile_y ) ].push_back( tri_index );
	//			}
	//		}
	//	}

	//	void rasterize_bins(){
	//		for( int tile_y = 0; tile_y < m_tile_count_y; ++tile_y ){
	//			for( int tile_x = 0; tile_x < m_tile_count_x; ++tile_x ){
	//				auto const& bin = m_tile_bins[ tile_index( tile_x, tile_y ) ];
	//				if( bin.empty() ) continue;

	//				const auto tile_origin_x = tile_x * static_cast< int >( tile_extent );
	//				const auto tile_origin_y = tile_y * static_cast< int >( tile_extent );
	//				const auto tile_end_x = std::min( tile_origin_x + static_cast< int >( tile_extent ), static_cast< int >( m_target->width() ) );
	//				const auto tile_end_y = std::min( tile_origin_y + static_cast< int >( tile_extent ), static_cast< int >( m_target->height() ) );

	//				for( auto tri_index : bin ){
	//					rasterize_tile(
	//						m_binned_triangles[ tri_index ],
	//						std::max( tile_origin_x, m_binned_triangles[ tri_index ].min_x ),
	//						std::max( tile_origin_y, m_binned_triangles[ tri_index ].min_y ),
	//						std::min( tile_end_x, m_binned_triangles[ tri_index ].max_x ),
	//						std::min( tile_end_y, m_binned_triangles[ tri_index ].max_y )
	//					);
	//				}
	//			}
	//		}
	//	}

	//	void rasterize_tile(
	//		binned_triangle const& tri,
	//		int min_x,
	//		int min_y,
	//		int max_x,
	//		int max_y ){
	//		if( min_x >= max_x || min_y >= max_y ) return;

	//		const auto zero = _mm_set1_ps( 0.f );
	//		const auto eps = _mm_set1_ps( epsilon );
	//		const auto lane_offsets = _mm_set_ps( 3.f, 2.f, 1.f, 0.f );
	//		const auto four_step = _mm_set1_ps( 4.f );

	//		const auto edge0_lane_step = _mm_mul_ps( _mm_set1_ps( tri.edge_a[ 0 ] ), lane_offsets );
	//		const auto edge1_lane_step = _mm_mul_ps( _mm_set1_ps( tri.edge_a[ 1 ] ), lane_offsets );
	//		const auto edge2_lane_step = _mm_mul_ps( _mm_set1_ps( tri.edge_a[ 2 ] ), lane_offsets );

	//		const auto edge0_group_step = _mm_mul_ps( _mm_set1_ps( tri.edge_a[ 0 ] ), four_step );
	//		const auto edge1_group_step = _mm_mul_ps( _mm_set1_ps( tri.edge_a[ 1 ] ), four_step );
	//		const auto edge2_group_step = _mm_mul_ps( _mm_set1_ps( tri.edge_a[ 2 ] ), four_step );

	//		for( int y = min_y; y < max_y; ++y ){
	//			const auto fy = static_cast< float >( y ) + 0.5f;

	//			const auto row_w0 = tri.edge_a[ 0 ] * ( static_cast< float >( min_x ) + 0.5f ) +
	//				tri.edge_b[ 0 ] * fy + tri.edge_c[ 0 ];
	//			const auto row_w1 = tri.edge_a[ 1 ] * ( static_cast< float >( min_x ) + 0.5f ) +
	//				tri.edge_b[ 1 ] * fy + tri.edge_c[ 1 ];
	//			const auto row_w2 = tri.edge_a[ 2 ] * ( static_cast< float >( min_x ) + 0.5f ) +
	//				tri.edge_b[ 2 ] * fy + tri.edge_c[ 2 ];

	//			auto w0 = _mm_add_ps( _mm_set1_ps( row_w0 ), edge0_lane_step );
	//			auto w1 = _mm_add_ps( _mm_set1_ps( row_w1 ), edge1_lane_step );
	//			auto w2 = _mm_add_ps( _mm_set1_ps( row_w2 ), edge2_lane_step );

	//			for( int x = min_x; x < max_x; x += static_cast< int >( simd_width ) ){
	//				const auto remaining = max_x - x;
	//				const auto valid_mask = remaining >= static_cast< int >( simd_width )
	//					? 0xF
	//					: ( 1 << remaining ) - 1;

	//				const auto inside0 = tri.top_left[ 0 ] ? _mm_cmpge_ps( w0, zero ) : _mm_cmpge_ps( w0, eps );
	//				const auto inside1 = tri.top_left[ 1 ] ? _mm_cmpge_ps( w1, zero ) : _mm_cmpge_ps( w1, eps );
	//				const auto inside2 = tri.top_left[ 2 ] ? _mm_cmpge_ps( w2, zero ) : _mm_cmpge_ps( w2, eps );

	//				auto coverage_mask = _mm_movemask_ps( _mm_and_ps( _mm_and_ps( inside0, inside1 ), inside2 ) );
	//				coverage_mask &= valid_mask;

	//				if( coverage_mask != 0 ){
	//					alignas( 16 ) float w0_values[ 4 ];
	//					alignas( 16 ) float w1_values[ 4 ];
	//					alignas( 16 ) float w2_values[ 4 ];
	//					_mm_store_ps( w0_values, w0 );
	//					_mm_store_ps( w1_values, w1 );
	//					_mm_store_ps( w2_values, w2 );

	//					for( int lane = 0; lane < static_cast< int >( simd_width ); ++lane ){
	//						if( ( coverage_mask & ( 1 << lane ) ) == 0 ) continue;

	//						const auto t = w0_values[ lane ] * tri.inv_area_scalar;
	//						const auto u = w1_values[ lane ] * tri.inv_area_scalar;
	//						const auto v = w2_values[ lane ] * tri.inv_area_scalar;

	//						const auto t_vec = _mm_set1_ps( t );
	//						const auto u_vec = _mm_set1_ps( u );
	//						const auto v_vec = _mm_set1_ps( v );

	//						const auto inv_w =
	//							( t_vec * tri.inv_wa ) +
	//							( u_vec * tri.inv_wb ) +
	//							( v_vec * tri.inv_wc );

	//						const auto w = _mm_div_ps( _mm_set1_ps( 1.f ), inv_w );

	//						std::array<__m128, num_elements> frag;
	//						for( std::size_t i = {}; i < num_elements; ++i ){
	//							frag[ i ] = (
	//								( tri.va[ i ] * ( t_vec * tri.inv_wa ) ) +
	//								( tri.vb[ i ] * ( u_vec * tri.inv_wb ) ) +
	//								( tri.vc[ i ] * ( v_vec * tri.inv_wc ) )
	//								) * w;
	//						}

	//						const auto px = x + lane;
	//						const auto depth = simd::extract<2>( frag[ Position_ID ] );

	//						const auto i = ( static_cast< std::size_t >( y ) * m_target->width() ) + static_cast< std::size_t >( px );
	//						if( depth >= m_depth_buffer->at( i ) )
	//							continue;

	//						pshader_in frag_pshader;
	//						simd_array_to_tuple( frag, frag_pshader );
	//						m_target->pixel( px, y ) = m_effect.pshader( frag_pshader );
	//					}
	//				}

	//				w0 = _mm_add_ps( w0, edge0_group_step );
	//				w1 = _mm_add_ps( w1, edge1_group_step );
	//				w2 = _mm_add_ps( w2, edge2_group_step );
	//			}
	//		}
	//	}

	//	std::size_t tile_index( int tile_x, int tile_y ) const{
	//		return static_cast< std::size_t >( tile_y * m_tile_count_x + tile_x );
	//	}

	//private:
	//	Effect m_effect;
	//	surface<Color32>* m_target = nullptr;
	//	std::vector<std::float_t>* m_depth_buffer = nullptr;

	//	int m_tile_count_x = 0;
	//	int m_tile_count_y = 0;
	//	std::vector<binned_triangle> m_binned_triangles;
	//	std::vector<std::vector<std::size_t>> m_tile_bins;
	//};

}
