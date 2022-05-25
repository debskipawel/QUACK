#include "windowApplication.h"

using namespace std;
using namespace mini;

WindowApplication::WindowApplication(HINSTANCE hInstance, int wndWidht, int wndHeight,
									 wstring wndTitle)
									 : m_window(hInstance, wndWidht, wndHeight, wndTitle, this), m_hInstance(hInstance)
{ }

int WindowApplication::MainLoop()
{
	MSG msg = { nullptr };
	while ( GetMessageW(&msg, nullptr, 0, 0) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}

int WindowApplication::Run(int cmdShow)
{
	m_window.Show(cmdShow);
	return MainLoop();
}