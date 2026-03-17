#pragma once

#include "effects.hpp"
#include "graphics/font.hpp"
#include "math/math_constants.hpp"

#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

// ============================================================
// 2D Renderer
//
// Provides a high-level 2D drawing API built on top of the 3D
// software renderer pipeline.  All 2D primitives are converted
// to 3D triangle meshes and submitted to the appropriate
// pipeline instance stored inside renderer2d.
//
// Coordinate convention
//   - Origin (0, 0) is the top-left corner of the render target.
//   - X increases to the right, Y increases downward.
//   - All geometry is placed at z = 0 in view space.
//
// Primitives  (line, rectangle, circle)  accept position + color.
// Sprites and text accept position + texture / UV coordinates.
// ============================================================

namespace dny{

// ------------------------------------------------------------------
// Orthographic projection for 2D rendering
//
// Maps pixel coordinates [0 .. width] x [0 .. height] to
// NDC [-1 .. 1] x [1 .. -1] (y is flipped so screen top = +1).
// ------------------------------------------------------------------
inline auto make_ortho_2d( float width, float height ) noexcept -> matrix_4x4<float>{
	auto m = matrix_4x4<float>{};
	m.m_rows[ 0 ] = {  2.f / width,    0.f,           0.f, 0.f };
	m.m_rows[ 1 ] = {  0.f,           -2.f / height,  0.f, 0.f };
	m.m_rows[ 2 ] = {  0.f,            0.f,           1.f, 0.f };
	m.m_rows[ 3 ] = { -1.f,            1.f,           0.f, 1.f };
	return m;
}

// ------------------------------------------------------------------
// Font pixel shader
//
// Samples the alpha channel from a white-on-transparent font atlas
// and multiplies it by a user-supplied tint colour.  Pixels whose
// atlas alpha is at or below epsilon are discarded.
// ------------------------------------------------------------------
struct font_2d_ps_cbuffer{
	ColorF tint_color{ 1.f, 1.f, 1.f, 1.f };
};

// Vertex type coming out of pnu_vertex_shader (position, normal, texcoord)
using font_2d_vertex_out = basic_vertex<vector4<float>, vector3<float>, vector2<float>>;

class font_2d_pixel_shader : public basic_pixel_shader<
	font_2d_vertex_out,
	font_2d_ps_cbuffer,
	bilinear_sampler>{
public:
	static constexpr std::size_t Position_ID = 0;

	std::optional<ColorF> operator()( vertex_in const& vin ) const noexcept{
		const auto uv     = std::get<2>( vin.m_fields );
		const auto sample = sampler.sample( uv, texture[ 0 ] );
		if( sample.alpha() <= epsilon ) return std::nullopt;
		return ColorF{
			cbuffer.tint_color.red(),
			cbuffer.tint_color.green(),
			cbuffer.tint_color.blue(),
			sample.alpha() * cbuffer.tint_color.alpha()
		};
	}
};

// Ensure font shader input matches pnu_vertex_shader output
static_assert(
	std::is_same_v<::pnu_vertex_shader::vertex_out, font_2d_vertex_out>,
	"font_2d_vertex_out must match pnu_vertex_shader::vertex_out"
);

// ------------------------------------------------------------------
// 2D effect and pipeline type aliases
// ------------------------------------------------------------------

// Coloured primitives (line, rectangle, circle) - vertex colour interpolation
using color_2d_effect   = basic_effect<::debug_vertex_shader, ::debug_pixel_shader,   raster_2d_state>;
// Textured sprites - bilinear UV sampling
using sprite_2d_effect  = basic_effect<::pnu_vertex_shader,   ::pnu_pixel_shader,     raster_2d_state>;
// Font glyphs - font atlas alpha sampling with tint colour
using font_2d_effect    = basic_effect<::pnu_vertex_shader,   font_2d_pixel_shader,   raster_2d_state>;

using color_2d_pipeline  = pipeline<color_2d_effect>;
using sprite_2d_pipeline = pipeline<sprite_2d_effect>;
using font_2d_pipeline   = pipeline<font_2d_effect>;

// ------------------------------------------------------------------
// renderer2d
//
// Context object that holds the three pipeline instances and exposes
// high-level draw functions for 2D content.
//
// Usage per frame:
//   ctx.begin( render_target, depth_buffer );   // once
//   ctx.fill_rect( ... );
//   ctx.draw_sprite( ... );
//   ctx.draw_text( ... );
//
// No explicit end() call is required; state is reset by the next
// begin() call.
// ------------------------------------------------------------------
class renderer2d{
	using color_vtx   = color_2d_pipeline::vshader_in;   // basic_vertex<vec3, ColorF>
	using texture_vtx = sprite_2d_pipeline::vshader_in;  // basic_vertex<vec3, vec3, vec2>

public:
	renderer2d() = default;

	// Call once before any draw calls to bind the render target and
	// build the orthographic projection for this frame.
	void begin( surface<Color32>& render_target, std::vector<float>& depth_buffer ){
		m_render_target = &render_target;
		m_depth_buffer  = &depth_buffer;

		const auto w = static_cast<float>( render_target.width()  );
		const auto h = static_cast<float>( render_target.height() );
		m_cbuffer = ::transform_constant_buffer{
			matrix_4x4<float>::identity(),
			matrix_4x4<float>::identity(),
			make_ortho_2d( w, h )
		};
	}

	// ----------------------------------------------------------------
	// Primitives - position + colour
	// ----------------------------------------------------------------

	// Draw a thick line segment between two 2D pixel-space points.
	void draw_line(
		vector2<float> p1,
		vector2<float> p2,
		ColorF         color,
		float          thickness = 1.f )
	{
		const auto raw = primitives::generate_debug_line_segment(
			vector3<float>{ p1.x, p1.y, 0.f },
			vector3<float>{ p2.x, p2.y, 0.f },
			thickness, color
		);
		std::vector<color_vtx> verts;
		verts.reserve( raw.size() );
		for( auto const& v : raw ){
			verts.push_back( color_vtx{ { v.position, v.color } } );
		}
		flush_color( verts );
	}

	void draw_line(
		vector2<std::int32_t> p1,
		vector2<std::int32_t> p2,
		Color32         color,
		float          thickness = 1.f ){
		draw_line(
			vector2<float>{ static_cast<float>( p1.x ), static_cast<float>( p1.y ) },
			vector2<float>{ static_cast<float>( p2.x ), static_cast<float>( p2.y ) },
			to_colorf( color ),
			thickness
		);
	}

	// Draw a rectangle outline.
	void draw_rect( Rect<float> const& rect, ColorF color, float thickness = 1.f ){
		std::vector<color_vtx> verts;
		verts.reserve( 24 );
		auto append = [ & ]( vector2<float> a, vector2<float> b ){
			for( auto const& v : primitives::generate_debug_line_segment(
				vector3<float>{ a.x, a.y, 0.f },
				vector3<float>{ b.x, b.y, 0.f },
				thickness, color ) )
			{
				verts.push_back( color_vtx{ { v.position, v.color } } );
			}
		};
		append( { rect.left,  rect.top    }, { rect.right, rect.top    } );
		append( { rect.right, rect.top    }, { rect.right, rect.bottom } );
		append( { rect.right, rect.bottom }, { rect.left,  rect.bottom } );
		append( { rect.left,  rect.bottom }, { rect.left,  rect.top    } );
		flush_color( verts );
	}
	void draw_rect( Rect<std::int32_t> const& rect, Color32 color, float thickness = 1.f ){
		draw_rect(
			Rect<float>{
				static_cast<float>( rect.left ),
				static_cast<float>( rect.top ),
				static_cast<float>( rect.right ),
				static_cast<float>( rect.bottom )
			},
			to_colorf( color ),
			thickness
		);
	}

	// Draw a solid filled rectangle.
	void fill_rect( Rect<float> const& rect, ColorF color ){
		auto mv = [ & ]( float x, float y ) -> color_vtx{
			return color_vtx{ { vector3<float>{ x, y, 0.f }, color } };
		};
		const std::vector<color_vtx> verts{
			mv( rect.left,  rect.top    ),
			mv( rect.right, rect.top    ),
			mv( rect.right, rect.bottom ),
			mv( rect.left,  rect.top    ),
			mv( rect.right, rect.bottom ),
			mv( rect.left,  rect.bottom )
		};
		flush_color( verts );
	}
	void fill_rect( Rect<std::int32_t> const& rect, Color32 color ){
		// Overload for integer rects and colours - just converts to float and forwards to the main function.
		fill_rect(
			Rect<float>{ static_cast<float>( rect.left ), static_cast<float>( rect.top ), static_cast<float>( rect.right ), static_cast<float>( rect.bottom ) },
			to_colorf( color )
		);
	}

	// Draw a solid filled circle approximated by a triangle fan.
	void fill_circle(
		vector2<float> center,
		float          radius,
		ColorF         color,
		std::uint32_t  segments = 32 )
	{
		segments = std::max( 3u, segments );
		std::vector<color_vtx> verts;
		verts.reserve( static_cast<std::size_t>( segments ) * 3 );
		const auto cv = color_vtx{ { vector3<float>{ center.x, center.y, 0.f }, color } };
		for( std::uint32_t i = 0; i < segments; ++i ){
			const auto a0 = PI2 * ( static_cast<float>( i     ) / static_cast<float>( segments ) );
			const auto a1 = PI2 * ( static_cast<float>( i + 1 ) / static_cast<float>( segments ) );
			verts.push_back( cv );
			verts.push_back( color_vtx{ {
				vector3<float>{
					center.x + std::cos( a0 ) * radius,
					center.y + std::sin( a0 ) * radius,
					0.f
				}, color
			} } );
			verts.push_back( color_vtx{ {
				vector3<float>{
					center.x + std::cos( a1 ) * radius,
					center.y + std::sin( a1 ) * radius,
					0.f
				}, color
			} } );
		}
		flush_color( verts );
	}
	void fill_circle(
		vector2<std::int32_t> center,
		std::int32_t                  radius,
		Color32         color,
		std::uint32_t  segments = 32 )
	{
		fill_circle(
			vector2<float>{ static_cast<float>( center.x ), static_cast<float>( center.y ) },
			static_cast<float>( radius ),
			to_colorf( color ),
			segments
		);
	}

	// ----------------------------------------------------------------
	// Sprites - position + texture coordinates
	// ----------------------------------------------------------------

	// Draw a textured quad filling dest using the full texture [0,0]-[1,1].
	void draw_sprite( Rect<float> const& dest, surface<ColorF> const& texture ){
		draw_sprite( dest, texture, Rect<float>{ 0.f, 0.f, 1.f, 1.f } );
	}
	void draw_sprite( Rect<std::int32_t> const& dest, surface<ColorF> const& texture ){
		draw_sprite(
			Rect<float>{
				static_cast<float>( dest.left ),
				static_cast<float>( dest.top ),
				static_cast<float>( dest.right ),
				static_cast<float>( dest.bottom )
			},
			texture,
			Rect<float>{ 0.f, 0.f, 1.f, 1.f }
		);
	}

	// Draw a textured quad filling dest using the UV sub-rectangle uv.
	// uv coordinates are in normalised [0, 1] space.  Use this for
	// sprite-sheet sub-regions.
	void draw_sprite(
		Rect<float> const&     dest,
		surface<ColorF> const& texture,
		Rect<float> const&     uv )
	{
		const auto n = vector3<float>{ 0.f, 0.f, -1.f };
		auto mv = [ & ]( float x, float y, float u, float v ) -> texture_vtx{
			return texture_vtx{ { vector3<float>{ x, y, 0.f }, n, vector2<float>{ u, v } } };
		};
		const std::vector<texture_vtx> verts{
			mv( dest.left,  dest.top,    uv.left,  uv.top    ),
			mv( dest.right, dest.top,    uv.right, uv.top    ),
			mv( dest.right, dest.bottom, uv.right, uv.bottom ),
			mv( dest.left,  dest.top,    uv.left,  uv.top    ),
			mv( dest.right, dest.bottom, uv.right, uv.bottom ),
			mv( dest.left,  dest.bottom, uv.left,  uv.bottom )
		};
		flush_sprite( verts, texture );
	}
	void draw_sprite(
		Rect<std::int32_t> const& dest,
		surface<ColorF> const& texture,
		Rect<std::int32_t> const& uv ){
		draw_sprite(
			Rect<float>{
				static_cast<float>( dest.left ),
				static_cast<float>( dest.top ),
				static_cast<float>( dest.right ),
				static_cast<float>( dest.bottom )
			},
			texture,
			Rect<float>{
				static_cast<float>( uv.left  ) / static_cast<float>( texture.width() ),
				static_cast<float>( uv.top ) / static_cast<float>( texture.height() ),
				static_cast<float>( uv.right ) / static_cast<float>( texture.width() ),
				static_cast<float>( uv.bottom ) / static_cast<float>( texture.height() )
			}
		);
	}

	// ----------------------------------------------------------------
	// Text - position + font-atlas texture coordinates
	// ----------------------------------------------------------------

	// Render a string of text starting at pixel-space position using
	// font.  The font atlas is converted to a greyscale alpha texture
	// on first use and cached for subsequent calls.
	void draw_text(
		std::string_view text,
		vector2<float>   position,
		Font const&      font,
		ColorF           color )
	{
		const auto& atlas = get_font_atlas( font );
		const auto  aw    = static_cast<float>( font.atlas_width()  );
		const auto  ah    = static_cast<float>( font.atlas_height() );
		const auto  n     = vector3<float>{ 0.f, 0.f, -1.f };

		std::vector<texture_vtx> verts;
		verts.reserve( text.size() * 6 );

		auto pen_x = position.x;
		for( char ch : text ){
			const auto gr  = font.glyph_rect( ch );
			const auto gw  = static_cast<float>( gr.width()  );
			const auto gh  = static_cast<float>( gr.height() );
			const auto adv = static_cast<float>( font.glyph_advance( ch ) );

			if( gw > 0.f && gh > 0.f ){
				const auto u0 = gr.left   / aw;
				const auto u1 = gr.right  / aw;
				const auto v0 = gr.top    / ah;
				const auto v1 = gr.bottom / ah;

				const auto x0 = pen_x;
				const auto y0 = position.y;
				const auto x1 = pen_x + gw;
				const auto y1 = position.y + gh;

				auto mv = [ & ]( float x, float y, float u, float v ) -> texture_vtx{
					return texture_vtx{ { vector3<float>{ x, y, 0.f }, n, vector2<float>{ u, v } } };
				};

				verts.push_back( mv( x0, y0, u0, v0 ) );
				verts.push_back( mv( x1, y0, u1, v0 ) );
				verts.push_back( mv( x1, y1, u1, v1 ) );
				verts.push_back( mv( x0, y0, u0, v0 ) );
				verts.push_back( mv( x1, y1, u1, v1 ) );
				verts.push_back( mv( x0, y1, u0, v1 ) );
			}

			pen_x += adv;
		}

		flush_font( verts, atlas, color );
	}

	void draw_text(
		std::string_view text,
		vector2<std::int32_t> position,
		Font const&           font,
		Color32               color )
	{
		draw_text(
			text,
			vector2<float>{ static_cast<float>( position.x ), static_cast<float>( position.y ) },
			font,
			to_colorf( color )
		);
	}

private:
	// ----------------------------------------------------------------
	// Flush helpers - configure and fire each pipeline
	// ----------------------------------------------------------------

	void flush_color( std::vector<color_vtx> const& verts ){
		if( verts.empty() || !m_render_target ) return;
		m_color_pipeline.set_render_target( *m_render_target );
		m_color_pipeline.set_depth_buffer( *m_depth_buffer );
		m_color_pipeline.set_cbuffer( m_cbuffer );
		m_color_pipeline.render( verts );
	}

	void flush_sprite( std::vector<texture_vtx> const& verts, surface<ColorF> const& tex ){
		if( verts.empty() || !m_render_target ) return;
		m_sprite_pipeline.set_render_target( *m_render_target );
		m_sprite_pipeline.set_depth_buffer( *m_depth_buffer );
		m_sprite_pipeline.set_cbuffer( m_cbuffer );
		m_sprite_pipeline.set_pixel_shader_textures( 0, tex );
		m_sprite_pipeline.render( verts );
	}

	void flush_font(
		std::vector<texture_vtx> const& verts,
		surface<ColorF> const&          atlas,
		ColorF const&                   tint )
	{
		if( verts.empty() || !m_render_target ) return;
		m_font_pipeline.set_render_target( *m_render_target );
		m_font_pipeline.set_depth_buffer( *m_depth_buffer );
		m_font_pipeline.set_cbuffer( m_cbuffer );
		m_font_pipeline.set_cbuffer( font_2d_ps_cbuffer{ tint } );
		m_font_pipeline.set_pixel_shader_textures( 0, atlas );
		m_font_pipeline.render( verts );
	}

	// Build (or retrieve from cache) a greyscale-alpha surface<ColorF>
	// from the font's Color32 atlas.  Pixels that are transparent black
	// in the atlas become alpha=0; all other pixels become opaque white
	// so the font_2d_pixel_shader can tint them freely.
	surface<ColorF> const& get_font_atlas( Font const& font ){
		auto const* key = &font;
		auto it = m_font_atlas_cache.find( key );
		if( it != m_font_atlas_cache.end() ) return it->second;

		const auto aw = static_cast<std::uint32_t>( font.atlas_width()  );
		const auto ah = static_cast<std::uint32_t>( font.atlas_height() );
		auto atlas = surface<ColorF>{ aw, ah };

		static constexpr auto trans_black = Color32{ 0u };
		for( std::uint32_t y = 0; y < ah; ++y ){
			for( std::uint32_t x = 0; x < aw; ++x ){
				const auto src = font.pixel(
					static_cast<std::int32_t>( x ),
					static_cast<std::int32_t>( y )
				);
				atlas.pixel( x, y ) = ( src != trans_black )
					? ColorF{ 1.f, 1.f, 1.f, 1.f }
					: ColorF{ 0.f, 0.f, 0.f, 0.f };
			}
		}

		auto [ ins, inserted ] = m_font_atlas_cache.emplace( key, std::move( atlas ) );
		return ins->second;
	}

private:
	color_2d_pipeline  m_color_pipeline;
	sprite_2d_pipeline m_sprite_pipeline;
	font_2d_pipeline   m_font_pipeline;

	surface<Color32>*   m_render_target = nullptr;
	std::vector<float>* m_depth_buffer  = nullptr;

	::transform_constant_buffer m_cbuffer;

	std::unordered_map<Font const*, surface<ColorF>> m_font_atlas_cache;
};

} // namespace dny
