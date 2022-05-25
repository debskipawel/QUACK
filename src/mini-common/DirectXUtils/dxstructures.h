#pragma once

#include <d3d11.h>

namespace mini
{
	namespace utils
	{
		/**********************************************************************
		 Wraper for  DXGI_SWAP_CHAIN_DESC  structure. Provides  initialization
		 of members with default values.
		**********************************************************************/
		struct SwapChainDescription : DXGI_SWAP_CHAIN_DESC
		{
			explicit SwapChainDescription(HWND hWnd, UINT width = 0, UINT height = 0);
			SwapChainDescription(HWND hWnd, SIZE size) : SwapChainDescription(hWnd, size.cx, size.cy) { }
		};

		struct Texture2DDescription : D3D11_TEXTURE2D_DESC
		{
			explicit Texture2DDescription(UINT width = 0, UINT height = 0,
				DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, UINT mipLevels = 0);

			static Texture2DDescription DepthTextureDescription(UINT width = 0, UINT height = 0);
		};

		struct Texture3DDescription : D3D11_TEXTURE3D_DESC
		{
			explicit Texture3DDescription(UINT width = 0, UINT height = 0, UINT depth = 0,
				DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, UINT mipLevels = 0);
		};

		struct DepthViewDescription : D3D11_DEPTH_STENCIL_VIEW_DESC
		{
			DepthViewDescription();
		};

		struct ViewportDescription : D3D11_VIEWPORT
		{
			ViewportDescription(UINT width = 0, UINT height = 0);
		};

		struct BufferDescription : D3D11_BUFFER_DESC
		{
			BufferDescription(UINT byteWidth, D3D11_BIND_FLAG flags, D3D11_USAGE usage = D3D11_USAGE_DEFAULT);

			static BufferDescription VertexBufferDescription(UINT byteWidth, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
			{
				return{ byteWidth, D3D11_BIND_VERTEX_BUFFER, usage };
			}

			static BufferDescription IndexBufferDescription(UINT byteWidth)
			{
				return{ byteWidth, D3D11_BIND_INDEX_BUFFER };
			}

			template<typename T, UINT N = 1U>
			static BufferDescription ConstantBufferDescription()
			{
				return ConstantBufferDescription(sizeof(T)*N);
			}

			static BufferDescription ConstantBufferDescription(UINT byteWidth);
		};

		struct RasterizerDescription : D3D11_RASTERIZER_DESC
		{
			RasterizerDescription(bool counterClockwise = false);
		};

		struct SamplerDescription : D3D11_SAMPLER_DESC
		{
			SamplerDescription();
		};

		struct BlendDescription : D3D11_BLEND_DESC
		{
			BlendDescription();

			static BlendDescription AlphaBlendDescription();
			static BlendDescription AdditiveBlendDescription();
		};

		struct DepthStencilDescription : D3D11_DEPTH_STENCIL_DESC
		{
			DepthStencilDescription();

			static DepthStencilDescription StencilWriteDescription();
			static DepthStencilDescription StencilTestDescription();
		};

		struct ShaderResourceViewDescription : D3D11_SHADER_RESOURCE_VIEW_DESC
		{
			ShaderResourceViewDescription();

			static ShaderResourceViewDescription Texture2DViewDescription();
		};

		struct SubresourceData : D3D11_SUBRESOURCE_DATA
		{
			SubresourceData();
		};
	}
}