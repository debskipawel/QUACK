#pragma once

#include "dxptr.h"
#include <d3d11.h>
#include "exceptions.h"

namespace mini
{
	class dxDevice;

	class DxSwapChain
	{
	public:
		//Takes ownership of the pointer
		explicit DxSwapChain(dx_ptr<IDXGISwapChain>&& other = nullptr) : m_swapChain(std::move(other)) { }

		DxSwapChain(const DxSwapChain& other) : m_swapChain(clone(other.m_swapChain)) { }
		DxSwapChain(DxSwapChain&& other) noexcept : m_swapChain(std::move(other.m_swapChain)) { }

		DxSwapChain& operator=(const DxSwapChain& other)
		{
			m_swapChain = clone(other.m_swapChain);
			return *this;
		}

		DxSwapChain& operator=(DxSwapChain&& other) noexcept
		{
			m_swapChain = std::move(other.m_swapChain);
			return *this;
		}

		template<typename T = ID3D11Texture2D>
		dx_ptr<T> GetBuffer(unsigned int idx = 0) const
		{
			T* tmp;
			auto hr = m_swapChain->GetBuffer(idx, __uuidof(T), reinterpret_cast<void**>(&tmp));
			dx_ptr<T> result(tmp);
			if (FAILED(hr))
				THROW_DX(hr);
			return result;
		}

		bool Present(unsigned int syncInterval = 0, unsigned int flags = 0) const;
	private:
		dx_ptr<IDXGISwapChain> m_swapChain;
	};
}
