#pragma once

#include <string>
#include <Windows.h>

namespace mini
{
	/**********************************************************************//*!
	 * Wraps a standard WINAPI window message.
	 * @sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms633573(v=vs.85).aspx
	 *************************************************************************/
	struct WindowMessage
	{
		UINT message;	/*!< The message.					   */
		WPARAM wParam;	/*!< Additional message information.   */
		LPARAM lParam;	/*!< Additional message information.   */
		LRESULT result;	/*!< Result of the message processing. */
	};

	/**********************************************************************//*!
	 * Interface that should be implemented by a class that wants to receive
	 * notifications about messages send to a window.
	 *************************************************************************/
	class IWindowMessageHandler
	{
	public:
		virtual ~IWindowMessageHandler()
		{
		}

		/******************************************************************//*!
		 * Called whenever window receives a message.
		 * @param [in,out] m contains message ID and its parameters.
		 * If the message is processed, method should return true and m.result
		 * should be set to the result of message processing.
	     *********************************************************************/
		virtual bool ProcessMessage(WindowMessage& m) = 0;
	};

	/**********************************************************************//*!
	 * @brief WINAPI window.
	 * 
	 * Represents an empty window that can be used for DirectX drawing.
	 *************************************************************************/
	class Window
	{
	public:
		/// Can be used as default width of a window.
		/// @remarks Current value is 1280
		static const int m_defaultWindowWidth;

		/// Can be used as default height of a window.
		/// @remarks Current value is 720
		static const int m_defaultWindowHeight;

		/******************************************************************//*!
		 * @brief Creates a window.
		 * 
		 * Creates an empty window of a given width and height with a default
		 * title and registers a window message handler.
		 * 
		 * Window is not shown immediately. Use Window::Show(int) after it has
		 * beed created.
		 * @param [in] hInstance Application instance handle.
		 * @param [in] width Deisred window width.
		 * @param [in] height Deisired window height.
		 * @param [in] h (optional) Window message handler.
		 * @throws WinAPIException Window could not be created.
		 * @remark Window class named "DirectX 11 Window" is registered if it
		 * doesn't exist yet.
		 *********************************************************************/
		Window(HINSTANCE hInstance, int width, int height,
			   IWindowMessageHandler* h = nullptr);

		/******************************************************************//*!
		 * @brief Creates a window.
		 * 
		 * Creates an empty window with a given width, height and title,
		 * and registers a window message handler.
		 * 
		 * Window is not shown immediately. Use Show after it has
		 * beed created.
		 * @param [in] hInstance Application instance handle.
		 * @param [in] width Deisred window width.
		 * @param [in] height Deisired window height.
		 * @param [in] title Desired window title
		 * @param [in] h (optional) Window message handler.
		 * @throws WinAPIException Window could not be created.
		 * @remark Window class named "DirectX 11 Window" is registered if it
		 * doesn't exist yet.
		 *********************************************************************/
		Window(HINSTANCE hInstance, int width, int height, const std::wstring& title,
			   IWindowMessageHandler* h = nullptr);

		Window(const Window& other) = delete;

		/******************************************************************//*!
		 * Destroys the window
		 *********************************************************************/
		virtual ~Window();

		/******************************************************************//*!
		 * Sets window's show state.
		 * @param [in] cmdShow Specifies how the window is to be shown
		 * @sa https://msdn.microsoft.com/en-us/library/ms633548(v=vs.85).aspx
		 *********************************************************************/
		virtual void Show(int cmdShow);
		/******************************************************************//*!
		 * Retrieves size of the window's client area.
		 * @returns SIZE structure containing width and height of the
		 * window's client area.
		 *********************************************************************/
		SIZE getClientSize() const;
		/******************************************************************//*!
		 * Retrieves coordinates of the window's client area.
		 * @returns RECT structure containing top, bottom, left and right
		 * cooridnates of window's client area. Top and left are always 0.
		 * Right and bottom contain width and height of the client area.
		 *********************************************************************/
		RECT getClientRectangle() const;
		/******************************************************************//*!
		 * Retrieves WINAPI handle for this window.
		 * @returns HWND handle of this window.
		 *********************************************************************/
		HWND getHandle() const { return m_hWnd; }

	protected:
		/******************************************************************//*!
		 * @brief Processes messages incoming to this window.
		 * 
		 * This member function is called whenever this window receives a
		 * message from the system. This implementation forwards all messages
		 * (except for WM_DESTROY, which is handled by posting WM_QUIT message)
		 * to the registered IWindowMessageHandler object.
		 * 
		 * Derrived classes can override this member function to implement
		 * additional program logic.
		 *********************************************************************/
		virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		/******************************************************************//*!
		 * Checks if the window class with name specified by
		 * Window::m_windowClassName is registered and if its WindowProc
		 * callback function is set to Window::WndProc(HWND,UINT,WPARAM,LPARAM)
		 * @param hInstance Application instance handle
		 *********************************************************************/
		static bool IsWindowClassRegistered(HINSTANCE hInstance);

		/******************************************************************//*!
		 * Registers the window class for instances of this type if its not yet
		 * present.
		 * @param hInstance Application instance handle
		 * @sa Window::IsWindowClassRegistered
		 *********************************************************************/
		static void RegisterWindowClass(HINSTANCE hInstance);

		/******************************************************************//*!
		 * @brief WindowProc callback function for window instances of this
		 * type.
		 * 
		 * Callback function called by WINAPI to process messages incoming to
		 * windows of this type. It handles forwarding of messages to specific
		 * window instances by calling non-static member function
		 * Window::WndProc(UINT,WPARAM,LPARAM) on the window instance that is
		 * the recepient of a given message.
		 * @param [in] hWnd Handle of the window receiving the message.
		 * @param [in] msg The message.
		 * @param [in] wParam Additional message information.
		 * @param [in] lParam Additional message information.
		 * @returns Result of message processing.
		 * @remarks Since on some platforms Windows will silently eat all
		 * exceptions that leak through callback, the function catches them all
		 * displays an error message box and terminates the program.
		 *********************************************************************/
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		/******************************************************************//*!
		 * Name of the WINAPI window class used by instances of this type.
		 *********************************************************************/
		static std::wstring m_windowClassName;

		
		/******************************************************************//*!
		 * Creates a window with given width, height, and title.
		 * @param [in] width Desired window width.
		 * @param [in] height Desired window height.
		 * @param [in] title Desired window title.
		 * @throws WinAPIException Window could not be created.
		 *********************************************************************/
		void CreateWindowHandle(int width, int height, const std::wstring& title);

		/// Handle of this window instance
		HWND m_hWnd;
		/// Handle of this application instance
		HINSTANCE m_hInstance;
		/// Registered window message handler
		/// @remarks Can be nullptr if no message handler is present.
		IWindowMessageHandler * m_messageHandler;
	};
}