#include "duckDemo.h"

using namespace DirectX;

namespace mini::gk2
{
	DuckDemo::DuckDemo(HINSTANCE appInstance)
		: DxApplication(appInstance, 1280, 720, L"Kaczucha"),
		m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
		m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
		m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
		m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
		m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4, 2>())
	{
		auto s = m_window.getClientSize();
		auto ar = static_cast<float>(s.cx) / s.cy;
		XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));

		auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
		auto psCode = m_device.LoadByteCode(L"phongPS.cso");
		m_phongVS = m_device.CreateVertexShader(vsCode);
		m_phongPS = m_device.CreatePixelShader(psCode);
		m_phongInputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

		m_duck = Mesh::LoadDuckMesh(m_device, L"../resources/mesh/duck.txt");
		XMStoreFloat4x4(&m_duckMtx, XMMatrixScaling(0.005f, 0.005f, 0.005f));

		UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.0f });

		m_device.context()->IASetInputLayout(m_phongInputlayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb); //Vertex Shaders - 0: worldMtx, 1: viewMtx,invViewMtx, 2: projMtx, 3: tex1Mtx, 4: tex2Mtx
		ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightPos.get() };
		m_device.context()->PSSetConstantBuffers(0, 2, psb); //Pixel Shaders - 0: surfaceColor, 1: lightPos[2]
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

		SetShaders(m_phongVS, m_phongPS);
		DrawMesh(m_duck, m_duckMtx);
	}
	
	void DuckDemo::UpdateCameraCB(DirectX::XMMATRIX viewMtx)
	{
		XMVECTOR det;
		XMMATRIX invViewMtx = XMMatrixInverse(&det, viewMtx);
		XMFLOAT4X4 view[2];
		XMStoreFloat4x4(view, viewMtx);
		XMStoreFloat4x4(view + 1, invViewMtx);
		UpdateBuffer(m_cbViewMtx, view);
	}

	void DuckDemo::SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps)
	{
		m_device.context()->VSSetShader(vs.get(), nullptr, 0);
		m_device.context()->PSSetShader(ps.get(), nullptr, 0);
	}
	
	void DuckDemo::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
	{
		UpdateBuffer(m_cbWorldMtx, worldMtx);
		m.Render(m_device.context());
	}
}
