#pragma once
#include <DirectXMath.h>
#include "dxDevice.h"

namespace mini
{
	namespace gk2
	{
		class EnvironmentMapper
		{
		public:
			enum VSConstantBufferSlots
			{
				WorldMtxSlot,
				ViewMtxSlot,
				ProjMtxSlot
			};

			enum PSConstantBufferSlots
			{
				SurfaceColorSlot
			};

			enum PSSamplerSlots
			{
				TextureSamplerSlot
			};

			static const int TEXTURE_SIZE;

			EnvironmentMapper() = default;

			EnvironmentMapper(const DxDevice& device, float nearPlane, float farPlane, DirectX::XMFLOAT3 position);

			void Begin(const dx_ptr<ID3D11DeviceContext>& context) const;

			DirectX::XMMATRIX FaceViewMtx(D3D11_TEXTURECUBE_FACE face) const;
			DirectX::XMFLOAT4X4 FaceProjMtx() const;

			void SetTarget(const dx_ptr<ID3D11DeviceContext>& context);
			void ClearTarget(const dx_ptr<ID3D11DeviceContext>& context);
			void SaveFace(const dx_ptr<ID3D11DeviceContext>& context, D3D11_TEXTURECUBE_FACE face);

		private:
			float m_nearPlane;
			float m_farPlane;
			DirectX::XMFLOAT4 m_position;

			dx_ptr<ID3D11VertexShader> m_envVS;
			dx_ptr<ID3D11PixelShader> m_envPS;

			dx_ptr<ID3D11Texture2D> m_envTexture, m_faceTexture;
			dx_ptr<ID3D11ShaderResourceView> m_envView;
			dx_ptr<ID3D11RenderTargetView> m_renderTarget;
			dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
		};
	}
}