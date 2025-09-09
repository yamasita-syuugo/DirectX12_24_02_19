#include "UseHeader.h"

LPDIRECTINPUT8 DirectInput::DIDevicce = NULL;
LPDIRECTINPUTDEVICE8 DirectInput::DIKeyboard = NULL;
BYTE DirectInput::DIKBState[256] = {};
BYTE DirectInput::DIKBOldState[256] = {};

extern const DIDATAFORMAT c_dfDIKeyboard = {};
extern const GUID GUID_SysKeyboard = {};
extern const GUID IID_IDirectInput8W = {};
#define WINAPI      __stdcall
#define REFIID const IID &
extern HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);

DirectInput::DirectInput(HINSTANCE hinst)
{
	//DirectInput8Create((HINSTANCE)hinst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DIDevicce, NULL);
	//DIDevicce->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	//DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
}

DirectInput::~DirectInput()
{
}

bool DirectInput::GetKBState()
{
	return false;
}



void DirectInput::Execute() {
	//memcpy(DIKBOldState,DIKBState,sizeof(BYTE) * 256);
	//DIKeyboard->GetDeviceState(256, DIKBState);
}