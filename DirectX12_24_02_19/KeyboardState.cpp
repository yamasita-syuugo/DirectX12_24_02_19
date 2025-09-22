#include "UseHeader.h"

LPDIRECTINPUT8 DirectInput::DIDevicce = NULL;
LPDIRECTINPUTDEVICE8 DirectInput::DIKeyboard = NULL;
BYTE DirectInput::DIKBState[256] = {};
BYTE DirectInput::DIKBOldState[256] = {};


DirectInput::DirectInput(HINSTANCE hinst, HWND hWnd)
{
	DirectInput8Create((HINSTANCE)hinst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DIDevicce, NULL);
	DIDevicce->EnumDevices(DI8DEVTYPE_KEYBOARD, nullptr, nullptr, DIEDFL_ATTACHEDONLY);
	DIDevicce->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	DIKeyboard->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
}

DirectInput::~DirectInput()
{
}

BYTE* DirectInput::GetKBState()
{
	return DIKBState;
}
BYTE* DirectInput::GetOldKBState()
{
	return DIKBOldState;
}



void DirectInput::Execute(HWND hWnd) {
	//DIDevicce->RunControlPanel(hWnd);
	memcpy(DIKBOldState,DIKBState,sizeof(BYTE) * 256);
	DIKeyboard->GetDeviceState(256, DIKBState);
}