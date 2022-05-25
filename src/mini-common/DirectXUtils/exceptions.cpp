#include "exceptions.h"

using namespace std;
using namespace mini;

WinAPIException::WinAPIException(const wchar_t* location, DWORD errorCode)
	: Exception(location), m_code(errorCode)
{

}

wstring WinAPIException::getMessage() const
{
	wstring message;
	try
	{
		LPWSTR lpMsgBuf;
		if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, m_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&lpMsgBuf), 0,
			nullptr))
		{
			message = lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
		message += wstring(L"\nLocation: ") + getErrorLocation();
	}
	catch(...)
	{	}
	return message;
}

CustomException::CustomException(const wchar_t* location, const std::wstring& message)
	: Exception(location), m_message(message)
{

}

CustomException::CustomException(const wchar_t* location, std::wstring&& message)
	: Exception(location), m_message(move(message))
{

}

std::wstring CustomException::getMessage() const
{
	return m_message + L"\nLocation: " + getErrorLocation();
}
