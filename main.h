#pragma once

#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")


#include <dwrite.h>
#pragma comment(lib, "dwrite")

#include <wincodec.h>

#include "Player.h"
#include "HeyHo.h"
#include "Egg.h"

using namespace std;
#include <stdio.h>
#include <vector>

#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console" )
#endif

#define BLOCK_L_1F 200.0f
#define BLOCK_R_1F 800.0f
#define BLOCK_L_2F 323.0f
#define BLOCK_R_2F 680.0f
#define DEFAULT_Y_LOC_1F 410.0f
#define DEFAULT_Y_LOC_2F 208.0f



// 자원 안전 반환 매크로.
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
// 비트맵을 로드하는 함수 원형
HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap);
