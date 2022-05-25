#pragma once
#include "mesh.h"
#include "constantBuffer.h"
#include "effect.h"

namespace mini
{
	class InputLayoutManager;

	class SpriteRenderer
	{
	public:
		class SpriteEffect : public StaticEffect<BasicEffect, VSConstantBuffers, PSSamplers>
		{
		public:
			SpriteEffect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps,
				dx_ptr<ID3D11SamplerState>&& sampler,
				const ConstantBuffer<DirectX::XMFLOAT4X4> cbSpriteTransform,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbTexTransform);
		};

		SpriteRenderer(const DxDevice& device, InputLayoutManager& lm);

		void DrawSprite(const dx_ptr<ID3D11DeviceContext>& context,
			const dx_ptr<ID3D11ShaderResourceView>& spriteTexture, DirectX::XMFLOAT2 spritePosition,
			DirectX::XMFLOAT2 spriteSize);

	private:
		ConstantBuffer<DirectX::XMFLOAT4X4> m_cbSpriteTransform, m_cbTextureTansform;
		Mesh m_quad;
		dx_ptr<ID3D11InputLayout> m_layout;
		dx_ptr<ID3D11RasterizerState> m_rsState;
		dx_ptr<ID3D11BlendState> m_bsState;
		dx_ptr<ID3D11DepthStencilState> m_dsState;
		SpriteEffect m_effect;
	};
}
