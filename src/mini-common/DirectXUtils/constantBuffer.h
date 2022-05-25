#pragma once
#include <d3d11.h>
#include "dxptr.h"
#include <type_traits>

namespace mini
{
	class DxDevice;

	class ConstantBufferBase
	{
	public:
		//Creates instance in an undefined state.
		//Other methods cannot be used unless object is assigned to.
		ConstantBufferBase() { }

		explicit ConstantBufferBase(dx_ptr<ID3D11Buffer>&& buffer);
		ConstantBufferBase(const ConstantBufferBase& right);
		ConstantBufferBase(ConstantBufferBase&& right) noexcept;
		ConstantBufferBase& operator=(const ConstantBufferBase& right);
		ConstantBufferBase& operator==(ConstantBufferBase&& right);

		void Update(const dx_ptr<ID3D11DeviceContext>& context, const void* dataPtr, unsigned int byteWidth);
		
		void Map(const dx_ptr<ID3D11DeviceContext>& context);
		void* GetMappedPtr() const;
		void Unmap(const dx_ptr<ID3D11DeviceContext>& context);

		ID3D11Buffer& operator*() const { return *m_bufferObject.get(); }
		operator ID3D11Buffer*() const { return m_bufferObject.get(); }

	private:
		long m_mapped;
		dx_ptr<ID3D11Buffer> m_bufferObject;
		D3D11_MAPPED_SUBRESOURCE m_resource;
	};

	template<typename T, unsigned int N = 1>
	class ConstantBuffer : ConstantBufferBase
	{
		friend class DxDevice;

	public:
		using value_type = typename std::conditional<N == 1, T, T[N]>::type;
		using ref_type = value_type&;
		using cref_type = const value_type&;
		using ptr_type = T*;

		//Creates instance in an undefined state.
		//Other methods cannot be used unless object is assigned to.
		ConstantBuffer() { }

		ConstantBuffer(const ConstantBuffer<T, N>& right)
			: ConstantBufferBase(right) { }
		ConstantBuffer(ConstantBuffer<T, N>&& right) noexcept
			: ConstantBufferBase(static_cast<ConstantBufferBase&&>(right))
		{ }

		ConstantBuffer<T, N>& operator=(const ConstantBuffer<T, N>& right)
		{
			this->ConstantBufferBase::operator=(right);
			return *this;
		}

		ConstantBuffer<T, N>& operator=(ConstantBuffer<T, N>&& right) noexcept
		{
			this->ConstantBufferBase::operator=(static_cast<ConstantBufferBase&&>(right));
			return *this;
		}

		void Update(const dx_ptr<ID3D11DeviceContext>& context, cref_type data)
		{
			return ConstantBufferBase::Update(context, reinterpret_cast<const void*>(dataPtr(data)), sizeof(data));
		}

		T* GetMappedPtr() const { return reinterpret_cast<T*>(GetMappedPtr()); }

		using ConstantBufferBase::Map;
		using ConstantBufferBase::Unmap;
		using ConstantBufferBase::operator*;
		using ConstantBufferBase::operator ID3D11Buffer*;

	private:
		explicit ConstantBuffer(dx_ptr<ID3D11Buffer>&& buffer)
			: ConstantBufferBase(move(buffer))
		{ }

		static const T* dataPtr(const T& data)
		{
			return &data;
		}

		static const T* dataPtr(const T (&data)[N])
		{
			return data;
		}
	};
}