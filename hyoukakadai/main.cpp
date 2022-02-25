#pragma region include周りの宣言
#include<Windows.h>
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

//まっつんホーム汚すぎ
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include"object3D.h"
#include"Sprites.h"
#include"pmdmodel.h"
#include"WinApp.h"
#define PI 3.141592654
//#include"Device.h"
#pragma endregion
using namespace DirectX;
using namespace Microsoft::WRL;

static ID3D12Device* g_pd3dDevice = NULL;
static int const NUM_FRAMES_IN_FLIGHT = 3;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
//LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void CircleMove(float centerx, float centerz, float* x, float* z, float angle)
{
	float lenx = *x - centerx, lenz = *z - centerz;
	*x = lenx * cos(angle) - lenz * sin(angle) + centerx;
	*z = lenx * sin(angle) + lenz * cos(angle) + centerz;

}
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
#pragma region PMDヘッダー構造体
struct PMDHeader
{
	float version;
	char modelname[20];
	char comment[256];
};
#pragma endregion


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#pragma region DirectXの初期化
	const int window_width = 900;
	const int window_height = 720;

	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = L"DirectX";
	w.hInstance = GetModuleHandle(nullptr);
	w.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&w);

	RECT wrc = { 0,0,window_width,window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(w.lpszClassName,
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
	MSG msg{};
	//でバッグレイヤーのやつ
#ifdef _DEBUG
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();

	}
#endif

#pragma endregion
	//directx初期化

	HRESULT result;
	ID3D12Device* dev = nullptr;
	//ID3D12Device* dev;
	//IDXGIFactory6* dxgiFactory = nullptr;
	//変更
	ComPtr<IDXGIFactory6>dxgiFactory;
	ComPtr<IDXGISwapChain4> swapchain;
	ComPtr<IDXGISwapChain1> swapchain1;
	ComPtr<ID3D12CommandAllocator>cmdAllocator;
	ID3D12GraphicsCommandList* cmdList;
	ComPtr<ID3D12CommandQueue>cmdQueue;
	ComPtr<ID3D12DescriptorHeap>rtvHeaps;

	//dxgiファクトリーの生成
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	//アダプタの列挙用
	std::vector<ComPtr<IDXGIAdapter1>>adapters;

	//ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter1>tmpAdapter;
	for (int i = 0; dxgiFactory->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	//グラフィックボードのアダプタを列挙2
	for (int i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC1 adesc;
		adapters[i]->GetDesc1(&adesc);
		if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"Intel") == std::wstring::npos) {
			tmpAdapter = adapters[i];
			break;
		}
	}

	//デバイスの生成
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;
	for (int i = 0; i < _countof(levels); i++) {
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK) {
			featureLevel = levels[i];
			break;
		}
	}

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));

	//コマンドリストの生成
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));

	//コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

	//スワップチェーンの生成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 900;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	dxgiFactory->CreateSwapChainForHwnd(cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, &swapchain1);
	swapchain1.As(&swapchain);

	//でスクリプタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 2;
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	//レンダーターゲットviewの生成
	std::vector<ComPtr<ID3D12Resource>>backBuffers(2);
	for (int i = 0; i < 2; i++) {
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
			i, dev->GetDescriptorHandleIncrementSize(heapDesc.Type));
		//D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//handle.ptr += i * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		dev->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
	}
	//リソース設定
	ComPtr<ID3D12Resource>depthBuffer;

	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT, window_width, window_height, 1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	//リソース生成
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0), IID_PPV_ARGS(&depthBuffer));

	//深度view用でスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ComPtr<ID3D12DescriptorHeap>dsvHeap;
	result = dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//フェンスの生成
	ComPtr<ID3D12Fence>fence;
	UINT64 fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

#pragma region キーボードなどの入力処理
	//////キーボードの初期化
	ComPtr<IDirectInput8>dinput;
	result = DirectInput8Create(w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	////
	//キーボードデバイスの作成
	////
	ComPtr<IDirectInputDevice8>devkeyboard;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//////にゅうりょくデータ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);

	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
#pragma endregion

	//描画初期化処理
	//頂点データ3点分の座標
#pragma region 描画初期化処理
#pragma region 頂点データ構造体と初期化(vertices)	
//頂点データ構造体
	//三角錐用
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	//スプライト用
	struct VertexPosUv
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	//背景用
	struct BackGround
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	//PMD頂点構造体
	struct PMDVertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		unsigned  short boneNo[2];
		unsigned char boneWeight;
		unsigned char edgeFlg;
	};

	const float topHeight = 10.0f;
	const int Divs = 3;
	const float radius = 10.0f;
	//三角錐の頂点データ
	Vertex vertices[Divs + 1 + 1] = {};

	//for (int i = 0; i < 3; i++) {
	vertices[0].pos.x = radius * sinf(XM_2PI * 0 / Divs);
	vertices[0].pos.y = radius * cosf(XM_2PI * 0 / Divs);
	vertices[0].pos.z = 0;

	vertices[1].pos.x = radius * sinf(XM_2PI * 1 / Divs);
	vertices[1].pos.y = radius * cosf(XM_2PI * 1 / Divs);
	vertices[1].pos.z = 0;

	vertices[2].pos.x = radius * sinf(XM_2PI * 2 / Divs);
	vertices[2].pos.y = radius * cosf(XM_2PI * 2 / Divs);
	vertices[2].pos.z = 0;
	//}eeeewwwww

	vertices[3].pos = { 0,0,0 };

	vertices[4].pos = { 0,0,-topHeight };

	//背景用の板ポリゴン
	BackGround background[] = {
		{{-20.4f,0.0f,0.0f},{}, {0.0f,3.0f}},
		{{-20.4f,50.7f,0.0f},{}, {0.0f,0.0f}},
		{{20.0f,0.0f,0.0f},{}, {3.0f,3.0f}},
		{{20.0f,50.7f,0.0f},{}, {3.0f,0.0f}},
	};


#pragma endregion


#pragma region インデックスデータ
	//インデックスデータ
	unsigned short indices[3 * Divs * 2];
	indices[0] = 1;
	indices[1] = 0;
	indices[2] = 3;

	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	indices[6] = 0;
	indices[7] = 2;
	indices[8] = 3;

	indices[9] = 0;
	indices[10] = 1;
	indices[11] = 4;

	indices[12] = 1;
	indices[13] = 2;
	indices[14] = 4;

	indices[15] = 2;
	indices[16] = 0;
	indices[17] = 4;

	for (int i = 0; i < _countof(indices) / 3; i++)
	{
		unsigned short indices0 = indices[i * 3 + 0];
		unsigned short indices1 = indices[i * 3 + 1];
		unsigned short indices2 = indices[i * 3 + 2];

		XMVECTOR p0 = XMLoadFloat3(&vertices[indices0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices2].pos);

		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);

		XMVECTOR normal = XMVector3Cross(v1, v2);
		normal = XMVector3Normalize(normal);
		XMStoreFloat3(&vertices[indices0].normal, normal);
		XMStoreFloat3(&vertices[indices1].normal, normal);
		XMStoreFloat3(&vertices[indices2].normal, normal);
	}
#pragma endregion

	//背景用の板ポリゴン　インデックスデータ
	unsigned short backgroundindices[] = {
		0,1,2,
		1,2,3,
	};
	//頂点バッファのサイズ
	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * _countof(vertices));
	//頂点バッファへの生成
	ComPtr<ID3D12Resource>vertBuff;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));

	//仮想メモリを取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i];
	}
	//マップ解除
	vertBuff->Unmap(0, nullptr);

#pragma region 背景用の板ポリゴン
	//頂点バッファのサイズ
	UINT backsizeVB = static_cast<UINT>(sizeof(BackGround) * _countof(background));
	//頂点バッファへの生成
	ComPtr<ID3D12Resource>backvertBuff;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&backvertBuff));

	//仮想メモリを取得
	BackGround* backvertMap = nullptr;
	result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
	for (int i = 0; i < _countof(background); i++) {
		backvertMap[i] = background[i];
	}
	//マップ解除
	backvertBuff->Unmap(0, nullptr);

#pragma endregion

#pragma endregion
	//頂点バッファviewの作成
	D3D12_VERTEX_BUFFER_VIEW vbview{};

	vbview.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbview.SizeInBytes = sizeVB;
	vbview.StrideInBytes = sizeof(Vertex);

	//インデックスバッファの設定	
	ComPtr<ID3D12Resource>indexBuff;
	UINT sizeIB = sizeof(indices);

	//インデックスバッファの生成
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeIB), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&indexBuff));

	//仮想メモリを取得
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);

	//全インデックスに対して
	for (int i = 0; i < _countof(indices); i++) {
		indexMap[i] = indices[i];
	}
	indexBuff->Unmap(0, nullptr);

	//インデックスバッファviewの作成
	D3D12_INDEX_BUFFER_VIEW ibview{};
	ibview.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibview.Format = DXGI_FORMAT_R16_UINT;
	ibview.SizeInBytes = sizeof(indices);

#pragma region 背景用の板ポリゴン
	//頂点バッファviewの作成
	D3D12_VERTEX_BUFFER_VIEW backvbview{};

	backvbview.BufferLocation = backvertBuff->GetGPUVirtualAddress();
	backvbview.SizeInBytes = backsizeVB;
	backvbview.StrideInBytes = sizeof(BackGround);

	//インデックスバッファの設定	
	ComPtr<ID3D12Resource>backindexBuff;
	UINT backsizeIB = sizeof(backgroundindices);

	//インデックスバッファの生成
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeIB), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&backindexBuff));

	//仮想メモリを取得
	unsigned short* backindexMap = nullptr;
	result = backindexBuff->Map(0, nullptr, (void**)&backindexMap);

	//全インデックスに対して
	for (int i = 0; i < _countof(backgroundindices); i++) {
		backindexMap[i] = backgroundindices[i];
	}
	backindexBuff->Unmap(0, nullptr);

	//インデックスバッファviewの作成
	D3D12_INDEX_BUFFER_VIEW backibview{};
	backibview.BufferLocation = backindexBuff->GetGPUVirtualAddress();
	backibview.Format = DXGI_FORMAT_R16_UINT;
	backibview.SizeInBytes = sizeof(backgroundindices);

#pragma endregion

	//仮想メモリを取得

	//射影変換行列の作り
	XMMATRIX matprojection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)window_width / window_height, 0.1f, 1000.0f);

	//ビュー変換行列
	XMMATRIX matview;
	XMFLOAT3 eye(0, 0, -10);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//カメラの回転角
	float angle = 0.0f;

	//ワールド変換行列
	XMMATRIX matworld0;

	XMFLOAT3 position;
	position = { 0.0f,0.0f,0.0f };

	XMMATRIX matworld1;


	//デスクリプタヒープ拡張用の変数
	const int constantBufferNum = 128;
	//定数バッファ用でスクリプタヒープの生成
	ID3D12DescriptorHeap* basicDescHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = constantBufferNum + 1;

	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	auto basicHeapHandle0 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		0, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//2
	auto basicHeapHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		1, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//3dオブジェクトの数
	const int object_num = 1;

	Object3d object3ds[object_num];//敵機のようなもの
	Object3d player;//プレイヤー
	Object3d backgrounds;//保留
	Object3d backgroundsleft;//保留
	Object3d backgroundsright;//保留
	Object3d pmdmodel;
#pragma region オブジェクトの初期化処理
	//player用

	InitializeObject3d(&player, 1, dev, basicDescHeap);
	InitializeObject3d(&pmdmodel, 1, dev, basicDescHeap);

	//背景用　今は保留
	InitializeObject3d(&backgrounds, 1, dev, basicDescHeap);
	InitializeObject3d(&backgroundsleft, 1, dev, basicDescHeap);
	InitializeObject3d(&backgroundsright, 1, dev, basicDescHeap);
	//player周りのオブジェクト用
//for (int i = 0; i < _countof(object3ds); i++) {
	InitializeObject3d(&object3ds[0], 1, dev, basicDescHeap);
	//	if (i >= 0) {
			//親子関係切る
			//object3ds[i].parent = &object3ds[i - 1];
	object3ds[0].scale = { 0.5f,0.5f,0.5f };
	object3ds[0].rotation = { 0.0f,0.0f,0.0f };
	//}
//}
#pragma endregion

#pragma region オブジェクト配置とかスケール変更
	//オブジェクトの配置
	//プレイヤーオブジェクト

	player.position = { 0,0,0.0f };

	//player[1]をplayer[0]の子として持つ親子関係 飾りみたいなもん;
	player.rotation = { 0,0,0 };
	player.position = { 0,0,-0 };
	player.scale = { 0.1f,0.1f,0.1f };
	//player[1].scale = { 0.3f,0.3f,0.3f };
	//左側配置z
	object3ds[0].position = { 0.0f,0.0f,90.0f };
	//右側配置

	//背景の設定
	backgrounds.position = { -40,-800,800 };
	backgrounds.scale = { 150,150,1 };

	backgroundsleft.position = { -400,-400,80 };
	backgroundsleft.scale = { 100,100,1 };
	backgroundsleft.rotation = { 90,0,0 };
	backgroundsright.position = { -400,-410,4580 };
	backgroundsright.scale = { 100,100,1 };
	backgroundsright.rotation = { 90,0,0 };
	//敵機が生きてるか死んでるかどうかのフラグ
	int objectarrive[object_num];
	for (int i = 0; i < _countof(objectarrive); i++) {
		objectarrive[i] = 1;
	}
	float playerangle = 0;
	float angleradius = 40;
#pragma endregion

	//シェーダリソースビューのアドレス計算処理
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	//ハンドルのアドレスを進める
	cpuDescHandleSRV.ptr += constantBufferNum * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	gpuDescHandleSRV.ptr += constantBufferNum * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV2 = basicDescHeap->GetGPUDescriptorHandleForHeapStart();
	gpuDescHandleSRV2.ptr += constantBufferNum * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//共通データの関数呼び出し
	SpriteCommon spritecommon;
	//スプライト共通データ生成
	spritecommon = SpriteCommonCreate(dev, window_width, window_height);
	//スプライト用テクスチャ読み込みの関数呼び出し
	SpriteCommonLoadTexture(spritecommon, 0, L"Resources/hart.png", dev);
	SpriteCommonLoadTexture(spritecommon, 1, L"Resources/DQIX_-_Cruelcumber.png", dev);
	SpriteCommonLoadTexture(spritecommon, 2, L"Resources/gameoversprite.png", dev);
	SpriteCommonLoadTexture(spritecommon, 3, L"Resources/tittlesprite.png", dev);
	//スプライト初期化関数の呼び出し
	Sprite sprite[6];
	//スプライトの生成
	for (int i = 0; i < 6; i++) {
		sprite[i] = SpriteCreate(dev, window_width, window_height);
	}
	//WICテクスチャのロード
	//画像ファイルの用意
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(L"Resources/Tabizine_160808_1.jpg", WIC_FLAGS_NONE, &metadata, scratchImg);

	const Image* img = scratchImg.GetImage(0, 0, 0);


	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, metadata.width,
		static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	////3
	ComPtr<ID3D12Resource>texBuff = nullptr;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0), D3D12_HEAP_FLAG_NONE, &texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texBuff));

	//テクスチャバッファにデータ転送
	result = texBuff->WriteToSubresource(0, nullptr, img->pixels, static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch));

	////3
	auto basicHeapHandle2 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		2, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));


	////シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	srvDesc.Format = metadata.format;
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	dev->CreateShaderResourceView(texBuff.Get(), &srvDesc, cpuDescHandleSRV);

	auto gpuDescHandle0 = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		0, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	////2
	auto gpuDescHandle1 = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		1, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	////3
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle2 = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		2, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	);
	float angles = 0;
	float angles2 = 0;
	//3Dオブジェクト用パイプライン生成
	PipelineSet obj3Dpipelineset = create3Dpipeline(dev);
	//スプライト用パイプライン設定
	PipelineSet spriteobj3Dpipelineset = create3DpipelineSprite(dev);

	PipelineSet pmdcreatepipeline = create3Dpipelinepmd(dev);

	//スプライトの配置
	sprite[0].position = { 20,12,0 };
	sprite[1].position = { 84,12,0 };
	sprite[2].position = { 148,12,0 };
	sprite[3].position = { 702,12,0 };
	sprite[4].position = { 0,0,0 };
	sprite[4].scale = { 14.5,12,0 };
	sprite[5].position = { 0,0,0 };
	sprite[5].scale = { 14.5,12,0 };

	//スプラトごとにテクスチャを指定
	//テクスチャ0がライフの画像
	//テクスチャ1がズッキーニャの画像
	sprite[0].texNumber = 0;
	sprite[1].texNumber = 0;
	sprite[2].texNumber = 0;
	sprite[3].texNumber = 1;
	sprite[4].texNumber = 2;
	sprite[5].texNumber = 3;

	//SpriteUpdate(sprite, spritecommon);
#pragma endregion

	float colorr = 0.0f;
	float backcolor1 = 1.0f;
	float backcolor2 = 1.0f;
	float backcolor3 = 1.0f;
	int retry = 0;
	//オブジェクトの更新処理で使う引き数の色情報用
	XMFLOAT4 RedColor = { 1,0.6,0.6,0.7f };
	XMFLOAT4 BlueColor = { 0.5,0.5f,0.5f,1 };
	XMFLOAT4 GreenColor = { 0,1,0,1 };

	float angle2 = 80.110001f;
	float angle3 = 80.110001f;
	float angle4 = 80.110001f;
	float d = 20;
	XMVECTOR v0 = { player.position.x,player.position.y,player.position.z - 10,0 };
	XMMATRIX rotM;// = XMMatrixIdentity();
	XMVECTOR eye2;
	XMVECTOR target2 = { object3ds[0].position.x, object3ds[1].position.y, object3ds[2].position.z, 0 };
	XMVECTOR up2 = { 0,0.3f,0,0 };


	int scene = 2;

	while (true) {
		//ウィンドウメッセージ処理
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
		//v0.m128_f32[0] = player.position.x;
		v0.m128_f32[1] = player.position.y;
		//v0.m128_f32[2] = player.position.z-10;
		//グラフィックのやつ各とこ
		//毎フレーム処理
		//キーボード処理
		result = devkeyboard->Acquire();
		BYTE key[256] = {};

		result = devkeyboard->GetDeviceState(sizeof(key), key);

		////背景色の設定
		float clearColor[] = { 1.0f,1.0f,1.0f,0.0f };
		//背景がこっち来るように
		if (scene == 2) {
			if (backgrounds.position.z > 100) {
				//backgrounds.position.z -= 0.02f;
			}
			//	backgroundsleft.position.z -= 3.32f;
				//}
			//	if (backgroundsright.position.z > -300) {
				//backgroundsright.position.z -= 3.32f;
				//	}
			if (backgroundsleft.position.z < -4500) {
				backgroundsleft.position.z = 800;
			}
			if (backgroundsright.position.z < -4500) {
				backgroundsright.position.z = 800;
			}
#pragma region 移動処理とか

			//移動のやつ

			//pmdmodel.position.z += 0.4f;
			//if (key[DIK_UP] || key[DIK_DOWN] || key[DIK_LEFT] || key[DIK_RIGHT]) {
			if (key[DIK_UP]) {
				player.position.z += 3;
				angles2 += XMConvertToRadians(15);//自機の回転
			}
			if (key[DIK_DOWN]) {
				player.position.z -= 3;
				angles2 -= XMConvertToRadians(15);//自機の回転
				//if (pmdmodel.position.y > -205) {
					//player.position.y -= 1.0f;
				//}
			}
			if (key[DIK_LEFT]) {
				//playerangle--;
				//playerangle -= XMConvertToRadians(90.0f);//自機の回転
			}
			if (key[DIK_RIGHT]) {
				//playerangle++;
			   //playerangle += XMConvertToRadians(90.0f);//自機の回転
			}


			//自機の回転に制限つける
			//angles = min(angles, XMConvertToRadians(1800));
			//angles = max(angles, XMConvertToRadians(-1800));

			//angles2 = min(angles2, XMConvertToRadians(900));
			//angles2 = max(angles2, XMConvertToRadians(-900));
		//}
		//敵を中心とした円運動
			player.position.x = object3ds[0].position.x + cosf(playerangle * PI / 180) * angleradius;
			player.position.z = object3ds[0].position.z + sinf(playerangle * PI / 180) * angleradius;

			//単位ベクトルの設定
			float distance = 10;
			XMFLOAT3 player_to_target = XMFLOAT3(object3ds[0].position.x - player.position.x,
				object3ds[0].position.y - player.position.y, object3ds[0].position.z - player.position.z);
			float length = sqrt(pow(player_to_target.x, 2) + pow(player_to_target.y, 2) + pow(player_to_target.z, 2));
			XMFLOAT3 UnitVec_player_target = XMFLOAT3(player_to_target.x / length, player_to_target.y / length, player_to_target.z / length);
			//注視点の設定
			target2.m128_f32[0] = object3ds[0].position.x + UnitVec_player_target.x;
			target2.m128_f32[1] = object3ds[0].position.y + UnitVec_player_target.y;
			target2.m128_f32[2] = object3ds[0].position.z + UnitVec_player_target.z;
			//カメラ位置の設定
			eye2.m128_f32[0] = player.position.x - UnitVec_player_target.x * distance;
			eye2.m128_f32[1] = 3.0f;
			eye2.m128_f32[2] = player.position.z - UnitVec_player_target.z * distance;



			//行列を作り直す
			rotM = XMMatrixRotationX(XMConvertToRadians(angle));
			XMVECTOR v;
			v = XMVector3TransformNormal(v0, rotM);
			//eye2 = target2 + v;
			matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

#pragma endregion 

#pragma region オブジェクト更新処理
			//敵用の更新処理
		//	for (int i = 0; i < _countof(object3ds); i++) {
			UpdateObject3d(&object3ds[0], matview, matprojection, { 1,0,0,1 });
			//}

			//player用の更新処理
			UpdateObject3d(&player, matview, matprojection, BlueColor);
			//背景用の更新処理　今は保留
			UpdateObject3d(&backgrounds, matview, matprojection, { backcolor1,backcolor2,backcolor3,1 });
			UpdateObject3d(&backgroundsleft, matview, matprojection, { 1,1,1,0.04 });
			UpdateObject3d(&backgroundsright, matview, matprojection, { 1,1,1,0.05 });
			//pmdmodel表示
			//UpdateObject3d(&pmdmodel, matview, matprojection, { 0,0,0,0 });

			//スプライトの更新処理呼び出し
			for (int i = 0; i < 5; i++) {
				SpriteUpdate(sprite[i], spritecommon);
			}
		}
#pragma endregion

		//仮想メモリを取得
		Vertex* vertMap = nullptr;
		result = vertBuff->Map(0, nullptr, (void**)&vertMap);
		for (int i = 0; i < _countof(vertices); i++) {
			vertMap[i] = vertices[i];
		}
		vertBuff->Unmap(0, nullptr);

#pragma endregion
#pragma region 背景用の板ポリゴン
		//仮想メモリを取得
		BackGround* backvertMap = nullptr;
		result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
		for (int i = 0; i < _countof(background); i++) {
			backvertMap[i] = background[i];
		}
		backvertBuff->Unmap(0, nullptr);

		//x,y座標のデバッグログ
		wchar_t str[256];
		//for (int i = 0; i < _countof(vertices4); i++) {
		swprintf_s(str, L"position:%f\n", pmdmodel.position.y);
		//}
		OutputDebugString(str);
		swprintf_s(str, L"objectZ:%f\n", object3ds[0].position.z);
		OutputDebugString(str);


		UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
		//実行
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));
		//描画先指定
		auto rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, dev->GetDescriptorHandleIncrementSize(heapDesc.Type));

		//深度ステンシルビュー用でスクリプタヒープ
		D3D12_CPU_DESCRIPTOR_HANDLE dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

		//画面クリア
		cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

#pragma region 描画コマンド
		//描画コマンド
		cmdList->SetPipelineState(obj3Dpipelineset.pipelinestate.Get());
		cmdList->SetGraphicsRootSignature(obj3Dpipelineset.rootsignature.Get());

		//スプライト共通コマンド
		//cmdList->SetPipelineState(pmdcreatepipeline.pipelinestate.Get());
		//cmdList->SetGraphicsRootSignature(pmdcreatepipeline.rootsignature.Get());
		cmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, window_width, window_height));

		cmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, window_width, window_height));

		//プリティ部形状の設定コマンド
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//描画処理の呼び出し

		//if (scene != 3) {
		//	for (int i = 0; i < _countof(object3ds); i++) {
			//	if (objectarrive[i] == 1) {
		DrawObject3d(&object3ds[0], cmdList, basicDescHeap, vbview, ibview, gpuDescHandleSRV, _countof(indices));
		//	}
		//}
	//}

	//playerの描画
		DrawObject3d(&player, cmdList, basicDescHeap, vbview, ibview, gpuDescHandleSRV, _countof(indices));
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		DrawObject3d(&backgrounds, cmdList, basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		DrawObject3d(&backgroundsleft, cmdList, basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		DrawObject3d(&backgroundsright, cmdList, basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);

		//スプライト描画
		SpriteCommonBeginDraw(spritecommon, cmdList);//,basicDescHeap);

		for (int i = 0; i < 3; i++) {
			SpriteDraw(sprite[i], cmdList, spritecommon, dev);
		}

		if (scene == 1)
		{
			SpriteUpdate(sprite[5], spritecommon);
			SpriteDraw(sprite[5], cmdList, spritecommon, dev);
		}
		//グラフィックスコマンド関数の呼び出し
		//cmdList->SetPipelineState(pmdcreatepipeline.pipelinestate.Get());
		//cmdList->SetGraphicsRootSignature(pmdcreatepipeline.rootsignature.Get());
		//プリティ部形状の設定コマンド
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//DrawObject3d(&pmdmodel, cmdList, basicDescHeap, pmdvbview, pmdibview, gpuDescHandleSRV, indicesNum);

	//描画コマンド　ここまで
#pragma endregion
		//リソースバリアを戻す
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

		cmdList->Close();

		ID3D12CommandList* cmdLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(1, cmdLists);
		cmdQueue->Signal(fence.Get(), ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		cmdAllocator->Reset();
		cmdList->Reset(cmdAllocator.Get(), nullptr);
		swapchain->Present(1, 0);

	}

	UnregisterClass(w.lpszClassName, w.hInstance);
	OutputDebugStringA("Hello,jjjj\n");

	return 0;
}

