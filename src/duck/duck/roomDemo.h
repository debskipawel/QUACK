#pragma once
#include "dxApplication.h"
#include "mesh.h"
#include "environmentMapper.h"
#include "particleSystem.h"

namespace mini::gk2
{
	class RoomDemo : public DxApplication
	{
	public:
		using Base = DxApplication;

		explicit RoomDemo(HINSTANCE appInstance);

	protected:
		void Update(const Clock& dt) override;
		void Render() override;

	private:
#pragma region CONSTANTS
		static constexpr float TABLE_H = 1.0f;
		static constexpr float TABLE_TOP_H = 0.1f;
		static constexpr float TABLE_R = 1.5f;
		static constexpr float MAPPER_NEAR = 0.4f;
		static constexpr float MAPPER_FAR = 8.0f;

		//can't have in-class initializer since XMFLOAT... types' constructors are not constexpr
		static const DirectX::XMFLOAT3 TEAPOT_POS;
		static const DirectX::XMFLOAT4 TABLE_POS;
		static const DirectX::XMFLOAT4 LIGHT_POS[2];
#pragma endregion
		dx_ptr<ID3D11Buffer> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx,	//vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
			m_cbTex1Mtx,	//vertex shader constant buffer slot 3
			m_cbTex2Mtx;	//vertex shader constant buffer slot 4
		dx_ptr<ID3D11Buffer> m_cbViewMtx; //vertex shader constant buffer slot 1
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbLightPos; //pixel shader constant buffer slot 1

		Mesh m_wall; //uses m_wallsMtx[6]
		Mesh m_sphere; //uses m_sphereMtx
		Mesh m_teapot; //uses m_tepotMtx
		Mesh m_box; //uses m_boxMtx
		Mesh m_lamp; //uses m_lampMtx
		Mesh m_chairSeat; //uses m_chairMtx
		Mesh m_chairBack; //uses m_chairMtx
		Mesh m_tableLeg; //uses m_tableLegsMtx[4]
		Mesh m_tableTop; //uses m_tableTopMtx
		Mesh m_tableSide; //uses m_tableSideMtx
		Mesh m_monitor; //uses m_monitorMtx
		Mesh m_screen; //uses m_monitorMtx
		Mesh m_duck;
		dx_ptr<ID3D11Buffer> m_vbParticles;

		DirectX::XMFLOAT4X4 m_projMtx, m_wallsMtx[6], m_sphereMtx, m_teapotMtx, m_boxMtx, m_lampMtx,
			m_chairMtx, m_tableLegsMtx[4], m_tableTopMtx, m_tableSideMtx, m_monitorMtx, m_duckMtx;

		dx_ptr<ID3D11SamplerState> m_samplerWrap;

		// TODO : 1.05 Add second sampler state field

		dx_ptr<ID3D11ShaderResourceView> m_wallTexture;
		dx_ptr<ID3D11ShaderResourceView> m_posterTexture;
		dx_ptr<ID3D11ShaderResourceView> m_perlinTexture;
		dx_ptr<ID3D11ShaderResourceView> m_woodTexture;
		dx_ptr<ID3D11ShaderResourceView> m_smokeTexture;
		dx_ptr<ID3D11ShaderResourceView> m_opacityTexture;

		dx_ptr<ID3D11RasterizerState> m_rsCullFront;
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		dx_ptr<ID3D11InputLayout> m_inputlayout, m_particleLayout;

		dx_ptr<ID3D11VertexShader> m_phongVS, m_textureVS, m_multiTexVS, m_particleVS;
		dx_ptr<ID3D11GeometryShader> m_particleGS;
		dx_ptr<ID3D11PixelShader> m_phongPS, m_texturePS, m_colorTexPS, m_multiTexPS, m_particlePS;

		EnvironmentMapper m_envMapper;

		ParticleSystem m_particles;

		void UpdateCameraCB(DirectX::XMMATRIX viewMtx);
		void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }
		void UpdateLamp(float dt);
		void UpdateParticles(float dt);

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawParticles();

		void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);
		void SetSurfaceColor(DirectX::XMFLOAT4 color);
		void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps);
		void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList, const dx_ptr<ID3D11SamplerState>& sampler);
		void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList) { SetTextures(std::move(resList), m_samplerWrap); }

		void DrawWalls();
		void DrawTeapot();
		void DrawTableElement(Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawTableLegs(DirectX::XMVECTOR camVec);
		void DrawTransparentObjects();
		void DrawScene();
	};
}