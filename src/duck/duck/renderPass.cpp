#include "renderPass.h"
#include "dxDevice.h"
#include <D3DCompiler.h>
#include "spriteRenderer.h"
#include "inputLayoutManager.h"

using namespace std;
using namespace DirectX;
using namespace mini;
using namespace gk2;

void RenderPass::_initShaders(const DxDevice& device, const CBVariableManager& variables,
	const wstring& vsShader, const wstring& psShader)
{
	const auto vsCode = DxDevice::LoadByteCode(vsShader);
	m_vsSignatureID = m_layouts->registerSignatureID(vsCode);
	auto vs = device.CreateVertexShader(vsCode);
	const auto psCode = DxDevice::LoadByteCode(psShader);
	auto ps = device.CreatePixelShader(psCode);
	AddEffect(make_unique<BasicEffect>(move(vs), move(ps)));
	D3D11_SHADER_DESC desc;
	const auto vsRefl = _reflectShader(vsCode, desc);
	_addShaderConstantBuffers<VSConstantBuffers>(device, vsRefl, desc);
	const auto psRefl = _reflectShader(psCode, desc);
	_addShaderConstantBuffers<PSConstantBuffers>(device, psRefl, desc);
	_addShaderSamplers<PSSamplers>(variables, psRefl, desc);
	_addShaderTextures<PSShaderResources>(variables, psRefl, desc);
}

void RenderPass::_initShaders(const DxDevice& device, const CBVariableManager& variables,
	const wstring& vsShader, const wstring& gsShader, const wstring& psShader)
{
	_initShaders(device, variables, vsShader, psShader);
	const auto gsCode = DxDevice::LoadByteCode(gsShader);
	auto gs = device.CreateGeometryShader(gsCode);
	AddEffect(make_unique<GeometryShaderComponent>(move(gs)));
	D3D11_SHADER_DESC desc;
	auto gsRefl = _reflectShader(gsCode, desc);
	_addShaderConstantBuffers<GSConstantBuffers>(device, gsRefl, desc);
}

RenderPass::RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
	const wstring& vsShader, const wstring& psShader)
	: m_layouts(layouts)
{
	_initShaders(device, variables, vsShader, psShader);
}

RenderPass::RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
	const wstring& vsShader, const wstring& gsShader, const wstring& psShader)
	: m_layouts(layouts)
{
	_initShaders(device, variables, vsShader, psShader);
}

void RenderPass::_addRenderTarget(const RenderTargetsEffect& renderTarget, bool clearRenderTarget)
{
	auto rt = make_unique<RenderTargetsEffect>(renderTarget);
	rt->SetClearOnBegin(clearRenderTarget);
	AddEffect(move(rt));
}

RenderPass::RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
	const RenderTargetsEffect& renderTarget, bool clearRenderTarget, const wstring& vsShader, const wstring& psShader)
	: m_layouts(layouts)
{
	_addRenderTarget(renderTarget, clearRenderTarget);
	_initShaders(device, variables, vsShader, psShader);
}

RenderPass::RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
	const RenderTargetsEffect& renderTarget, bool clearRenderTarget,
	const wstring& vsShader, const wstring& gsShader, const wstring& psShader)
	: m_layouts(layouts)
{
	_addRenderTarget(renderTarget, clearRenderTarget);
	_initShaders(device, variables, vsShader, gsShader, psShader);
}

void RenderPass::AddModel(const Model* m)
{
	m_models.push_back(m);
}

void RenderPass::AddEffect(std::unique_ptr<EffectComponent>&& effect)
{
	m_effect.m_components.push_back(move(effect));
}

void RenderPass::Execute(const dx_ptr<ID3D11DeviceContext>& context, CBVariableManager& manager)
{
	m_effect.Begin(context);
	for (const Model* model : m_models)
	{
		const auto itEnd = model->end();
		for (auto it = model->begin(); it != itEnd; ++it)
		{
			manager.UpdateModel(it);
			_updateCBuffers(context, manager);
			context->IASetInputLayout(m_layouts->getLayout(it.meshSignatureID(), m_vsSignatureID).get());
			it.mesh().Render(context);
		}
	}
}

dx_ptr<ID3D11ShaderReflection> RenderPass::_reflectShader(const vector<BYTE>& shaderCode, D3D11_SHADER_DESC& shaderDesc)
{
	ID3D11ShaderReflection *temp;
	auto hr = D3DReflect(shaderCode.data(), shaderCode.size(), __uuidof(ID3D11ShaderReflection), reinterpret_cast<void**>(&temp));
	dx_ptr<ID3D11ShaderReflection> shaderRefl(temp);
	if (FAILED(hr))
		THROW_DX(hr);
	hr = shaderRefl->GetDesc(&shaderDesc);
	if (FAILED(hr))
		THROW_DX(hr);
	return move(shaderRefl);
}

vector<CBufferDesc> RenderPass::_getConstantBufferDescriptions(const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc)
{
	vector<CBufferDesc> result;
	result.reserve(shaderDesc.ConstantBuffers);
	for (unsigned int i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		ID3D11ShaderReflectionConstantBuffer* cbuffer = shaderRefl->GetConstantBufferByIndex(i); //not COM interface, no need to Release() it
		D3D11_SHADER_BUFFER_DESC cbufferDesc;
		auto hr = cbuffer->GetDesc(&cbufferDesc);
		if (FAILED(hr))
			THROW_DX(hr);
		result.emplace_back();
		result.back().size = cbufferDesc.Size;
		result.back().variables.reserve(cbufferDesc.Variables);
		for (unsigned int j = 0; j < cbufferDesc.Variables; ++j)
		{
			ID3D11ShaderReflectionVariable* cbVar = cbuffer->GetVariableByIndex(j); //not COM interface, no need to Release() it
			D3D11_SHADER_VARIABLE_DESC cbVarDesc;
			hr = cbVar->GetDesc(&cbVarDesc);
			if (FAILED(hr))
				THROW_DX(hr);
			result.back().variables.push_back({ cbVarDesc.Name, cbVarDesc.StartOffset, cbVarDesc.Size });
		}
	}
	return result;
}

void RenderPass::_updateCBuffers(const dx_ptr<ID3D11DeviceContext>& context, const CBVariableManager& manager)
{
	for (auto cb : m_cbuffers)
		cb->Update(context, manager);
}

vector<string> RenderPass::_getNames(const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc, D3D_SHADER_INPUT_TYPE type)
{
	vector<string> samplers;
	for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC desc;
		shaderRefl->GetResourceBindingDesc(i, &desc);
		if (desc.Type == type)
		{
			if (samplers.size() <= desc.BindPoint)
				samplers.resize(static_cast<size_t>(desc.BindPoint) + 1);
			samplers[desc.BindPoint] = desc.Name;
		}
	}
	return samplers;
}
