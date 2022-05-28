#pragma once

#include "dxApplication.h"
#include "mesh.h"

namespace mini::gk2
{
	class DuckDemo : public DxApplication
	{
	public:
		using Base = DxApplication;

		explicit DuckDemo(HINSTANCE appInstance);

	protected:
		dx_ptr<ID3D11VertexShader> m_phongVS, m_textureVS, m_multiTexVS, m_particleVS;
		dx_ptr<ID3D11GeometryShader> m_particleGS;
		dx_ptr<ID3D11PixelShader> m_phongPS, m_texturePS, m_colorTexPS, m_multiTexPS, m_particlePS;

		dx_ptr<ID3D11InputLayout> m_phongInputlayout;

		Mesh m_duck;

		DirectX::XMFLOAT4X4 m_projMtx;
		DirectX::XMFLOAT4X4 m_duckMtx;

		dx_ptr<ID3D11Buffer> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;				   //vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbViewMtx;  //vertex shader constant buffer slot 1
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbLightPos; //pixel shader constant buffer slot 1


		void Update(const Clock& c) override;
		void Render() override;

		void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps);

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);

		void UpdateCameraCB(DirectX::XMMATRIX viewMtx);
		void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }
	};
}
