#pragma once

#include "window.h"

namespace mini
{
	/**********************************************************************//*!
	 * @brief Represents an instance of WINAPI application.
	 * 
	 * Handles a lifetime of a GUI application, including creation of a window
	 * and implementation of the application main loop.
	 * 
	 * It should be derrived from to handle more specific application logic.
	 *************************************************************************/
	class WindowApplication : protected IWindowMessageHandler
	{
	public:
		/******************************************************************//*!
		 * @brief Creates application instance.
		 * 
		 * Creates new application instance along with its associated window
		 * with desired widht, height and title.
		 * @param [in] hInstance Application instance handle (one that was
		 * passed to WinMain.
		 * @param [in] wndWidth Desired width of the associated window.
		 * @param [in] wndHeight Desired height of the associated window.
		 * @param [in] wndTitle Desired title of the associated window.
		 *********************************************************************/
		explicit WindowApplication(HINSTANCE hInstance,
			int wndWidth = Window::m_defaultWindowWidth,
			int wndHeight = Window::m_defaultWindowHeight,
			std::wstring wndTitle = L"Default Window");

		/******************************************************************//*!
		 * Destroys the window and frees application resources.
		 *********************************************************************/
		virtual ~WindowApplication() { }

		/******************************************************************//*!
		 * Shows the window and runs program's main loop.
		 * @returns Application exit code.
		 *********************************************************************/
		int Run(int cmdShow = SW_SHOWNORMAL);

		/******************************************************************//*!
		 * Retrieves application's instance handle.
		 * @returns HINSTANCE handle of this program.
		 *********************************************************************/
		HINSTANCE getHandle() const { return m_hInstance; }

	protected:
		/******************************************************************//*!
		 * @brief Handles system messages comming from the associated window.
		 * 
		 * This is the implementation of IWindowMessageHandler. Uppon creation
		 * application instance passes itself as a message handler to its
		 * associated Window. Whenever it recieves a message, it forwards it
		 * to this member function.
		 * 
		 * This implementation ignores all messages. Derrived classes can
		 * override this member function if needed.
		 * @param [in,out] msg contains message ID and its parameters.
		 * If the message is processed, method should return true and m.result
		 * should be set to the result of message processing.
		 * @sa IWindowMessageHandler::ProcessMessage
		 *********************************************************************/
		bool ProcessMessage(WindowMessage& msg) override { return false; }

		/******************************************************************//*!
		 * @brief Main loop of the program.
		 * 
		 * This member function is called by Run to handle program's main loop.
		 * This is a simple blocking implementation, which waits for incomming
		 * messages in the program's queue and dispatches them. Derrived
		 * classes can override this member function to implement different
		 * behavior.
		 * @returns Application exit code.
		 *********************************************************************/
		virtual int MainLoop();

		/******************************************************************//*!
		 * Window associated with the application.
		 *********************************************************************/
		Window m_window;

	private:
		HINSTANCE m_hInstance;
	};
}