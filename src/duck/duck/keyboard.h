#pragma once
#include "diDeviceBase.h"

namespace mini
{
	struct KeyboardState
	{
		static const unsigned int STATE_TAB_LENGTH = 256;
		static const BYTE KEY_MASK = 0x80;

		BYTE m_keys[STATE_TAB_LENGTH];

		KeyboardState()
		{
			ZeroMemory(m_keys, STATE_TAB_LENGTH * sizeof(char));
		}

		KeyboardState(const KeyboardState& other) = default;

		KeyboardState& operator=(const KeyboardState& other) = default;

		bool isKeyDown(BYTE keyCode) const
		{
			return 0 != (m_keys[keyCode] & KEY_MASK);
		}

		bool keyPressed(const KeyboardState& next, BYTE keyCode)
		{
			return keyPressed(*this, next, keyCode);
		}

		bool keyReleased(const KeyboardState& next, BYTE keyCode)
		{
			return keyReleased(*this, next, keyCode);
		}

		static bool keyPressed(const KeyboardState& prev, const KeyboardState& current, BYTE keyCode)
		{
			return !prev.isKeyDown(keyCode) && current.isKeyDown(keyCode);
		}

		static bool keyReleased(const KeyboardState& prev, const KeyboardState& current, BYTE keyCode)
		{
			return keyPressed(current, prev, keyCode);
		}

		bool isKeyUp(BYTE keyCode) const
		{
			return 0 == (m_keys[keyCode] & KEY_MASK);
		}

		bool operator[](BYTE keyCode) const
		{
			return 0 != (m_keys[keyCode] & KEY_MASK);
		}
	};

	class Keyboard : public DeviceBase
	{
	public:
		bool GetState(KeyboardState& state) const;

		explicit Keyboard(di_ptr&& device = nullptr)
			: DeviceBase(std::move(device))
		{ }

		Keyboard& operator=(const Keyboard& other) = delete;
	};
}