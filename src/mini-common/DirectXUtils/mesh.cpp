#include "mesh.h"
#include <algorithm>
using namespace std;
using namespace mini;

void Mesh::Render(const dx_ptr<ID3D11DeviceContext>& context) const
{
	if (!m_indexBuffer || m_vertexBuffers.empty())
		return;
	context->IASetPrimitiveTopology(m_primitiveType);
	context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, m_buffersCount, m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
	context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::Mesh()
	: m_buffersCount(0), m_indexCount(0), m_primitiveType(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
{ }

Mesh::Mesh(dx_ptr_vector<ID3D11Buffer>&& vbuffers, vector<unsigned int>&& vstrides, vector<unsigned int>&& voffsets,
		   dx_ptr<ID3D11Buffer>&& indices, unsigned int indexCount,	D3D_PRIMITIVE_TOPOLOGY primitiveType)
{
	assert(vbuffers.size() == voffsets.size() && vbuffers.size() == vstrides.size());
	m_indexCount = indexCount;
	m_buffersCount = static_cast<unsigned>(vbuffers.size());
	m_primitiveType = primitiveType;
	m_indexBuffer = move(indices);
	
	m_vertexBuffers = std::move(vbuffers);
	m_strides = move(vstrides);
	m_offsets = move(voffsets);
}

Mesh::~Mesh()
{
	Release();
}

void Mesh::Release()
{
	m_vertexBuffers.clear();
	m_strides.clear();
	m_offsets.clear();
	m_indexBuffer.reset();
	m_buffersCount = 0;
	m_indexCount = 0;
	m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

Mesh& Mesh::operator=(Mesh&& right)
{
	Release();
	m_vertexBuffers = move(right.m_vertexBuffers);
	m_indexBuffer = move(right.m_indexBuffer);
	m_strides = move(right.m_strides);
	m_offsets = move(right.m_offsets);
	m_buffersCount = right.m_buffersCount;
	m_indexCount = right.m_indexCount;
	m_primitiveType = right.m_primitiveType;
	right.Release();
	return *this;
}

Mesh::Mesh(Mesh&& right)
	: m_indexBuffer(move(right.m_indexBuffer)), m_vertexBuffers(move(right.m_vertexBuffers)),
	  m_strides(move(right.m_strides)), m_offsets(move(right.m_offsets)), m_buffersCount(right.m_buffersCount),
      m_indexCount(right.m_indexCount), m_primitiveType(right.m_primitiveType)
{
	right.Release();
}