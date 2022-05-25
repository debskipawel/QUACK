#include "dxApplication.h"
#include "dxstructures.h"

using namespace mini;
using namespace utils;

DxApplication::DxApplication(HINSTANCE hInstance, int wndWidth, int wndHeight, std::wstring wndTitle)
	: WindowApplication(hInstance, wndWidth, wndHeight, wndTitle), m_device(m_window)
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	auto windowSize = m_window.getClientSize();
	auto backBufferTexture = m_device.swapChain().GetBuffer();
	ViewportDescription viewport(windowSize.cx, windowSize.cy);
	m_renderTarget = RenderTargetsEffect(viewport, m_device.CreateDepthStencilView(windowSize), m_device.CreateRenderTargetView(backBufferTexture));
	m_renderTarget.Begin(m_device.context());
}


int DxApplication::MainLoop()
{
	MSG msg = { nullptr };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_clock.Query();
			Update(m_clock);
			Render();
			m_device.swapChain().Present(1);
		}
	}
	return static_cast<int>(msg.wParam);
}

void DxApplication::Render()
{
	float clearColor[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_renderTarget.ClearRenderTargets(m_device.context(), clearColor);
}
