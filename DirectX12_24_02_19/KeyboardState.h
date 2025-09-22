#pragma once

#include <dinput.h>

class DirectInput
{
private:
static LPDIRECTINPUT8 DIDevicce;
static LPDIRECTINPUTDEVICE8 DIKeyboard;
static BYTE DIKBState[256];
static BYTE DIKBOldState[256];


public:
	DirectInput(HINSTANCE hinst,HWND hWnd);
	~DirectInput();

	void Execute(HWND hWnd);

private:


public:
	BYTE* GetKBState();
	BYTE* GetOldKBState();
};