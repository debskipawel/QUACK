#include "duckDemo.h"

#include <random>
#include <array>

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
		m_cbLightPos(m_device.CreateConstantBuffer<Vector4, 2>()),
		m_time(0.0f),
		m_duckTexture(m_device.CreateShaderResourceView(L"../resources/textures/ducktex.png"))
	{
		auto s = m_window.getClientSize();
		auto ar = static_cast<float>(s.cx) / s.cy;
		XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));

		auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
		auto psCode = m_device.LoadByteCode(L"phongPS.cso");
		m_phongVS = m_device.CreateVertexShader(vsCode);
		m_phongPS = m_device.CreatePixelShader(psCode);
		m_positionNormalLayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

		vsCode = m_device.LoadByteCode(L"envMapVS.cso");
		psCode = m_device.LoadByteCode(L"envMapPS.cso");
		m_envVS = m_device.CreateVertexShader(vsCode);
		m_envPS = m_device.CreatePixelShader(psCode);

		vsCode = m_device.LoadByteCode(L"duckVS.cso");
		psCode = m_device.LoadByteCode(L"duckPS.cso");
		m_duckVS = m_device.CreateVertexShader(vsCode);
		m_duckPS = m_device.CreatePixelShader(psCode);
		m_positionNormalTexLayout = m_device.CreateInputLayout(VertexPositionNormalTex::Layout, vsCode);

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
		m_time += dt;

		HandleCameraInput(dt);

		UpdateDuckPos();
	}

	void DuckDemo::Render()
	{
		Base::Render();

		ResetRenderTarget();
		UpdateBuffer(m_cbProjMtx, m_projMtx);
		UpdateCameraCB();

		SetDuckShaders();
		DrawMesh(m_duck, m_duckMtx);
		
		SetCubeMapShaders();
		DrawMesh(m_box, Matrix::Identity);
	}

	void DuckDemo::SetDuckShaders()
	{
		m_device.context()->IASetInputLayout(m_positionNormalTexLayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb);

		ID3D11Buffer* psb[] = { m_cbLightPos.get() };
		m_device.context()->PSSetConstantBuffers(0, 1, psb);

		ID3D11ShaderResourceView* views[] = { m_duckTexture.get() };
		ID3D11SamplerState* samplers[] = { m_samplerWrap.get() };
		m_device.context()->PSSetShaderResources(0, 1, views);
		m_device.context()->PSSetSamplers(0, 1, samplers);

		SetShaders(m_duckVS, m_duckPS);
	}

	void DuckDemo::SetPhongShaders()
	{
		UpdateBuffer(m_cbSurfaceColor, Vector4{ 1.0f, 1.0f, 1.0f, 0.0f });

		m_device.context()->IASetInputLayout(m_positionNormalLayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb);
		ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightPos.get() };
		m_device.context()->PSSetConstantBuffers(0, 2, psb);

		SetShaders(m_phongVS, m_phongPS);
	}

	void DuckDemo::SetCubeMapShaders()
	{
		m_device.context()->IASetInputLayout(m_positionNormalLayout.get());
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

	std::array<float, 4> DeBoor(float t, int degree = 3)
	{
		std::array<float, 4> result{ 0.0f };
		result[0] = 1.0f;

		// knots are 0, 1, 2, 3 - t is always from 0 to 1
		for (int j = 1; j <= degree; j++)
		{
			for (int i = j; i >= 1; i--)
			{
				float left = i - j;
				float right = i + 1;
				result[i] = result[i] * ((right - t) / j) + result[i - 1] * ((t - left) / j);
			}

			result[0] = result[0] * (1.0f - t) / j;
		}

		return result;
	}

	float RandomDistribution(float min, float max)
	{
		static std::random_device rd;
		static std::mt19937 e2(rd());
		static std::uniform_real_distribution<> dist(0, 1);

		float next = dist(e2);

		return next * (max - min) + min;
	}
	
	void DuckDemo::UpdateDuckPos()
	{
		while (m_time > DUCK_PERIOD)
		{
			if (!m_duckCurveControlPoints.empty())
			{
				m_duckCurveControlPoints.pop();
			}

			m_time -= DUCK_PERIOD;
		}

		while (m_duckCurveControlPoints.size() < 4)
		{
			m_duckCurveControlPoints.push(Vector2{ RandomDistribution(-10, 10), RandomDistribution(-10, 10) });
		}

		float parameter = m_time / DUCK_PERIOD;

		std::array<Vector2, 4> controlPoints;

		for (int i = 0; i < controlPoints.size(); i++)
		{
			controlPoints[i] = m_duckCurveControlPoints.front();
			m_duckCurveControlPoints.pop();
		}

		// calculate position and tangent to the curve based on control points
		auto values = DeBoor(parameter, 3);
		auto derivatives = DeBoor(parameter, 2);

		Vector2 position = {};
		Vector2 tangent = {};

		// calculate position on curve
		for (int i = 0; i < controlPoints.size(); i++)
		{
			position += values[i] * controlPoints[i];
		}

		// calculate tangent to the curve in point
		for (int i = 0; i < controlPoints.size() - 1; i++)
		{
			auto Q = controlPoints[i + 1] - controlPoints[i];
			tangent += derivatives[i] * Q;
		}

		tangent.Normalize();

		Vector3 pos = { position.x, 0.0f, position.y };
		Vector3 target = pos - Vector3{ tangent.x, 0.0f, tangent.y };

		auto transform = Matrix::CreateLookAt(pos, target, Vector3{ 0.0f, 1.0f, 0.0f });
		transform.Invert();

		m_duckMtx = Matrix::CreateScale(0.01f) * transform;

		for (int i = 0; i < controlPoints.size(); i++)
		{
			m_duckCurveControlPoints.push(controlPoints[i]);
		}
	}
}
