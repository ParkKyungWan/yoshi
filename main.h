#pragma once

#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")


#include <dwrite.h>
#pragma comment(lib, "dwrite")

#include <wincodec.h>

#include "Player.h"

#include <stdio.h>
#include <string>

#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console" )
#endif

// 자원 안전 반환 매크로.
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
