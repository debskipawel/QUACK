#pragma once

#include "dxptr.h"
#include "window.h"
#include "dxStructures.h"
#include <vector>

namespace mini
{
	class DxDevice
	{
	public:
		explicit DxDevice(const Window& window);

		const dx_ptr<ID3D11DeviceContext>& context() const { return m_context; }
		const dx_ptr<IDXGISwapChain>& swapChain() const { return m_swapChain; }
		ID3D11Device* operator->() const { return m_device.get(); }

		dx_ptr<ID3D11RenderTargetView> CreateRenderTargetView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

		dx_ptr<ID3D11Texture2D> CreateTexture(const D3D11_TEXTURE2D_DESC& desc) const;

		dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

		dx_ptr<ID3D11Buffer> CreateBuffer(const void* data, const D3D11_BUFFER_DESC& desc) const;

		static std::vector<BYTE> LoadByteCode(const std::wstring& filename);

		dx_ptr<ID3D11VertexShader> CreateVertexShader(std::vector<BYTE> vsCode) const;

		/***************** NEW *****************/
		//Geometry Shader Creation

		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(std::vector<BYTE> gsCode) const;

		dx_ptr<ID3D11PixelShader> CreatePixelShader(std::vector<BYTE> psCode) const;

		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elements, unsigned int count,
													const std::vector<BYTE>& vsCode) const;
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
													const std::vector<BYTE>& vsCode) const
		{
			return CreateInputLayout(elements.data(), elements.size(), vsCode);
		}
		template<unsigned int N>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC(&elements)[N],
													const std::vector<BYTE>& vsCode) const
		{
			return CreateInputLayout(elements, N, vsCode);
		}
		template<typename VertexType>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const std::vector<BYTE>& vsCode) const
		{
			return CreateInputLayout(VertexType::Layout, vsCode);
		}

		dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(SIZE size) const;
		template<class T>
		dx_ptr<ID3D11Buffer> CreateVertexBuffer(const std::vector<T>& vertices) const
		{
			auto desc = BufferDescription::VertexBufferDescription(vertices.size() * sizeof(T));
			return CreateBuffer(reinterpret_cast<const void*>(vertices.data()), desc);
		}

		/***************** NEW *****************/
		//New overload for dynamic vertex buffers (no initial data)
		
		template<class T>
		dx_ptr<ID3D11Buffer> CreateVertexBuffer(unsigned int N) const
		{
			auto desc = BufferDescription::VertexBufferDescription(N*sizeof(T));
			desc.Usage = D3D11_USAGE_DYNAMIC; //since no data is provided assume buffer data will be frequently modified
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			return CreateBuffer(nullptr, desc);
		}

		template<typename T>
		dx_ptr<ID3D11Buffer> CreateIndexBuffer(const std::vector<T>& indices) const
		{
			auto desc = BufferDescription::IndexBufferDescription(indices.size() * sizeof(T));
			return CreateBuffer(reinterpret_cast<const void*>(indices.data()), desc);
		}

		template<typename T, size_t N = 1>
		dx_ptr<ID3D11Buffer> CreateConstantBuffer() const
		{
			BufferDescription desc = BufferDescription::ConstantBufferDescription(N * sizeof(T));
			return CreateBuffer(nullptr, desc);
		}

		dx_ptr<ID3D11BlendState> CreateBlendState(const BlendDescription& desc = {}) const;

		dx_ptr<ID3D11DepthStencilState> CreateDepthStencilState(const DepthStencilDescription& desc = {}) const;

		dx_ptr<ID3D11RasterizerState> CreateRasterizerState(const RasterizerDescription& desc = { }) const;

		/***************** NEW *****************/

		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const dx_ptr<ID3D11Texture2D>& texture) const;
		//Loading textures from image/dds files using stand-alone DDS/WIC loaders
		//from DirectXTex texture processing library: https://github.com/microsoft/DirectXTex
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const std::wstring& texPath) const;

		dx_ptr<ID3D11SamplerState> CreateSamplerState(const SamplerDescription& desc) const;

	private:
		mini::dx_ptr<ID3D11Device> m_device;
		mini::dx_ptr<ID3D11DeviceContext> m_context;
		mini::dx_ptr<IDXGISwapChain> m_swapChain;
	};
}