#pragma once

#include "diptr.h"

namespace mini
{
	class DeviceBase
	{
	public:
		static const unsigned int GET_STATE_RETRIES = 2;
		static const unsigned int AQUIRE_RETRIES = 2;

	protected:
		explicit DeviceBase(di_ptr&& device)
			: m_device(std::move(device))
		{ }

		bool GetState(unsigned int size, void* ptr) const;

		di_ptr m_device;
	};
}