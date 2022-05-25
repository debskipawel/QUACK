#include "inputLayoutManager.h"
#include <D3DCompiler.h>
#include "exceptions.h"
#include <cassert>

using namespace mini;
using namespace std;

InputSignature InputLayoutManager::_getSignature(const vector<BYTE>& vsByteCode)
{
	ID3D11ShaderReflection* prefl;
	auto hr = D3DReflect(vsByteCode.data(), vsByteCode.size(), __uuidof(ID3D11ShaderReflection),
		reinterpret_cast<void**>(&prefl));
	dx_ptr<ID3D11ShaderReflection> reflection(prefl);
	if (FAILED(hr))
		THROW_DX(hr);
	D3D11_SHADER_DESC shaderDesc;
	if (FAILED(hr = reflection->GetDesc(&shaderDesc)))
		THROW_DX(hr);
	vector<D3D11_SIGNATURE_PARAMETER_DESC> signature(shaderDesc.InputParameters);
	for (auto i = 0U; i < shaderDesc.InputParameters; ++i)
	{
		if (FAILED(hr = reflection->GetInputParameterDesc(i, &signature[i])))
			THROW_DX(hr);
		signature[i].SemanticName = _findOrInsertSemanticName(signature[i].SemanticName).data();
	}
	return move(signature);
}

size_t InputLayoutManager::registerVertexAttributesID(const VertexAttributes& attributes)
{
	auto it = m_knownVertexAttributes.find(attributes);
	if (it == m_knownVertexAttributes.end())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> attribs;
		attribs.reserve(attributes.size());
		for (auto& a : attributes)
		{
			attribs.push_back(a);
			attribs.back().SemanticName = _findOrInsertSemanticName(a.SemanticName).data();
		}

		auto insertResult = m_knownVertexAttributes.insert(make_pair(move(attribs), m_knownVertexAttributes.size()));
		assert(insertResult.second);
		it = insertResult.first;
	}
	return it->second;
}

const dx_ptr<ID3D11InputLayout>& InputLayoutManager::getLayout(size_t vertexAttributesID, size_t signatureID)
{
	auto idPair = make_pair(vertexAttributesID, signatureID);
	auto it = m_layouts.find(idPair);
	if (it == m_layouts.end())
	{
		auto signatureIt = find_if(m_knownInputSignatures.begin(), m_knownInputSignatures.end(),
			[signatureID](auto& p) {return p.second.first == signatureID; });
		auto attribsIt = find_if(m_knownVertexAttributes.begin(), m_knownVertexAttributes.end(),
			[vertexAttributesID](auto& p) { return p.second == vertexAttributesID; });
		if (signatureIt == m_knownInputSignatures.end() || attribsIt == m_knownVertexAttributes.end())
			THROW(L"Unregistered vertex attribute set or vertex shader input signature!");
		auto layout = m_device.CreateInputLayout(attribsIt->first.data(), static_cast<unsigned>(attribsIt->first.size()),
			signatureIt->second.second);
		
		auto insertResult = m_layouts.emplace(idPair, move(layout));
		assert(insertResult.second);
		it = insertResult.first;
	}
	return it->second;
}
