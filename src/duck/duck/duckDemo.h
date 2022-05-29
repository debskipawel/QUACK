#pragma once

#include "dxApplication.h"
#include "mesh.h"

#include <queue>

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

		void SetDuckShaders();
		void SetPhongShaders();
		void SetCubeMapShaders();
		void SetWaterShaders();

		void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps);

		void DrawMesh(const Mesh& m, Matrix worldMtx);

		void UpdateRaindrops();
		void UpdateDuckPos();
		void UpdateWaterNormals();

		void UpdateCameraCB(Matrix viewMtx);
		void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }

		float m_waterLevel = -0.5f;

		std::vector<float> m_heights;
		std::vector<float> m_prevHeights;
		std::vector<float> m_absorption;
		std::vector<int> m_range;

		float m_time;
		const float DUCK_PERIOD = 5.0f;
		std::queue<Vector2> m_duckCurveControlPoints;

		dx_ptr<ID3D11VertexShader> m_phongVS, m_envVS, m_duckVS, m_waterVS;
		dx_ptr<ID3D11PixelShader> m_phongPS, m_envPS, m_duckPS, m_waterPS;

		dx_ptr<ID3D11InputLayout> m_positionNormalLayout;
		dx_ptr<ID3D11InputLayout> m_positionNormalTexLayout;

		dx_ptr<ID3D11RasterizerState> m_noCullRastState;
		dx_ptr<ID3D11SamplerState> m_samplerWrap;

		Mesh m_duck;
		Mesh m_box;
		Mesh m_waterPlane;

		Matrix m_projMtx;
		Matrix m_duckMtx;

		dx_ptr<ID3D11ShaderResourceView> m_cubeMap;
		dx_ptr<ID3D11ShaderResourceView> m_duckTexture;
		dx_ptr<ID3D11ShaderResourceView> m_grayNoise;

		dx_ptr<ID3D11Texture2D> m_waterNormalTexture;
		dx_ptr<ID3D11ShaderResourceView> m_waterNormalSrv;

		dx_ptr<ID3D11Buffer> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;				   //vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbViewMtx;  //vertex shader constant buffer slot 1
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbLightPos; //pixel shader constant buffer slot 1
	};
}
