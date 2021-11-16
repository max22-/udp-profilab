#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#define DLLEXPORT extern"C" __declspec(dllexport)
//Extern "C" declarieren, damit die DLL von externem C-Compiler compiliert werden kann
#pragma once
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szComdLine, int iCmdShow);

