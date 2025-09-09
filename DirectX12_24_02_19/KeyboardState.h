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
	DirectInput(HINSTANCE hinst);
	~DirectInput();

	void Execute();

private:


public:
	bool GetKBState();
};