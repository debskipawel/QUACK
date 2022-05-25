#pragma once
#include "effect.h"
#include <DirectXMath.h>
#include "window.h"
#include "clock.h"

namespace mini
{
	class Window;
	class DxDevice;

	namespace gk2
	{
		class GUIRenderer
		{
		public:
			GUIRenderer(const DxDevice& device, const Window& w);
			~GUIRenderer();

			bool ProcessMessage(WindowMessage& msg);

			void Update(const Clock& c);

			void Render(const DxDevice& device);

		private:
			dx_ptr<ID3D11VertexShader> m_vs;
			dx_ptr<ID3D11PixelShader> m_ps;
			dx_ptr<ID3D11Buffer> m_indexBuffer, m_vertexBuffer;
			dx_ptr<ID3D11SamplerState> m_sampler;
			dx_ptr<ID3D11ShaderResourceView> m_fontTexture;
			dx_ptr<ID3D11BlendState> m_bs;
			dx_ptr<ID3D11RasterizerState> m_rs;
			dx_ptr<ID3D11DepthStencilState> m_dss;
			dx_ptr<ID3D11InputLayout> m_layout;
			ConstantBuffer<DirectX::XMFLOAT4X4> m_cbProj;
			int m_vertexCount, m_indexCount;
		};
	}
}
