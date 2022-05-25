#pragma once

#include "windowApplication.h"
#include "dxDevice.h"
#include "clock.h"
#include "effect.h"

namespace mini
{
	class DxApplication : public WindowApplication
	{
	public:
		explicit DxApplication(HINSTANCE hInstance,
			int wndWidth = Window::m_defaultWindowWidth,
			int wndHeight = Window::m_defaultWindowHeight,
			std::wstring wndTitle = L"DirectX Window");

	protected:
		int MainLoop() override;

		virtual void Update(const Clock& c) { }
		virtual void Render();

		const RenderTargetsEffect& getDefaultRenderTarget() const { return m_renderTarget; }
		const Clock& getClock() const { return m_clock; }

		template<typename T, size_t N = 1>
		ConstantBuffer<T, N> CreateCBuffer()
		{
			return m_device.CreateConstantBuffer<T, N>();
		}

		template<typename T>
		ConstantBuffer<T> CreateCBuffer(const T& data)
		{
			auto buff = CreateCBuffer<T>();
			UpdateCBuffer(buff, data);
			return buff;
		}

		template<typename T, size_t N>
		ConstantBuffer<T, N> CreateCBuffer(const T (&data)[N])
		{
			auto buff = CreateCBuffer<T, N>();
			Update(buff, data);
			return buff;
		}

		template<typename T, size_t N>
		void UpdateCBuffer(ConstantBuffer<T, N>& buffer, typename ConstantBuffer<T, N>::cref_type data)
		{
			buffer.Update(m_device.context(), data);
		}

		DxDevice m_device;

	private:
		RenderTargetsEffect m_renderTarget;
		Clock m_clock;
	};
}
