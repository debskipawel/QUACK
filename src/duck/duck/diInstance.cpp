#include "diInstance.h"
#include "exceptions.h"
#include <cassert>

using namespace std;
using namespace mini;

DiInstance::DiInstance(HINSTANCE appInstance)
{
	Initialize(appInstance);
}

void DiInstance::Initialize(HINSTANCE appInstance)
{
	if (!appInstance)
		return;
	IDirectInput8* di;
	auto hr = DirectInput8Create(appInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		reinterpret_cast<void**>(&di), nullptr);
	m_instance.reset(di);
	if (FAILED(hr))
		THROW_DX(hr);
}

di_ptr DiInstance::CreateInputDevice(HWND hWnd, const GUID& deviceGuid, const DIDATAFORMAT& dataFormat) const
{
	assert(m_instance);
	IDirectInputDevice8* d;
	auto hr = m_instance->CreateDevice(deviceGuid, &d, nullptr);
	di_ptr device(d);
	if (FAILED(hr))
		THROW_DX(hr);
	hr = device->SetDataFormat(&dataFormat);
	if (FAILED(hr))
		THROW_DX(hr);
	hr = device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		THROW_DX(hr);
	return device;
}
