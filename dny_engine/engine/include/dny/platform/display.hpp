#pragma once

#include "core/colors.hpp"
#include "math/math.hpp"
#include "d3d11sdk.hpp"

#include <span>

namespace dny{
	static constexpr std::string_view vshader_str = R"DELIM(
struct VSOut{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
};

VSOut main( uint vertexID : SV_VertexID ){
	// Fullscreen triangle covering NDC
	float2 positions[ 3 ] =
	{
		float2( -1.0f, -1.0f ),
		float2( -1.0f, 3.0f ),
		float2( 3.0f, -1.0f )
	};

	float2 uvs[ 3 ] =
	{
		float2( 0.0f, 1.0f ),
		float2( 0.0f, -1.0f ),
		float2( 2.0f, 1.0f )
	};

	VSOut o;
	o.pos = float4( positions[ vertexID ], 0.0f, 1.0f );
	o.uv = uvs[ vertexID ];

	return o;
} )DELIM";

	static constexpr std::string_view pshader_str = R"DELIM(
Texture2D    Framebuffer : register( t0 );
SamplerState PointClamp : register( s0 );

float4 main( float4 pos : SV_Position, float2 uv : TEXCOORD0 ) : SV_Target
{
	return Framebuffer.Sample( PointClamp, uv );
})DELIM";

	class display{
	public:
		using native_handle = void*;
	public:
		template<typename Handler>
		display( 
			std::int32_t width_, 
			std::int32_t height_, 
			Handler& handler_ )
			: m_width( width_ ), m_height( height_ ){

			init_window( width_, height_, handler_ );
			init_direct3d();
		}
		~display(){
			win32::destroy_window( m_handle );
			win32::unregister_window_class( s_class_name, m_instance );
		}

		void present( std::int32_t width_, std::int32_t height_, std::span<const Color32> buffer_ )const{
			const auto max_width = m_width / 2;
			const auto max_height = m_height / 2;
			if( buffer_.size() > max_width * max_height ){
				throw std::runtime_error{ "System buffer too large." };
			}
			update( buffer_ );

			d3d_context->OMSetRenderTargets( 1u, d3d_render_target.GetAddressOf(), nullptr );
			d3d_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			d3d_context->Draw( 3, 0 );
			dxgi_swapchain->Present( 0u, 0u );
		}

		std::int32_t width()const noexcept{
			return m_width;
		}
		std::int32_t height()const noexcept{
			return m_height;
		}

		native_handle handle()const{
			return m_handle;
		}
		Rect<std::int32_t> client_rect()const{
			RECT rect;
			GetClientRect( m_handle, &rect );
			return {
				static_cast< std::int32_t >( rect.left ),
				static_cast< std::int32_t >( rect.top ),
				static_cast< std::int32_t >( rect.right ),
				static_cast< std::int32_t >( rect.bottom )
			};
		}
		void clamp_cursor(){
			RECT rect;
			GetClientRect( m_handle, &rect );
			MapWindowPoints( m_handle, nullptr, reinterpret_cast< POINT* >( &rect ), 2 );
			ClipCursor( &rect );
		}
		void unclamp_cursor(){
			ClipCursor( nullptr );
		}
		void show(){ ShowWindow( m_handle, SW_SHOWDEFAULT ); }
		void hide(){ ShowWindow( m_handle, SW_MINIMIZE ); }
	private:
		template<typename HandlerT>
		void init_window( std::int32_t width_, std::int32_t height_, HandlerT& handler_ ){
			WNDPROC wproc = []( HWND win_handle_, UINT msg, WPARAM wparam, LPARAM lparam ){
				if( auto* handler = win32::get_user_data<HandlerT>( win_handle_ ) )
					return handler->message_proc( msg, wparam, lparam );

				return DefWindowProc( win_handle_, msg, wparam, lparam );
			};

			const auto wc = WNDCLASSEX{
				.cbSize = sizeof( WNDCLASSEX ),
				.lpfnWndProc = wproc,
				.hInstance = m_instance,
				.hCursor = LoadCursorW( nullptr, IDC_ARROW ),
				.lpszClassName = s_class_name,
			};

			const auto atom = win32::register_window_class( wc );

			const auto dt_rect = win32::get_client_rect( win32::get_desktop() );
			auto wrect = win32::adjust_window_rect(
				width_, height_, WS_OVERLAPPEDWINDOW, 0u, false
			);

			const auto x = ( dt_rect.right - ( wrect.right - wrect.left ) ) / 2;
			const auto y = ( dt_rect.bottom - ( wrect.bottom - wrect.top ) ) / 2;
			wrect.left += x;
			wrect.top += y;
			wrect.right += x;
			wrect.bottom += y;

			m_handle = win32::create_window( CREATESTRUCT{
				.hInstance = m_instance,
				.cy = wrect.bottom - wrect.top,
				.cx = wrect.right - wrect.left,
				.y = y,
				.x = x,
				.style = WS_OVERLAPPEDWINDOW,
				.lpszName = L"Sandbox Window",
				.lpszClass = s_class_name,
				} );

			win32::set_userdata( m_handle, handler_ );

		}
		void init_direct3d(){
			{ // Create device and device context
				d3d_device = d3d11::create_device();
				d3d_context = d3d11::create_immediate_context( d3d_device.Get() );
			}
			{// Create and set viewport
				D3D11_VIEWPORT vp{};
				vp.TopLeftX = 0.0f;
				vp.TopLeftY = 0.0f;
				vp.Width = static_cast< float >( width() );
				vp.Height = static_cast< float >( height() );
				vp.MinDepth = 0.0f;
				vp.MaxDepth = 1.0f;

				d3d_context->RSSetViewports( 1, &vp );
			}
			{ // Create swap chain
				auto scd = DXGI_SWAP_CHAIN_DESC1{};
				scd.BufferCount = 2;
				scd.Width = static_cast< std::uint32_t >( width() );
				scd.Height = static_cast< std::uint32_t >( height() );
				scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				scd.SampleDesc.Count = 1;
				scd.Scaling = DXGI_SCALING_STRETCH;
				scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

				auto dxgi_factory = dxgi::create_factory();
				dxgi_swapchain =
					dxgi::create_swapchain( dxgi_factory.Get(), d3d_device.Get(), m_handle, scd );
			}
			{ // Get swap chain back buffer and set as render target
				auto bbuffer = d3d11::get_back_buffer( dxgi_swapchain.Get() );
				auto rtv_desc = D3D11_RENDER_TARGET_VIEW_DESC{};
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				rtv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

				d3d_render_target =
					d3d11::create_render_target_view( d3d_device.Get(), bbuffer.Get(), rtv_desc );
			}
			{ // Create and set vertex shader
				auto v_shader_blob = d3d11::compile_shader( std::string( vshader_str ), "vs_5_0" );
				d3d_vertex_shader =
					d3d11::create_vertex_shader( d3d_device.Get(), v_shader_blob.Get() );
				d3d11::set_shader( d3d_context.Get(), d3d_vertex_shader.Get(), {} );
			}
			{ // Create and set point sampler
				auto sampler_desc = D3D11_SAMPLER_DESC{};
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

				d3d_point_sampler = d3d11::create_sampler_state( d3d_device.Get(), sampler_desc );
				d3d_context->PSSetSamplers( 0u, 1u, d3d_point_sampler.GetAddressOf() );
			}
			{ // Create screen texture
				auto tex_desc = D3D11_TEXTURE2D_DESC{};
				tex_desc.Width = static_cast< std::uint32_t >( width() / 2 );
				tex_desc.Height = static_cast< std::uint32_t >( height() / 2 );
				tex_desc.MipLevels = 1;
				tex_desc.ArraySize = 1;
				tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				tex_desc.SampleDesc = { 1u, 0u };
				tex_desc.Usage = D3D11_USAGE_DYNAMIC;
				tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				d3d_screen_texture =
					d3d11::create_texture2d<std::uint32_t>( d3d_device.Get(), tex_desc );
			}
			{ // Create and set shader resource view
				auto srv_desc = D3D11_SHADER_RESOURCE_VIEW_DESC{};
				srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				srv_desc.Texture2D = { 0, 1 };

				d3d_screen_view = d3d11::create_shader_resource_view(
					d3d_device.Get(),
					d3d_screen_texture.Get(),
					srv_desc
				);
				d3d_context->PSSetShaderResources( 0u, 1u, d3d_screen_view.GetAddressOf() );
			}
			{ // Create and set pixel shader
				auto p_shader_blob = d3d11::compile_shader( std::string( pshader_str ), "ps_5_0" );
				d3d_pixel_shader =
					d3d11::create_pixel_shader( d3d_device.Get(), p_shader_blob.Get() );
				d3d11::set_shader( d3d_context.Get(), d3d_pixel_shader.Get(), {} );
			}
		}
		void update( std::span<const Color32> buffer_ )const{
			auto mapper = d3d11::map_resource<ID3D11Texture2D>( {
				.context = d3d_context,
				.resource = d3d_screen_texture
				} );
			const auto res_pitch = mapper.pitch();
			auto data = mapper.data<Color32>();
			const auto buffer_width = m_width / 2;
			for( auto y = 0; y < m_height / 2; ++y ){
				auto in_beg = buffer_.begin() + ( y * buffer_width );
				auto in_end = in_beg + buffer_width;
				auto out_beg = data.begin() + ( y * res_pitch );
				std::copy( in_beg, in_end, out_beg );
			}
		}
	private:
		static constexpr const wchar_t s_class_name[] = L"dnyMyClass";

		HINSTANCE m_instance = win32::get_module_handle();
		HWND m_handle = nullptr;

		wrl::ComPtr<ID3D11Device>			  d3d_device;
		wrl::ComPtr<ID3D11DeviceContext>	  d3d_context;
		wrl::ComPtr<ID3D11RenderTargetView>	  d3d_render_target;
		wrl::ComPtr<ID3D11Texture2D>		  d3d_screen_texture;
		wrl::ComPtr<ID3D11ShaderResourceView> d3d_screen_view;
		wrl::ComPtr<IDXGISwapChain1>		  dxgi_swapchain;
		wrl::ComPtr<ID3D11SamplerState>		  d3d_point_sampler;
		wrl::ComPtr<ID3D11VertexShader>		  d3d_vertex_shader;
		wrl::ComPtr<ID3D11PixelShader>		  d3d_pixel_shader;

		std::int32_t m_width = {};
		std::int32_t m_height = {};
	};
}
