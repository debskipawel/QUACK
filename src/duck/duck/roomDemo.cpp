#include "roomDemo.h"
#include <array>
#include "mesh.h"
#include "textureGenerator.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

const XMFLOAT3 RoomDemo::TEAPOT_POS{ -1.3f, -0.74f, -0.6f };
const XMFLOAT4 RoomDemo::TABLE_POS{ 0.5f, -0.96f, 0.5f, 1.0f };
const XMFLOAT4 RoomDemo::LIGHT_POS[2] = { {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f, 1.0f} };

RoomDemo::RoomDemo(HINSTANCE appInstance)
	: DxApplication(appInstance, 1280, 720, L"Pokój"), 
	//Constant Buffers
	m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()), m_cbTex1Mtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbTex2Mtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4, 2>()),
	//Textures
	m_wallTexture(m_device.CreateShaderResourceView(L"resources/textures/brick_wall.jpg")),
	m_posterTexture(m_device.CreateShaderResourceView(L"resources/textures/lautrec_divan.jpg")),
	m_perlinTexture(m_device.CreateShaderResourceView(L"resources/textures/perlin.jpg")),
	m_smokeTexture(m_device.CreateShaderResourceView(L"resources/textures/smoke.png")),
	m_opacityTexture(m_device.CreateShaderResourceView(L"resources/textures/smokecolors.png")),
	//EnvMapper
	m_envMapper{ m_device, MAPPER_NEAR, MAPPER_FAR, TEAPOT_POS },
	//Particles
	m_particles{ {-1.3f, -0.6f, -0.14f} }
{
	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	UpdateBuffer(m_cbProjMtx, m_projMtx);
	UpdateCameraCB();

	//Sampler States
	SamplerDescription sd;
	// TODO : 0.01 Set to proper addressing (wrap) and filtering (16x anisotropic) modes of the sampler

	m_samplerWrap = m_device.CreateSamplerState(sd);
	// TODO : 1.06 Initialize second sampler state

	// TODO : 1.10 Moddify MipLODBias field for the second sampler


	//Wood texture
	constexpr auto woodTexWidth = 64U;
	constexpr auto woodTexHeight = 64U;
	constexpr auto woodTexBpp = 4U;
	constexpr auto woodTexStride = woodTexWidth*woodTexBpp;
	constexpr auto woodTexSize = woodTexStride*woodTexHeight;

	auto texDesc = Texture2DDescription(woodTexWidth, woodTexHeight);
	texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	auto woodTex = m_device.CreateTexture(texDesc);
	m_woodTexture = m_device.CreateShaderResourceView(woodTex);

	array<BYTE, woodTexSize> data;
	auto d = data.data();
	TextureGenerator txGen(6, 0.35f);
	for (auto i = 0; i < woodTexHeight; ++i)
	{
		auto x = i / static_cast<float>(woodTexHeight);
		for (auto j = 0; j < woodTexWidth; ++j)
		{
			auto y = j / static_cast<float>(woodTexWidth);
			auto c = txGen.Wood(x, y);
			auto ic = static_cast<BYTE>(c * 239);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 200);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 139);
			*(d++) = ic;
			*(d++) = 255;
		}
	}
	m_device.context()->UpdateSubresource(woodTex.get(), 0, nullptr, data.data(), woodTexStride, woodTexSize);
	m_device.context()->GenerateMips(m_woodTexture.get());

	//Meshes
	vector<VertexPositionNormal> vertices;
	vector<unsigned short> indices;
	m_wall = Mesh::Rectangle(m_device, 4.0f);
	m_teapot = Mesh::LoadMesh(m_device, L"resources/meshes/teapot.mesh");
	m_sphere = Mesh::Sphere(m_device, 8, 16, 0.3f);
	m_box = Mesh::ShadedBox(m_device);
	m_lamp = Mesh::LoadMesh(m_device, L"resources/meshes/lamp.mesh");
	m_chairSeat = Mesh::LoadMesh(m_device, L"resources/meshes/chair_seat.mesh");
	m_chairBack = Mesh::LoadMesh(m_device, L"resources/meshes/chair_back.mesh");
	m_monitor = Mesh::LoadMesh(m_device, L"resources/meshes/monitor.mesh");
	m_screen = Mesh::LoadMesh(m_device, L"resources/meshes/screen.mesh");
	m_tableLeg = Mesh::Cylinder(m_device, 4, 9, TABLE_H - TABLE_TOP_H, 0.1f);
	m_tableSide = Mesh::Cylinder(m_device, 1, 16, TABLE_TOP_H, TABLE_R);
	m_tableTop = Mesh::Disk(m_device, 16, TABLE_R);
	m_duck = Mesh::LoadDuckMesh(m_device, L"../resources/mesh/duck.txt");
	XMStoreFloat4x4(&m_duckMtx, XMMatrixScaling(0.005f, 0.005f, 0.005f));

	m_vbParticles = m_device.CreateVertexBuffer<ParticleVertex>(ParticleSystem::MAX_PARTICLES);

	//World matrix of all objects
	auto temp = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	auto a = 0.f;
	for (auto i = 0U; i < 4U; ++i, a += XM_PIDIV2)
		XMStoreFloat4x4(&m_wallsMtx[i], temp * XMMatrixRotationY(a));
	XMStoreFloat4x4(&m_wallsMtx[4], temp * XMMatrixRotationX(XM_PIDIV2));
	XMStoreFloat4x4(&m_wallsMtx[5], temp * XMMatrixRotationX(-XM_PIDIV2));
	XMStoreFloat4x4(&m_teapotMtx, XMMatrixTranslation(0.0f, -2.3f, 0.f) * XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(-1.3f, -0.74f, -0.6f));
	
	XMStoreFloat4x4(&m_sphereMtx, XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(TEAPOT_POS.x, TEAPOT_POS.y, TEAPOT_POS.z));
	XMStoreFloat4x4(&m_boxMtx, XMMatrixTranslation(-1.4f, -1.46f, -0.6f));
	XMStoreFloat4x4(&m_chairMtx, XMMatrixRotationY(XM_PI + XM_PI / 9 ) *
		XMMatrixTranslation(-0.1f, -1.06f, -1.3f));
	XMStoreFloat4x4(&m_monitorMtx, XMMatrixRotationY(XM_PIDIV4) *
		XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y + 0.42f, TABLE_POS.z));
	a = XM_PIDIV4;
	for (auto i = 0U; i < 4U; ++i, a += XM_PIDIV2)
		XMStoreFloat4x4(&m_tableLegsMtx[i], XMMatrixTranslation(0.0f, 0.0f, TABLE_R - 0.35f) * XMMatrixRotationY(a) *
			XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y - (TABLE_H + TABLE_TOP_H) / 2, TABLE_POS.z));
	XMStoreFloat4x4(&m_tableSideMtx, XMMatrixRotationY(XM_PIDIV4 / 4) *
		XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y - TABLE_TOP_H / 2, TABLE_POS.z));
	XMStoreFloat4x4(&m_tableTopMtx, XMMatrixRotationY(XM_PIDIV4 / 4) *
		XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y, TABLE_POS.z));

	//Constant buffers content
	UpdateBuffer(m_cbLightPos, LIGHT_POS);
	XMFLOAT4X4 tempMtx;

	// TODO : 1.08 Calculate correct transformation matrix for the poster texture
	XMStoreFloat4x4(&tempMtx, XMMatrixIdentity());
	
	UpdateBuffer(m_cbTex2Mtx, tempMtx);

	//Render states
	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	m_rsCullFront = m_device.CreateRasterizerState(rsDesc);

	m_bsAlpha = m_device.CreateBlendState(BlendDescription::AlphaBlendDescription());
	DepthStencilDescription dssDesc;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);

	auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
	auto psCode = m_device.LoadByteCode(L"phongPS.cso");
	m_phongVS = m_device.CreateVertexShader(vsCode);
	m_phongPS = m_device.CreatePixelShader(psCode);
	m_inputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	vsCode = m_device.LoadByteCode(L"texturedVS.cso");
	psCode = m_device.LoadByteCode(L"texturedPS.cso");
	m_textureVS = m_device.CreateVertexShader(vsCode);
	m_texturePS = m_device.CreatePixelShader(psCode);
	psCode = m_device.LoadByteCode(L"colorTexPS.cso");
	m_colorTexPS = m_device.CreatePixelShader(psCode);

	vsCode = m_device.LoadByteCode(L"multiTexVS.cso");
	psCode = m_device.LoadByteCode(L"multiTexPS.cso");
	m_multiTexVS = m_device.CreateVertexShader(vsCode);
	m_multiTexPS = m_device.CreatePixelShader(psCode);

	vsCode = m_device.LoadByteCode(L"particleVS.cso");
	psCode = m_device.LoadByteCode(L"particlePS.cso");
	auto gsCode = m_device.LoadByteCode(L"particleGS.cso");
	m_particleVS = m_device.CreateVertexShader(vsCode);
	m_particlePS = m_device.CreatePixelShader(psCode);
	m_particleGS = m_device.CreateGeometryShader(gsCode);
	m_particleLayout = m_device.CreateInputLayout<ParticleVertex>(vsCode);

	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UpdateLamp(0.0f);

	//We have to make sure all shaders use constant buffers in the same slots!
	//Not all slots will be use by each shader
	ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get(), m_cbTex1Mtx.get(), m_cbTex2Mtx.get() };
	m_device.context()->VSSetConstantBuffers(0, 5, vsb); //Vertex Shaders - 0: worldMtx, 1: viewMtx,invViewMtx, 2: projMtx, 3: tex1Mtx, 4: tex2Mtx
	m_device.context()->GSSetConstantBuffers(0, 1, vsb + 2); //Geometry Shaders - 0: projMtx
	ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightPos.get() };
	m_device.context()->PSSetConstantBuffers(0, 2, psb); //Pixel Shaders - 0: surfaceColor, 1: lightPos[2]
}

void RoomDemo::UpdateCameraCB(XMMATRIX viewMtx)
{
	XMVECTOR det;
	XMMATRIX invViewMtx = XMMatrixInverse(&det, viewMtx);
	XMFLOAT4X4 view[2];
	XMStoreFloat4x4(view, viewMtx);
	XMStoreFloat4x4(view + 1, invViewMtx);
	UpdateBuffer(m_cbViewMtx, view);
}

void RoomDemo::UpdateLamp(float dt)
{
	static auto time = 0.0f;
	time += dt;
	auto swing = 0.3f * XMScalarSin(XM_2PI*time / 8);
	auto rot = XM_2PI*time / 20;
	auto lamp = XMMatrixTranslation(0.0f, -0.4f, 0.0f) * XMMatrixRotationX(swing) * XMMatrixRotationY(rot) *
		XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&m_lampMtx, lamp);
}

void mini::gk2::RoomDemo::UpdateParticles(float dt)
{
	// TODO : 1.31 update particle system and copy vertex data to the buffer
}

void RoomDemo::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	HandleCameraInput(dt);
	UpdateLamp(static_cast<float>(dt));
	UpdateParticles(dt);
}

void RoomDemo::SetWorldMtx(DirectX::XMFLOAT4X4 mtx)
{
	UpdateBuffer(m_cbWorldMtx, mtx);
}

void RoomDemo::SetSurfaceColor(DirectX::XMFLOAT4 color)
{
	UpdateBuffer(m_cbSurfaceColor, color);
}

void mini::gk2::RoomDemo::SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps)
{
	m_device.context()->VSSetShader(vs.get(), nullptr, 0);
	m_device.context()->PSSetShader(ps.get(), nullptr, 0);
}

void mini::gk2::RoomDemo::SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList, const dx_ptr<ID3D11SamplerState>& sampler)
{
	m_device.context()->PSSetShaderResources(0, resList.size(), resList.begin());
	auto s_ptr = sampler.get();
	m_device.context()->PSSetSamplers(0, 1, &s_ptr);
}

void RoomDemo::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
	SetWorldMtx(worldMtx);
	m.Render(m_device.context());
}

void RoomDemo::DrawParticles()
{
	if (m_particles.particlesCount() == 0)
		return;
	//Set input layout, primitive topology, shaders, vertex buffer, and draw particles
	SetTextures({ m_smokeTexture.get(), m_opacityTexture.get() });
	m_device.context()->IASetInputLayout(m_particleLayout.get());
	SetShaders(m_particleVS, m_particlePS);
	m_device.context()->GSSetShader(m_particleGS.get(), nullptr, 0);
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	unsigned int stride = sizeof(ParticleVertex);
	unsigned int offset = 0;
	auto vb = m_vbParticles.get();
	m_device.context()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	m_device.context()->Draw(m_particles.particlesCount(), 0);

	//Reset layout, primitive topology and geometry shader
	m_device.context()->GSSetShader(nullptr, nullptr, 0);
	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RoomDemo::DrawWalls()
{
	//draw ceiling
	XMFLOAT4X4 texMtx;
	SetSurfaceColor(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));

	// TODO : 0.02 calculate texture tranformation matrix for walls/ceiling/floor that transforms rectangle [-2,-2]x[2,2] into [0,0]x[1,1]
	//XMStoreFloat4x4(&texMtx, XMMatrixIdentity());
	XMStoreFloat4x4(&texMtx, XMMatrixIdentity());
	UpdateBuffer(m_cbTex1Mtx, texMtx);


	// TODO : 0.03 set shaders to m_textureVS, m_texturePS
	// TODO : 0.11 draw ceiling with colorTexture pixel shaders instead
	SetShaders(m_phongVS, m_phongPS);

	// TODO : 0.04 set ceiling the texture to perlin noise


	DrawMesh(m_wall, m_wallsMtx[5]);

	//draw back wall
	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	
	// TODO : 1.04 Draw back wall with muliti-texture shaders, setting textures to wall and poster.
	// TODO : 1.07 Pass the new sampler state along with wall and poster textures

	// TODO : 0.05 set the walls' texture to brick wall 

	// TODO : 0.12 draw back walls with regular texture shaders

	DrawMesh(m_wall, m_wallsMtx[0]);

	//draw remainting walls
	// TODO : 0.13 draw remaining walls with regular texture shaders
	for (auto i = 1; i < 4; ++i)
		DrawMesh(m_wall, m_wallsMtx[i]);

	//draw floor
	// TODO : 0.09 Change texture matrix to stretch floor texture 8 times along y, i.e. transform rectangle [-2,-2]x[2,2] into [0,0]x[1,16]. Run the program again to see the difference.

	// TODO : 0.06 set floor texture to wood

	DrawMesh(m_wall, m_wallsMtx[4]);
}

void RoomDemo::DrawTeapot()
{
	// TODO : 1.25 Comment the following line and begin m_envMapper shaders instead
	SetShaders(m_phongVS, m_phongPS);

	SetSurfaceColor(XMFLOAT4(0.8f, 0.7f, 0.65f, 1.0f));

	// TODO : 1.26 [optional] Comment the following line and uncomment the next to replace teapot with a sphere
	DrawMesh(m_teapot, m_teapotMtx);
	//DrawMesh(m_sphere, m_sphereMtx);

	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void RoomDemo::DrawTableElement(Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
	SetWorldMtx(worldMtx);
	m_device.context()->RSSetState(m_rsCullFront.get());
	m.Render(m_device.context());
	m_device.context()->RSSetState(nullptr);
	m.Render(m_device.context());
}

void RoomDemo::DrawTableLegs(XMVECTOR camVec)
{
	XMFLOAT4 v(1.0f, 0.0f, 0.0f, 0.0f);
	auto plane1 = XMLoadFloat4(&v);
	v = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	auto plane2 = XMLoadFloat4(&v);
	auto left = XMVector3Dot(camVec, plane1).m128_f32[0] > 0;
	auto back = XMVector3Dot(camVec, plane2).m128_f32[0] > 0;
	if (left)
	{
		if (back)
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
		}
		else
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
		}
	}
	else
	{

		if (back)
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
		}
		else
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
		}
	}
}

void RoomDemo::DrawTransparentObjects()
{
	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	SetSurfaceColor(XMFLOAT4(0.1f, 0.1f, 0.1f, 0.9f));
	auto v = m_camera.getCameraPosition();
	auto camVec = XMVector3Normalize(XMLoadFloat4(&v) - XMLoadFloat4(&TABLE_POS));
	v = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	auto plane = XMLoadFloat4(&v);
	if (XMVector3Dot(camVec, plane).m128_f32[0] > 0)
	{
		SetShaders(m_phongVS, m_phongPS);
		m_device.context()->RSSetState(m_rsCullFront.get());
		DrawMesh(m_tableSide, m_tableSideMtx);
		m_device.context()->RSSetState(nullptr);
		DrawTableLegs(camVec);
		DrawMesh(m_tableSide, m_tableSideMtx);
		DrawTableElement(m_tableTop, m_tableTopMtx);
		DrawParticles();
	}
	else
	{
		DrawParticles();
		SetShaders(m_phongVS, m_phongPS);
		DrawTableElement(m_tableTop, m_tableTopMtx);
		m_device.context()->RSSetState(m_rsCullFront.get());
		DrawMesh(m_tableSide, m_tableSideMtx);
		m_device.context()->RSSetState(nullptr);
		DrawTableLegs(camVec);
		DrawMesh(m_tableSide, m_tableSideMtx);
	}
	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void RoomDemo::DrawScene()
{
	DrawWalls();
	DrawTeapot();
	SetShaders(m_phongVS, m_phongPS);

	//Draw shelf
	DrawMesh(m_box, m_boxMtx);
	//Draw lamp
	DrawMesh(m_lamp, m_lampMtx);
	//Draw chair seat
	DrawMesh(m_chairSeat, m_chairMtx);
	//Draw chairframe
	DrawMesh(m_chairBack, m_chairMtx);
	//Draw monitor
	DrawMesh(m_monitor, m_monitorMtx);
	//Draw screen
	DrawMesh(m_screen, m_monitorMtx);

	DrawMesh(m_duck, m_duckMtx);
	DrawTransparentObjects();
}

void RoomDemo::Render()
{
	Base::Render();
	// TODO : 1.20 Change projection matrix to for drawing in environment cube

	// TODO : 1.21 Set evnMapper render target

	// TODO : 1.22 For each cube face: clear envMapper render target, update camera constant buffer, draw the scene and copy the result to cubemap

	ResetRenderTarget();
	UpdateBuffer(m_cbProjMtx, m_projMtx);
	UpdateCameraCB();

	DrawScene();
}