#include "duckDemo.h"

#include <random>
#include <array>
#include <algorithm>
#include <execution>

#include "DDSTextureLoader.h"

using namespace DirectX;

namespace mini::gk2
{
	constexpr int WATER_MESH_SIZE = 256;
	constexpr float WAVE_SPEED = 1.0f;
	constexpr float POINTS_DISTANCE = 2.0f / (WATER_MESH_SIZE - 1);
	constexpr float INTEGRAL_STEP = 1.0f / WATER_MESH_SIZE;

	DuckDemo::DuckDemo(HINSTANCE appInstance)
		: DxApplication(appInstance, 1280, 720, L"Kaczucha"),
		m_cbWorldMtx(m_device.CreateConstantBuffer<Matrix>()),
		m_cbProjMtx(m_device.CreateConstantBuffer<Matrix>()),
		m_cbViewMtx(m_device.CreateConstantBuffer<Matrix, 2>()),
		m_cbSurfaceColor(m_device.CreateConstantBuffer<Vector4>()),
		m_cbLightPos(m_device.CreateConstantBuffer<Vector4, 2>()),
		m_time(0.0f),
		m_heights(WATER_MESH_SIZE* WATER_MESH_SIZE),
		m_prevHeights(WATER_MESH_SIZE* WATER_MESH_SIZE),
		m_absorption(WATER_MESH_SIZE* WATER_MESH_SIZE),
		m_range(WATER_MESH_SIZE * WATER_MESH_SIZE),
		m_duckTexture(m_device.CreateShaderResourceView(L"../resources/textures/ducktex.png"))
	{
		for (int i = 0; i < WATER_MESH_SIZE * WATER_MESH_SIZE; i++)
		{
			int x = i % WATER_MESH_SIZE;
			int y = i / WATER_MESH_SIZE;

			int dy = min(y, WATER_MESH_SIZE - y - 1);
			int dx = min(y, WATER_MESH_SIZE - x - 1);

			float l = static_cast<float>(min(dx, dy)) / (WATER_MESH_SIZE - 1);

			m_absorption[i] = 0.95f * min(1.0f, l / 0.2f);
			m_range[i] = i;
		}

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

		vsCode = m_device.LoadByteCode(L"waterVS.cso");
		psCode = m_device.LoadByteCode(L"waterPS.cso");
		m_waterVS = m_device.CreateVertexShader(vsCode);
		m_waterPS = m_device.CreatePixelShader(psCode);

		vsCode = m_device.LoadByteCode(L"duckVS.cso");
		psCode = m_device.LoadByteCode(L"duckPS.cso");
		m_duckVS = m_device.CreateVertexShader(vsCode);
		m_duckPS = m_device.CreatePixelShader(psCode);
		m_positionNormalTexLayout = m_device.CreateInputLayout(VertexPositionNormalTex::Layout, vsCode);

		m_duck = Mesh::LoadDuckMesh(m_device, L"../resources/mesh/duck.txt");
		m_duckMtx = Matrix::CreateScale(0.01f);

		m_box = Mesh::ShadedBox(m_device, -20.0f);
		m_waterPlane = Mesh::Rectangle(m_device, 20.0f);

		ID3D11ShaderResourceView* cubeMap = nullptr;
		auto hr = CreateDDSTextureFromFile(m_device.get().get(), m_device.context().get(), L"../resources/textures/linus_cubemap.dds", nullptr, &cubeMap);

		m_cubeMap = dx_ptr<ID3D11ShaderResourceView>(cubeMap);

		m_samplerWrap = m_device.CreateSamplerState(SamplerDescription{});

		RasterizerDescription rs;
		rs.CullMode = D3D11_CULL_NONE;
		m_noCullRastState = m_device.CreateRasterizerState(rs);

		auto texDesc = D3D11_TEXTURE2D_DESC{};
		texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.ArraySize = 1;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.Height = texDesc.Width = WATER_MESH_SIZE;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		texDesc.SampleDesc.Count = 1;
		texDesc.MipLevels = 1;
		
		m_waterNormalTexture = m_device.CreateTexture(texDesc);
		m_waterNormalSrv = m_device.CreateShaderResourceView(m_waterNormalTexture);
	}

	void DuckDemo::Update(const Clock& c)
	{
		double dt = c.getFrameTime();
		m_time += dt;

		HandleCameraInput(dt);

		UpdateDuckPos();
		UpdateRaindrops();
		UpdateWaterNormals();
	}

	void DuckDemo::Render()
	{
		Base::Render();

		ResetRenderTarget();
		UpdateBuffer(m_cbProjMtx, m_projMtx);
		UpdateCameraCB();

		SetWaterShaders();
		DrawMesh(m_waterPlane, Matrix::Identity);

		SetDuckShaders();
		DrawMesh(m_duck, m_duckMtx);
		
		SetCubeMapShaders();
		DrawMesh(m_box, Matrix::Identity);
	}

	void DuckDemo::SetDuckShaders()
	{
		m_device.context()->IASetInputLayout(m_positionNormalTexLayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_device.context()->RSSetState(nullptr);

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

		m_device.context()->RSSetState(nullptr);

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

		m_device.context()->RSSetState(nullptr);

		ID3D11ShaderResourceView* views[] = { m_cubeMap.get() };
		ID3D11SamplerState* samplers[] = { m_samplerWrap.get() };
		m_device.context()->PSSetShaderResources(0, 1, views);
		m_device.context()->PSSetSamplers(0, 1, samplers);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb);
		m_device.context()->PSSetConstantBuffers(0, 0, nullptr);

		SetShaders(m_envVS, m_envPS);
	}

	void DuckDemo::SetWaterShaders()
	{
		m_device.context()->IASetInputLayout(m_positionNormalLayout.get());
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_device.context()->RSSetState(m_noCullRastState.get());

		ID3D11ShaderResourceView* views[] = { m_cubeMap.get(), m_waterNormalSrv.get() };
		ID3D11SamplerState* samplers[] = { m_samplerWrap.get() };
		m_device.context()->PSSetShaderResources(0, 2, views);
		m_device.context()->PSSetSamplers(0, 1, samplers);

		ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
		m_device.context()->VSSetConstantBuffers(0, 3, vsb);
		m_device.context()->PSSetConstantBuffers(0, 1, vsb + 1);

		SetShaders(m_waterVS, m_waterPS);
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
	
	void DuckDemo::UpdateRaindrops()
	{
		if (RandomDistribution(0.0f, 1.0f) < 0.005f)
		{
			int x = static_cast<int>(RandomDistribution(0, 255));
			int y = static_cast<int>(RandomDistribution(0, 255));

			m_heights[y * WATER_MESH_SIZE + x] += 0.25f;
		}
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

		float angle = atan2f(tangent.x, tangent.y);

		m_duckMtx = Matrix::CreateScale(0.01f) * Matrix::CreateRotationY(angle + XM_PIDIV2) * Matrix::CreateTranslation(pos);

		for (int i = 0; i < controlPoints.size(); i++)
		{
			m_duckCurveControlPoints.push(controlPoints[i]);
		}

		// water disturbance
		Vector3 point = (pos / 20.0f + Vector3{0.5f, 0.0f, 0.5f}) * WATER_MESH_SIZE;
		int x = point.x;
		int y = point.z;

		m_heights[y * WATER_MESH_SIZE + x] += 0.25f;
	}
	
	void DuckDemo::UpdateWaterNormals()
	{
		float A = powf(WAVE_SPEED * INTEGRAL_STEP / POINTS_DISTANCE, 2.0f);
		float B = 2.0f - 4 * A;

		auto prevPrevHeights = m_prevHeights;
		m_prevHeights = m_heights;

		for (int y = 1; y < WATER_MESH_SIZE - 1; y++)
		{
			for (int x = 1; x < WATER_MESH_SIZE - 1; x++)
			{
				float d = m_absorption[y * WATER_MESH_SIZE + x];
				float prevValue = prevPrevHeights[y * WATER_MESH_SIZE + x];
				
				float sum = m_prevHeights[(y + 1) * WATER_MESH_SIZE + x] + m_prevHeights[(y - 1) * WATER_MESH_SIZE + x]
					+ m_prevHeights[y * WATER_MESH_SIZE + x + 1] + m_prevHeights[y * WATER_MESH_SIZE + x - 1];

				m_heights[y * WATER_MESH_SIZE + x] = d * (A * sum + B * m_prevHeights[y * WATER_MESH_SIZE + x] - prevValue);
			}
		}

		std::vector<unsigned char> vectors(WATER_MESH_SIZE * WATER_MESH_SIZE * 4);

		for (int x = 0; x < WATER_MESH_SIZE - 1; x++)
		{
			for (int y = 0; y < WATER_MESH_SIZE - 1; y++)
			{
				float height = m_heights[y * WATER_MESH_SIZE + x];
				float hNeighbourX = m_heights[y * WATER_MESH_SIZE + x + 1];
				float hNeighbourY = m_heights[(y + 1) * WATER_MESH_SIZE + x];

				auto dx = Vector3{ POINTS_DISTANCE, hNeighbourX - height, 0.0f };
				auto dz = Vector3{ 0.0f, hNeighbourX - height, POINTS_DISTANCE };

				auto normal = dx.Cross(dz);
				normal.Normalize();

				normal = normal.y < 0.0 ? -normal : normal;

				vectors[4 * (WATER_MESH_SIZE * y + x)] = (normal.x + 1.0) / 2.0f * 255;
				vectors[4 * (WATER_MESH_SIZE * y + x) + 1] = normal.y * 255;
				vectors[4 * (WATER_MESH_SIZE * y + x) + 2] = (normal.z + 1.0) / 2.0f * 255;
				vectors[4 * (WATER_MESH_SIZE * y + x) + 3] = 255;
			}
		}

		D3D11_MAPPED_SUBRESOURCE res;
		m_device.context()->Map(m_waterNormalTexture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

		if (res.RowPitch == WATER_MESH_SIZE * 4 * sizeof(unsigned char))
		{
			memcpy(res.pData, vectors.data(), vectors.size() * sizeof(unsigned char));
		}
		else
		{
			for (int i = 0; i < res.DepthPitch / res.RowPitch; i++)
			{
				memcpy((unsigned char*)res.pData + i * res.RowPitch, static_cast<unsigned char*>(vectors.data()) + i * WATER_MESH_SIZE * 4, WATER_MESH_SIZE * 4 * sizeof(unsigned char));
			}
		}

		m_device.context()->Unmap(m_waterNormalTexture.get(), 0);
	}
}
