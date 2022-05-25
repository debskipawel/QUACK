#pragma once

#include <set>
#include <map>
#include "inputElements.h"
#include "dxptr.h"
#include "dxDevice.h"
#include <cassert>

namespace mini
{
	class InputLayoutManager
	{
	public:
		explicit InputLayoutManager(const DxDevice& device)
			: m_device(device)
		{ }

		size_t registerSignatureID(const std::vector<BYTE>& vsByteCode)
		{
			return _findOrInsertSignature(vsByteCode);
		}
		size_t registerSignatureID(std::vector<BYTE>&& vsByteCode)
		{
			return _findOrInsertSignature(std::move(vsByteCode));
		}

		size_t registerVertexAttributesID(const VertexAttributes& attributes);

		template<int N>
		size_t registerVertexAttributesID(const D3D11_INPUT_ELEMENT_DESC (&elements)[N])
		{
			VertexAttributes attribs(std::begin(elements), std::end(elements), [this](D3D11_INPUT_ELEMENT_DESC& desc) {
				desc.SemanticName = _findOrInsertSemanticName(desc.SemanticName).data();
			});
			auto it = m_knownVertexAttributes.find(attribs);
			if (it == m_knownVertexAttributes.end())
			{
				auto insertResult = m_knownVertexAttributes.insert(make_pair(move(attribs), m_knownVertexAttributes.size()));
				assert(insertResult.second);
				it = insertResult.first;

			}
			return it->second;
		}

		const dx_ptr<ID3D11InputLayout>& getLayout(size_t vertexAttributesID, size_t signatureID);

	private:
		DxDevice m_device;

		//local copies of semantic names to be referenced by VertexAttributes and InputSignature elements
		std::set<std::string> m_knownSemantics;
		//maps encountered layouts of vertex attributes in vertex buffers of a mesh to internal IDs
		std::map<VertexAttributes, size_t> m_knownVertexAttributes;
		//maps encountered vertex shader input signatures to internal ID and byteCode of the first encountered
		//vertex shader with that signature (needed for input layout creation)
		std::map<InputSignature, std::pair<size_t, std::vector<BYTE>>> m_knownInputSignatures;
		//maps a pair of Vertex Attributes ID and Input Signature ID to an input layout matching the two
		std::map<std::pair<size_t, size_t>, dx_ptr<ID3D11InputLayout>> m_layouts;

		InputSignature _getSignature(const std::vector<BYTE>& vsByteCode);

		template<class Str>
		const std::string&  _findOrInsertSemanticName(Str&& str)
		{
			auto it = m_knownSemantics.find(str);
			if (it == m_knownSemantics.end())
			{
				auto insertResult = m_knownSemantics.insert(std::forward<Str>(str));
				assert(insertResult.second);
				it = insertResult.first;
			}
			return *it;
		}

		template<class Vec>
		size_t _findOrInsertSignature(Vec&& vsByteCode)
		{
			auto signature{ _getSignature(vsByteCode) };
			auto it = m_knownInputSignatures.find(signature);
			if (it == m_knownInputSignatures.end())
			{
				auto insertResult = m_knownInputSignatures.insert(
					make_pair(move(signature), make_pair(m_knownInputSignatures.size(), std::forward<Vec>(vsByteCode))));
				assert(insertResult.second);
				it = insertResult.first;
			}
			return it->second.first;
		}
	};
}
