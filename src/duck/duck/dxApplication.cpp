#include "dxApplication.h"
#include "exceptions.h"

using namespace DirectX;
using namespace mini;
using namespace std;

DxApplication::DxApplication(HINSTANCE hInstance, int wndWidth, int wndHeight, std::wstring wndTitle)
	: WindowApplication(hInstance, wndWidth, wndHeight, wndTitle),
	m_device(m_window), m_inputDevice(hInstance),
	m_mouse(m_inputDevice.CreateMouseDevice(m_window.getHandle())),
	m_keyboard(m_inputDevice.CreateKeyboardDevice(m_window.getHandle())),
	m_camera(XMFLOAT3(0, 0, 0), 0.01f, 50.0f, 5), m_viewport{ m_window.getClientSize() }
{
	ID3D11Texture2D *temp = nullptr;
	auto hr = m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	const dx_ptr<ID3D11Texture2D> backTexture{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);
	m_depthBuffer = m_device.CreateDepthStencilView(m_window.getClientSize());

	ResetRenderTarget();
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
			m_device.swapChain()->Present(0, 0);
		}
	}
	return static_cast<int>(msg.wParam);
}

void DxApplication::Render()
{
	const float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_device.context()->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void mini::DxApplication::UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const void* data, size_t count)
{
	D3D11_MAPPED_SUBRESOURCE res;
	auto hr = m_device.context()->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	if (FAILED(hr))
		THROW_DX(hr);
	memcpy(res.pData, data, count);
	m_device.context()->Unmap(buffer.get(), 0);
}

bool DxApplication::HandleCameraInput(double dt)
{
	MouseState mstate;
	if (!m_mouse.GetState(mstate))
		return false;
	auto d = mstate.getMousePositionChange();
	if (mstate.isButtonDown(0))
		m_camera.Rotate(d.y * ROTATION_SPEED, d.x * ROTATION_SPEED);
	else if (mstate.isButtonDown(1))
		m_camera.Zoom(d.y * ZOOM_SPEED);
	else
		return false;
	return true;
}

void mini::DxApplication::ResetRenderTarget()
{
	auto backBuffer = m_backBuffer.get();
	m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
	m_device.context()->RSSetViewports(1, &m_viewport);
}
