#pragma once

#include <Windows.h>
#include <string>

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define STRINGIFY(x) #x
#define TOWSTRING(x) WIDEN(STRINGIFY(x))
/**************************************************************************//*!
 * Expands to wide string literal in the format of [FILE]:[LINE], where [FILE]
 * is the name of current source file and [LINE] is the current line in the
 * file.
 *****************************************************************************/
#define __AT__ __WFILE__ L":" TOWSTRING(__LINE__)

namespace mini
{
	/**********************************************************************//*!
	 * @brief Abstract base class for exceptions supporting wide string error
	 * messages.
	 * 
	 * Base abstract class for exceptions used by this project. It stores
	 * a pointer to a C string describing the location of the error. User must
	 * make sure the Exception doesn't outlive the C string. __AT__ macro can
	 * be used to generate string literals with that description (literals
	 * have global lifetime, hence they will outlive any Exception.)
	 *************************************************************************/
	class Exception
	{
	public:
		virtual ~Exception()
		{
		}

		/******************************************************************//*!
		 * Creates new exception with the given location description.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 *********************************************************************/
		explicit Exception(const wchar_t* location) { m_location = location; }

		/******************************************************************//*!
		 * Retrieves the error message associated with this exception.
		 * @returns String describing the error and its location.
		 *********************************************************************/
		virtual std::wstring getMessage() const = 0;

		/******************************************************************//*!
		 * Retrieves the error code associated with this exception.
		 * @returns Code number of the error.
		 *********************************************************************/
		virtual int getExitCode() const = 0;

		/******************************************************************//*!
		 * Retrieves the location of the error.
		 * @returns Wide C string describing the location of the error.
		 *********************************************************************/
		const wchar_t* getErrorLocation() const { return m_location; }
	private:
		/// Location of the error.
		const wchar_t* m_location;
	};

	/**********************************************************************//*!
	 * @brief Exception representing an error encountered by a system function
	 * call.
	 * 
	 * Exception class used to represent system errors. It stores a pointer to
	 * a C string describing the location of the error and the error code.
	 * User must make sure the exception doesn't outlive the location
	 * description. The error code should be one of System Error Codes to
	 * provide a meaningfull message.
	 * \sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679360(v=vs.85).aspx
	 *************************************************************************/
	class WinAPIException : public Exception
	{
	public:
		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * code.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param errorCode Error code associated with this exception. This
		 * should be one of System Error Codes to provide a meaningfull
		 * message.
		 * \sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679360(v=vs.85).aspx
		 *********************************************************************/
		explicit WinAPIException(const wchar_t* location, DWORD errorCode = GetLastError());

		/******************************************************************//*!
		 * Retrieves the error code associated with this exception.
		 * @returns Code number of the error.
		 *********************************************************************/
		int getExitCode() const override { return getErrorCode(); }

		/******************************************************************//*!
		 * Retrieves the error code associated with this exception.
		 * @returns Code number of the error.
		 *********************************************************************/
		DWORD getErrorCode() const { return m_code; }

		/******************************************************************//*!
		 * Retrieves the error message associated with this exception.
		 * @returns String describing the error and its location.
		 * @remark The error message is obtained based on the error code
		 * using FormatMessage function and it will not produce meaningfull
		 * message if the error code associated with this exception is not
		 * on of system error codes.
		 * @sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679351(v=vs.85).aspx
		 *********************************************************************/
		std::wstring getMessage() const override;

	private:
		/// Error code associated with this exception
		DWORD m_code;
	};

	/**********************************************************************//*!
	 * @brief Exception class for storing error location and custom error
	 * message in a wide string format.
	 * 
	 * Exception class used to represent custom errors. It stores a pointer to
	 * a C string describing the location of the error and the error message.
	 * User must make sure the exception doesn't outlive the location
	 * description.
	 *************************************************************************/
	class CustomException : public Exception
	{
	public:
		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * message.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param message Custom error message associated with this exception.
		 *********************************************************************/
		CustomException(const wchar_t* location, const std::wstring& message);
		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * message.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param message Custom error message associated with this exception.
		 *********************************************************************/
		CustomException(const wchar_t* location, std::wstring&& message);
		/******************************************************************//*!
		 * Retrieves the error code associated with this exception type.
		 * @returns Code number of this exception type.
		 * @remark Currently the error code is set to -1.
		 *********************************************************************/
		int getExitCode() const override { return -1; }

		/******************************************************************//*!
		 * Retrieves the error message associated with this exception.
		 * @returns String describing the error and its location.
		 *********************************************************************/
		std::wstring getMessage() const override;

	private:
		/// Custom message describing the error.
		std::wstring m_message;
	};
}

/**************************************************************************//*!
 * Throws a WinAPIException.
 *****************************************************************************/
#define THROW_WINAPI throw mini::WinAPIException(__AT__)

/**************************************************************************//*!
 * Throws a WinAPIException.
 *****************************************************************************/
#define THROW_DX(hr) throw mini::WinAPIException(__AT__, hr)

/**************************************************************************//*!
 * Throws a CustomException with a given message.
 *****************************************************************************/
#define THROW(message) throw mini::CustomException(__AT__, message)