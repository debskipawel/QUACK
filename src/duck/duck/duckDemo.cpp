#include "duckDemo.h"

#include "DDSTextureLoader.h"

using namespace DirectX;

namespace mini::gk2
{
	DuckDemo::DuckDemo(HINSTANCE appInstance)
		: DxApplication(appInstance, 1280, 720, L"Kaczucha"),
		m_cbWorldMtx(m_device.CreateConstantBuffer<Matrix>()),
		m_cbProjMtx(m_device.CreateConstantBuffer<Matrix>()),
		m_cbViewMtx(m_device.CreateConstantBuffer<Matrix, 2>()),
		m_cbSurfaceColor(m_device.CreateConstantBuffer<Vector4>()),
		m_cbLightPos(m_device.CreateConstantBuffer<Vector4, 2>())
	{
		auto s = m_window.getClientSize();
		auto ar = static_cast<float>(s.cx) / s.cy;
		XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));

		auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
		auto psCode = m_device.LoadByteCode(L"phongPS.cso");
		m_phongVS = m_device.CreateVertexShader(vsCode);
		m_phongPS = m_device.CreatePixelShader(psCode);
		m_positionNormallayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

		vsCode = m_device.LoadByteCode(L"envMapVS.cso");
		psCode = m_device.LoadByteCode(L"envMapPS.cso");
		m_envVS = m_device.CreateVertexShader(vsCode);
		m_envPS = m_device.CreatePixelShader(psCode);

		m_duck = Mesh::LoadDuckMesh(m_device, L"../resources/mesh/duck.txt");
		m_duckMtx = Matrix::CreateScale(0.01f);

		m_box = Mesh::ShadedBox(m_device, -20.0f);

		ID3D11ShaderResourceView* cubeMap = nullptr;
		auto hr = CreateDDSTextureFromFile(m_device.get().get(), m_device.context().get(), L"../resources/textures/cubeMap.dds", nullptr, &cubeMap);

		m_cubeMap = dx_ptr<ID3D11ShaderResourceView>(cubeMap);

		m_samplerWrap = m_device.CreateSamplerState(SamplerDescription{});
	}

	void DuckDemo::Update(const Clock& c)
	{
		double dt = c.getFrameTime();
		HandleCameraInput(dt);
	}

	void DuckDemo::Render()
	{
		Base::Render();

		ResetRenderTarget();
		UpdateBuffer(m_cbProjMtx, m_projMtx);
		UpdateCameraCB();

		SetPhongShaders();
		DrawMesh(m_duck, m_duckMtx);
		
		SetCubeMapShaders();
		DrawMesh(m_box, Matrix::Identity);
	}

	void DuckDemo::SetPhongShaders()
	{
		UpdateBuffer(m_cbSurfaceColor, Vector4{ 1.0f, 1.0f, 1.0f, 0.0f });

		m_device.context()->IASetInputLayout(m_positionNormallayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb);
		ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightPos.get() };
		m_device.context()->PSSetConstantBuffers(0, 2, psb);

		SetShaders(m_phongVS, m_phongPS);
	}

	void DuckDemo::SetCubeMapShaders()
	{
		m_device.context()->IASetInputLayout(m_positionNormallayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11ShaderResourceView* views[] = { m_cubeMap.get() };
		ID3D11SamplerState* samplers[] = { m_samplerWrap.get() };
		m_device.context()->PSSetShaderResources(0, 1, views);
		m_device.context()->PSSetSamplers(0, 1, samplers);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb);
		m_device.context()->PSSetConstantBuffers(0, 0, nullptr);

		SetShaders(m_envVS, m_envPS);
	}
	
	void DuckDemo::UpdateCameraCB(Matrix viewMtx)
	{
		auto invViewMatrix = viewMtx.Invert();
		Matrix view[2] = { viewMtx, invViewMatrix };
		UpdateBuffer(m_cbViewMtx, view);
	}

	void DuckDemo::SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps)
	{
		m_device.context()->VSSetShader(vs.get(), nullptr, 0);
		m_device.context()->PSSetShader(ps.get(), nullptr, 0);
	}
	
	void DuckDemo::DrawMesh(const Mesh& m, Matrix worldMtx)
	{
		UpdateBuffer(m_cbWorldMtx, worldMtx);
		m.Render(m_device.context());
	}
}
