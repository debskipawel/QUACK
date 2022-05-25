#include "spriteRenderer.h"
#include "spriteVS.h"
#include "spritePS.h"
#include "inputLayoutManager.h"
#include "dxDevice.h"

using namespace DirectX;
using namespace mini;
using namespace utils;
using namespace std;

SpriteRenderer::SpriteEffect::SpriteEffect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps,
	dx_ptr<ID3D11SamplerState>&& sampler, const ConstantBuffer<XMFLOAT4X4> cbSpriteTransform,
	const ConstantBuffer<XMFLOAT4X4>& cbTexTransform)
	: StaticEffect(BasicEffect(move(vs), move(ps)), VSConstantBuffers{ cbSpriteTransform, cbTexTransform }, PSSamplers{ sampler.get() })
{
}

static RasterizerDescription SpriteRSState()
{
	RasterizerDescription result;
	result.CullMode = D3D11_CULL_NONE;
	result.DepthClipEnable = false;
	return result;
}

static SamplerDescription SpriteSamplerState()
{
	SamplerDescription result;
	result.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	result.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	return result;
}

static DepthStencilDescription SpriteDepthStencilState()
{
	DepthStencilDescription result;
	result.DepthEnable = false;
	result.DepthFunc = D3D11_COMPARISON_ALWAYS;
	result.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	return result;
}

SpriteRenderer::SpriteRenderer(const DxDevice& device, InputLayoutManager& lm)
	: m_cbSpriteTransform(device.CreateConstantBuffer<XMFLOAT4X4>()), m_cbTextureTansform(device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_rsState(device.CreateRasterizerState(SpriteRSState())),
	m_bsState(device.CreateBlendState()),
	m_dsState(device.CreateDepthStencilState(SpriteDepthStencilState())),
	m_effect(device.CreateVertexShader(SPRITE_VS_BYTE_CODE), device.CreatePixelShader(SPRITE_PS_BYTE_CODE),
		device.CreateSamplerState(SpriteSamplerState()), m_cbSpriteTransform, m_cbTextureTansform)
{
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	m_cbTextureTansform.Update(device.context(), identity);

	D3D11_INPUT_ELEMENT_DESC elem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	auto attribID = lm.registerVertexAttributesID(elem);
	auto signID = lm.registerSignatureID(
		vector<BYTE>{SPRITE_VS_BYTE_CODE, SPRITE_VS_BYTE_CODE + sizeof(SPRITE_VS_BYTE_CODE)});
	m_layout.reset(lm.getLayout(attribID, signID).get());
	m_layout->AddRef();

#pragma region Create quad mesh
	vector<XMFLOAT2> positions(4);
	positions[0] = XMFLOAT2(-1.0f, 1.0f);
	positions[1] = XMFLOAT2(1.0f, 1.0f);
	positions[2] = XMFLOAT2(1.0f, -1.0f);
	positions[3] = XMFLOAT2(-1.0f, -1.0f);
	vector<XMFLOAT2> texCoords(4);
	texCoords[0] = XMFLOAT2(0.0f, 0.0f);
	texCoords[1] = XMFLOAT2(1.0f, 0.0f);
	texCoords[2] = XMFLOAT2(1.0f, 1.0f);
	texCoords[3] = XMFLOAT2(0.0f, 1.0f);
	vector<unsigned short> indices(6);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;
	/*vector<dx_ptr<ID3D11Buffer>> vb;
	vector<unsigned int> vs;
	vector<unsigned int> vo;
	vb.reserve(2);
	vs.reserve(2);
	vo.reserve(2);
	vb.push_back(device.CreateVertexBuffer(positions));
	vs.push_back(sizeof(XMFLOAT2));
	vo.push_back(0);
	vb.push_back(device.CreateVertexBuffer(texCoords));
	vs.push_back(sizeof(XMFLOAT2));
	vo.push_back(0);*/
	m_quad = device.CreateMesh(indices, positions, texCoords);// Mesh(move(vb), move(vs), move(vo), device.CreateIndexBuffer(indices), 6, 0);
#pragma endregion
}

void SpriteRenderer::DrawSprite(const dx_ptr<ID3D11DeviceContext>& context,
	const dx_ptr<ID3D11ShaderResourceView>& spriteTexture, XMFLOAT2 spritePosition, XMFLOAT2 spriteSize)
{
	XMFLOAT4X4 spriteMtx;
	XMStoreFloat4x4(&spriteMtx, XMMatrixScaling(spriteSize.x, spriteSize.y, 1.0f) * 
		XMMatrixTranslation(spritePosition.x, spritePosition.y, 0.0f));
	m_cbSpriteTransform.Update(context, spriteMtx);

#pragma region Push states
	ID3D11RasterizerState* rsState;
	dx_ptr<ID3D11RasterizerState> oldRSState;
	context->RSGetState(&rsState);
	oldRSState.reset(rsState);

	ID3D11DepthStencilState* dsState;
	dx_ptr<ID3D11DepthStencilState> oldDSState;
	UINT oldDSStencilRef;
	context->OMGetDepthStencilState(&dsState, &oldDSStencilRef);
	oldDSState.reset(dsState);

	ID3D11BlendState* bsState;
	dx_ptr<ID3D11BlendState> oldBSState;
	FLOAT oldBSBlendFactors[4];
	UINT oldBSSampleMask;
	context->OMGetBlendState(&bsState, oldBSBlendFactors, &oldBSSampleMask);
	oldBSState.reset(bsState);

	ID3D11InputLayout* ilState;
	dx_ptr<ID3D11InputLayout> oldILState;
	context->IAGetInputLayout(&ilState);
	context->IASetInputLayout(m_layout.get());

#pragma endregion
	m_effect.Begin(context);
	ID3D11ShaderResourceView* srv[1] = { spriteTexture.get() };
	context->PSSetShaderResources(0, 1, srv);

	context->RSSetState(m_rsState.get());
	context->OMSetDepthStencilState(m_dsState.get(), 0);
	FLOAT factors[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_bsState.get(), factors, UINT_MAX);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_quad.Render(context);

#pragma region Pop states
	context->IASetInputLayout(oldILState.get());
	context->RSSetState(oldRSState.get());
	context->OMSetDepthStencilState(oldDSState.get(), oldDSStencilRef);
	context->OMSetBlendState(oldBSState.get(), oldBSBlendFactors, oldBSSampleMask);
#pragma endregion
}
