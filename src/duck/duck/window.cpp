#include "window.h"
#include "exceptions.h"
#include <basetsd.h>

using namespace std;
using namespace mini;

wstring Window::m_windowClassName = L"DirectX 11 Window";
const int Window::m_defaultWindowWidth = 1280;
const int Window::m_defaultWindowHeight = 720;

void Window::RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW c;
	ZeroMemory(&c, sizeof(WNDCLASSEXW));

	c.cbSize = sizeof(WNDCLASSEXW);
	c.style = CS_HREDRAW | CS_VREDRAW;
	c.lpfnWndProc = WndProc;
	c.hInstance = hInstance;
	c.hCursor = LoadCursor(nullptr, IDC_ARROW);
	c.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	c.lpszMenuName = nullptr;
	c.lpszClassName = m_windowClassName.c_str();
	c.cbWndExtra = sizeof(LONG_PTR);
	if (!RegisterClassExW(&c))
		THROW_WINAPI;
}
bool Window::IsWindowClassRegistered(HINSTANCE hInstance)
{
	WNDCLASSEXW c;
	if (!GetClassInfoExW(hInstance, m_windowClassName.c_str(), &c) == TRUE)
		return false;
	return c.lpfnWndProc != static_cast<WNDPROC>(Window::WndProc);
}

Window::Window(HINSTANCE hInstance, int width, int height, IWindowMessageHandler* h)
	: m_hInstance(hInstance), m_messageHandler(h)
{
	CreateWindowHandle(width, height, m_windowClassName);
}

Window::Window(HINSTANCE hInstance, int width, int height, const wstring& title,
	IWindowMessageHandler* h)
	: m_hInstance(hInstance), m_messageHandler(h)
{
	CreateWindowHandle(width, height, title);
}

void Window::CreateWindowHandle(int width, int height, const wstring& title)
{
	if (!IsWindowClassRegistered(m_hInstance))
		RegisterWindowClass(m_hInstance);
	RECT rect = { 0, 0, width, height };
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	if (!AdjustWindowRect(&rect, style, FALSE))
		THROW_WINAPI;
	m_hWnd = CreateWindowW(m_windowClassName.c_str(), title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, m_hInstance, this);
	if (!m_hWnd)
		THROW_WINAPI;
}

Window::~Window()
{
	//Clear the user data pointer, so this window will not recievie any more
	//messages to its WndProc. This prevents the call to PostQuitMessage in 
	//response to WM_DESTROY message caused by the following DestroyWindow
	//call. PostQuitMessage is not necessary at that point, because application
	//is already closing. However, the stray WM_QUIT would sit in the thread 
	//message queue, so when application closes due to an exception, the
	//WM_QUIT will be intercepted by the MessageBox showing the error message
	//causing it to close immediately.
	SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, 0L);

	DestroyWindow(m_hWnd);
}

LRESULT Window::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT paintStruct;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(EXIT_SUCCESS);
		break;
	default:
		WindowMessage m = { msg, wParam, lParam, 0 };
		if (m_messageHandler && m_messageHandler->ProcessMessage(m))
		{
			return m.result;
		}
		if (msg == WM_PAINT)
		{
			BeginPaint(m_hWnd, &paintStruct);
			EndPaint(m_hWnd, &paintStruct);
			break;
		}
		return DefWindowProc(m_hWnd, msg, wParam, lParam);
	}
	return 0;
}

void Window::Show(int cmdShow)
{
	ShowWindow(m_hWnd, cmdShow);
}

RECT Window::getClientRectangle() const
{
	RECT r;
	GetClientRect(m_hWnd, &r);
	return r;
}

SIZE Window::getClientSize() const
{
	auto r = getClientRectangle();
	SIZE s = { r.right - r.left, r.bottom - r.top };
	return s;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* wnd;
	if (msg == WM_CREATE)
	{
		auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		wnd = static_cast<Window*>(pcs->lpCreateParams);
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
		wnd->m_hWnd = hWnd;
	}
	else
	{
		wnd = reinterpret_cast<Window*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
	}

	//Windows likes to eat exceptions that leak through callbacks on some platforms.
	try
	{
		return wnd ? wnd->WndProc(msg, wParam, lParam) : DefWindowProc(hWnd, msg, wParam, lParam);
	}
	catch (Exception & e)
	{
		MessageBoxW(nullptr, e.getMessage().c_str(), L"Błąd", MB_OK | MB_ICONERROR);
		PostQuitMessage(e.getExitCode());
		return e.getExitCode();
	}
	catch (exception & e)
	{
		string s(e.what());
		MessageBoxW(nullptr, wstring(s.begin(), s.end()).c_str(), L"Błąd", MB_OK | MB_ICONERROR);
	}
	catch (const char* str)
	{
		string s(str);
		MessageBoxW(nullptr, wstring(s.begin(), s.end()).c_str(), L"Błąd", MB_OK | MB_ICONERROR);
	}
	catch (const wchar_t* str)
	{
		MessageBoxW(nullptr, str, L"Błąd", MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		MessageBoxW(nullptr, L"Nieznany Błąd", L"Błąd", MB_OK | MB_ICONERROR);
	}
	PostQuitMessage(EXIT_FAILURE);
	return EXIT_FAILURE;
}
