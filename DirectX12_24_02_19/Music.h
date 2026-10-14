#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>

#pragma comment(lib, "winmm.lib")

class BGMObject
{
public:
    BGMObject();
    ~BGMObject();

    void initialize();
    void Update();

private:
    TCHAR status[128];

};