#pragma once
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<vector>
#include<string>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#define DIRECTiNPUT_VERSION 
#include<dinput.h>

#include<DirectXTex.h>
#include<wrl.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include"object3D.h"
#include"Sprites.h"
#include"pmdmodel.h"
#pragma endregion
using namespace DirectX;
using namespace Microsoft::WRL;


HWND hwnd;
WNDCLASSEX w{};
const int window_width = 900;
const int window_height = 720;
MSG msg{};
HRESULT result;
ComPtr<IDirectInput8>dinput;
ComPtr<IDirectInputDevice8>devkeyboard;
BYTE key[256] = {};

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void windowCreate();
void keyboardSet();
void keyboardUpdate();
