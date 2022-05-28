#pragma once

#include "dxApplication.h"
#include "mesh.h"

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace mini::gk2
{
	class DuckDemo : public DxApplication
	{
	public:
		using Base = DxApplication;

		explicit DuckDemo(HINSTANCE appInstance);

	protected:

		void Update(const Clock& c) override;
		void Render() override;

		void SetPhongShaders();
		void SetCubeMapShaders();

		void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps);

		void DrawMesh(const Mesh& m, Matrix worldMtx);

		void UpdateCameraCB(Matrix viewMtx);
		void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }

		dx_ptr<ID3D11VertexShader> m_phongVS, m_envVS;
		dx_ptr<ID3D11PixelShader> m_phongPS, m_envPS;

		dx_ptr<ID3D11InputLayout> m_positionNormallayout;
		dx_ptr<ID3D11InputLayout> m_positionNormalTexlayout;

		dx_ptr<ID3D11SamplerState> m_samplerWrap;

		Mesh m_duck;
		Mesh m_box;
		Mesh m_waterPlane;

		Matrix m_projMtx;
		Matrix m_duckMtx;

		dx_ptr<ID3D11ShaderResourceView> m_cubeMap;

		dx_ptr<ID3D11Buffer> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;				   //vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbViewMtx;  //vertex shader constant buffer slot 1
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbLightPos; //pixel shader constant buffer slot 1
	};
}
