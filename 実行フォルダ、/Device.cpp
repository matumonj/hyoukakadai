#include"Device.h"
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void windowCreate()
{
#pragma region DirectXの初期化
	
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = L"DirectX";
	w.hInstance = GetModuleHandle(nullptr);
	w.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&w);

	RECT wrc = { 0,0,window_width,window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(w.lpszClassName,
		L"DirectX",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr
	);
	ShowWindow(hwnd, SW_SHOW);
	
}
void keyboardSet()
{
	HRESULT resualt;
#pragma region キーボードなどの入力処理
	//////キーボードの初期化
	result = DirectInput8Create(w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	////
	//キーボードデバイスの作成
	////
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//////にゅうりょくデータ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);

	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
#pragma endregion
}

void keyboardUpdate()
{
	result = devkeyboard->Acquire();
	

	result = devkeyboard->GetDeviceState(sizeof(key), key);
}