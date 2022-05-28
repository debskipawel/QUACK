#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>
#include <memory>

namespace mini
{
	class DiDeleter
	{
	public:
		void operator () (IDirectInputDevice8* diDevice) const
		{
			if (diDevice)
			{
				diDevice->Unacquire();
				diDevice->Release();
			}
		}
	};

	using di_ptr = std::unique_ptr<IDirectInputDevice8, DiDeleter>;
}
