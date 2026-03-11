#pragma once

#include "dny_win32sdk.hpp"

#include <span>
#include <string>
#include <system_error>
#include <type_traits>

#include <d3d11_4.h>
#include <d3dcompiler.h>

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

namespace dxgi{
	inline auto create_factory(){
		auto factory = wrl::ComPtr<IDXGIFactory2>{};
		auto hr = CreateDXGIFactory( IID_PPV_ARGS( factory.GetAddressOf() ) );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create DXGI factory." };
		}
		return factory;
	}

	inline auto enum_adapter( IDXGIFactory* factory, std::uint32_t index ){
		auto adapter = wrl::ComPtr<IDXGIAdapter>{};
		if( auto hr = factory->EnumAdapters( index, &adapter ); FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to enumerate DXGI adapter." };
		}
		return adapter;
	}

	inline auto enum_output( IDXGIAdapter* adapter, std::uint32_t index ){
		auto output = wrl::ComPtr<IDXGIOutput>{};
		auto hr = adapter->EnumOutputs( index, &output );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to enumerate DXGI output." };
		}
		return output;
	}

	inline auto create_swapchain(
		IDXGIFactory2* factory,
		ID3D11Device* d3d_device,
		HWND win_handle,
		DXGI_SWAP_CHAIN_DESC1 desc ){
		auto dxgi_device = wrl::ComPtr<IDXGIDevice>{};
		auto hr = d3d_device->QueryInterface(
			IID_PPV_ARGS( dxgi_device.ReleaseAndGetAddressOf() ) 
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to get DXGI device." };
		}

		auto swapchain = wrl::ComPtr<IDXGISwapChain1>{};
		hr = factory->CreateSwapChainForHwnd(
			dxgi_device.Get(),
			win_handle,
			&desc,
			nullptr,
			nullptr,
			&swapchain
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create DXGI swapchain." };
		}
		return swapchain;
	}

}



namespace d3d11{
	template<typename D3DResourceType> struct get_descriptor{};
	template<> struct get_descriptor<ID3D11Buffer>{
		using type = D3D11_BUFFER_DESC;
	};
	template<> struct get_descriptor<ID3D11Texture2D>{
		using type = D3D11_TEXTURE2D_DESC;
	};
	template<> struct get_descriptor<ID3D11ShaderResourceView>{
		using type = D3D11_SHADER_RESOURCE_VIEW_DESC;
	};
	template<> struct get_descriptor<ID3D11RenderTargetView>{
		using type = D3D11_RENDER_TARGET_VIEW_DESC;
	};

	template<typename D3DResourceType>
	using get_descriptor_t = typename get_descriptor<D3DResourceType>::type;

	template<typename D3DResourceType>
	class mapped_resource{
	public:
		struct create_params{
			wrl::ComPtr<ID3D11DeviceContext> context;
			wrl::ComPtr<D3DResourceType> resource;
			std::uint32_t subresource = 0u;
			D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD;
			std::uint32_t map_flags = 0u;
		};

	public:
		mapped_resource( create_params params )
			:
			m_params( std::move( params ) ){
			if( !m_params.context ){
				throw std::invalid_argument( "Device context pointer is null." );
			}

			auto hr = m_params.context->Map(
				m_params.resource.Get(),
				m_params.subresource,
				m_params.map_type,
				m_params.map_flags,
				&m_mapped
			);

			if( FAILED( hr ) ){
				throw win32::win32_error{ hr, "Failed to map resource." };
			}
		}
		mapped_resource( mapped_resource&& other ) noexcept
			:
			m_params( std::move( other.m_params ) ),
			m_mapped( other.m_mapped ){
			other.m_mapped = D3D11_MAPPED_SUBRESOURCE{};
		}
		~mapped_resource(){
			if( m_params.context ){
				m_params.context->Unmap(
					m_params.resource.Get(),
					m_params.subresource
				);
			}
		}

		mapped_resource& operator=( mapped_resource&& other ) noexcept{
			if( this != &other ){
				if( m_params.context ){
					m_params.context->Unmap(
						m_params.resource.Get(),
						m_params.subresource
					);
				}
				m_params = std::move( other.m_params );
				m_mapped = other.m_mapped;
				other.m_mapped = D3D11_MAPPED_SUBRESOURCE{};
			}
			return *this;
		}

		template<typename DataType>
		std::span<DataType> data(){
			auto desc = get_descriptor_t<D3DResourceType>{};
			m_params.resource->GetDesc( &desc );

			const auto buffer_size = [ & ](){
				if constexpr( std::is_same_v<D3DResourceType, ID3D11Buffer> ){
					return desc.ByteWidth / sizeof( DataType );
				}
				else if constexpr( std::is_same_v<D3DResourceType, ID3D11Texture2D> ){
					return desc.Width * desc.Height * sizeof( DataType );
				}
				else{
					static_assert(
						false,
						"Unsupported D3DResourceType for mapped_resource::data<DataType>()"
						);
				}
			}( );
			return std::span<DataType>(
				reinterpret_cast< DataType* >( m_mapped.pData ),
				buffer_size
			);
		}
		std::size_t pitch()const{
			if constexpr( std::is_same_v<D3DResourceType, ID3D11Texture2D> ){
				return static_cast< std::size_t >( m_mapped.RowPitch / 4 );
			}
			else{
				return static_cast< std::size_t >( m_mapped.RowPitch );
			}
		}
	private:
		create_params m_params;
		D3D11_MAPPED_SUBRESOURCE m_mapped;
	};


	inline auto create_device(){
		auto device = Microsoft::WRL::ComPtr<ID3D11Device>{};
		auto hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&device,
			nullptr,
			nullptr
		);

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create D3D11 device." };
		}

		return device;
	}

	inline auto create_immediate_context( ID3D11Device* device ){
		auto context = Microsoft::WRL::ComPtr<ID3D11DeviceContext>{};
		device->GetImmediateContext( &context );
		return context;
	}
	inline auto create_deferred_context( ID3D11Device* device ){
		auto context = Microsoft::WRL::ComPtr<ID3D11DeviceContext>{};
		auto hr = device->CreateDeferredContext( 0, &context );
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create D3D11 deferred context." };
		}
		return context;
	}
	inline auto get_back_buffer( IDXGISwapChain* swapchain, std::uint32_t buffer_index = 0u ){
		auto back_buffer = wrl::ComPtr<ID3D11Texture2D>{};
		auto hr = swapchain->GetBuffer(
			buffer_index,
			__uuidof( ID3D11Texture2D ),
			reinterpret_cast< void** >( back_buffer.GetAddressOf() )
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to get D3D11 back buffer from swapchain." };
		}
		return back_buffer;
	}
	template<typename DataType>
	auto create_texture2d( ID3D11Device* device, D3D11_TEXTURE2D_DESC const& desc, std::span<DataType> init_data = {} ){
		D3D11_SUBRESOURCE_DATA srd{
			.pSysMem = init_data.data(),
			.SysMemPitch = static_cast< UINT >( sizeof( DataType ) * desc.Width ),
		};

		auto texture = wrl::ComPtr<ID3D11Texture2D>{};
		auto hr = device->CreateTexture2D(
			&desc,
			init_data.data() ? &srd : nullptr,
			&texture
		);

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create D3D11 texture2D." };
		}

		return texture;
	}
	inline auto create_shader_resource_view( ID3D11Device* device, ID3D11Resource* resource, D3D11_SHADER_RESOURCE_VIEW_DESC const& desc ){
		auto srv = wrl::ComPtr<ID3D11ShaderResourceView>{};
		auto hr = device->CreateShaderResourceView(
			resource,
			&desc,
			&srv
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create D3D11 shader resource view." };
		}
		return srv;
	}
	inline auto create_render_target_view( ID3D11Device* device, ID3D11Resource* resource, D3D11_RENDER_TARGET_VIEW_DESC const& desc ){
		auto rtv = wrl::ComPtr<ID3D11RenderTargetView>{};
		auto hr = device->CreateRenderTargetView(
			resource,
			&desc,
			&rtv
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create D3D11 render target view." };
		}
		return rtv;
	}
	template<typename DataType>
	auto create_buffer( ID3D11Device* device, D3D11_BUFFER_DESC const& desc, std::span<const DataType> init_data = {} ){
		D3D11_SUBRESOURCE_DATA srd{
			.pSysMem = init_data.data(),
		};
		auto buffer = wrl::ComPtr<ID3D11Buffer>{};
		auto hr = device->CreateBuffer(
			&desc,
			init_data.data() ? &srd : nullptr,
			&buffer
		);
		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create D3D11 buffer." };
		}
		return buffer;
	}
	inline auto create_vertex_shader( ID3D11Device* device, ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr ){
		auto vshader = wrl::ComPtr<ID3D11VertexShader>{};
		auto hr = device->CreateVertexShader(
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(),
			linkage,
			&vshader
		);

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create the vertex shader." };
		}

		return vshader;
	}
	inline auto create_pixel_shader( ID3D11Device* device, ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr ){
		auto pshader = wrl::ComPtr<ID3D11PixelShader>{};

		auto hr = device->CreatePixelShader(
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(),
			linkage,
			&pshader
		);

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create pixel shader." };
		}

		return pshader;
	}
	inline auto create_sampler_state( ID3D11Device* device, D3D11_SAMPLER_DESC const& sampler_desc ){
		auto sampler = wrl::ComPtr<ID3D11SamplerState>{};
		const auto hr = device->CreateSamplerState(
			&sampler_desc,
			&sampler
		);

		if( FAILED( hr ) ){
			throw win32::win32_error{ hr, "Failed to create sampler state." };
		}

		return sampler;
	}

	template<typename ResourceType>
	auto map_resource( typename mapped_resource<ResourceType>::create_params params ){
		return mapped_resource<ResourceType>{ std::move( params ) };
	}
	template<typename ShaderType>
	void set_shader( ID3D11DeviceContext* context, ShaderType* shader, std::span<ID3D11ClassInstance*> class_instances ){
		using shader_setter = void( _stdcall ID3D11DeviceContext::* )( ShaderType*, ID3D11ClassInstance* const*, std::uint32_t );
		//using temp_type = decltype( &ID3D11DeviceContext::VSSetShader );

		shader_setter setter;
		if constexpr( std::is_same_v<ShaderType, ID3D11VertexShader>){
			setter = &ID3D11DeviceContext::VSSetShader;
			//context->VSSetShader(
			//	shader,
			//	class_instances.data(),
			//	static_cast< std::uint32_t >( class_instances.size() )
			//);
		}
		else if constexpr( std::is_same_v<ShaderType, ID3D11GeometryShader> ){
			setter = &ID3D11DeviceContext::GSSetShader;
			//context->GSSetShader(
			//	shader,
			//	class_instances.data(),
			//	static_cast< std::uint32_t >( class_instances.size() )
			//);
		}
		else if constexpr( std::is_same_v<ShaderType, ID3D11PixelShader> ){
			setter = &ID3D11DeviceContext::PSSetShader;
			//context->PSSetShader(
			//	shader,
			//	class_instances.data(),
			//	static_cast< std::uint32_t >( class_instances.size() )
			//);
		}
		else{
			static_assert( false, "Unsupported shader type" );
		}

		( context->*setter )(
			shader,
			class_instances.data(), 
			static_cast< std::uint32_t >( class_instances.size() )
		);
	}

	inline auto compile_shader(
		std::string source,
		std::string shader_version,
		std::string entry = "main",
		std::uint32_t flags1 = 0u,
		std::optional<D3D_SHADER_MACRO> macros = {},
		ID3DInclude* pInclude = nullptr
	){
		auto blob = wrl::ComPtr<ID3DBlob>{};
		auto err_blob = wrl::ComPtr<ID3DBlob>{};

		const auto hr = D3DCompile(
			source.c_str(),
			source.size(),
			nullptr,
			macros ? &( *macros ) : nullptr,
			pInclude,
			entry.c_str(),
			shader_version.c_str(),
			flags1,
			0u,
			&blob,
			&err_blob
		);

		if( FAILED( hr ) ){
			std::string reason = std::string{
				reinterpret_cast< char* >( err_blob->GetBufferPointer() ),
				err_blob->GetBufferSize()
			};

			throw win32::win32_error{ hr, reason };
		}

		return blob;
	}
}