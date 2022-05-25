#include "constantBuffer.h"
#include "dxstructures.h"
#include "exceptions.h"

using namespace std;
using namespace mini;
using namespace utils;

ConstantBufferBase::ConstantBufferBase(dx_ptr<ID3D11Buffer>&& buffer)
	: m_mapped(0), m_bufferObject(move(buffer))
{ }

ConstantBufferBase::ConstantBufferBase(const ConstantBufferBase& right)
	: m_mapped(0)
{
	right.m_bufferObject->AddRef();
	m_bufferObject.reset(right.m_bufferObject.get());
}

ConstantBufferBase::ConstantBufferBase(ConstantBufferBase&& right) noexcept
	: m_mapped(right.m_mapped), m_bufferObject(move(right.m_bufferObject)), m_resource(right.m_resource)
{
	right.m_mapped = 0;
}

ConstantBufferBase& ConstantBufferBase::operator=(const ConstantBufferBase& right)
{
	if (right.m_bufferObject.get() != m_bufferObject.get())
	{
		right.m_bufferObject->AddRef();
		m_bufferObject.reset(right.m_bufferObject.get());
		m_mapped = 0;
	}
	return *this;
}

ConstantBufferBase& ConstantBufferBase::operator==(ConstantBufferBase&& right)
{
	m_bufferObject = move(right.m_bufferObject);
	m_mapped = right.m_mapped;
	m_resource = right.m_resource;
	right.m_mapped = 0;
	return *this;
}

void ConstantBufferBase::Map(const dx_ptr<ID3D11DeviceContext>& context)
{
	if (InterlockedIncrement(&m_mapped) > 1)
		return;
	auto hr = context->Map(m_bufferObject.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m_resource);
	if (FAILED(hr))
		THROW_DX(hr);
}

void ConstantBufferBase::Unmap(const dx_ptr<ID3D11DeviceContext>& context)
{
	if (InterlockedDecrement(&m_mapped))
		return;
	context->Unmap(m_bufferObject.get(), 0);
}

void* ConstantBufferBase::GetMappedPtr() const
{
	if (!m_mapped)
		THROW(L"Buffer not mapped!");
	return m_resource.pData;
}

void ConstantBufferBase::Update(const dx_ptr<ID3D11DeviceContext>& context, const void* dataPtr, unsigned int byteWidth)
{
	if (!byteWidth)
		return;
	Map(context);
	memcpy(m_resource.pData, dataPtr, byteWidth);
	Unmap(context);
}