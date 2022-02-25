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
//#include"pipeline.h"
//#include"Header1.hlsli"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#pragma endregion
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma region 3Dオブジェクトのパイプライン構造体
struct PipelineSet {
	ComPtr<ID3D12PipelineState>pipelinestate;
	ComPtr<ID3D12RootSignature>rootsignature;
};

//定数バッファのデータ構造体
struct ConstBufferData {
	XMFLOAT4 color;
	XMMATRIX mat;
};

#pragma region 定数バッファ周りの構造体
struct Object3d
{
	//定数バッファ
	ID3D12Resource* constBuff;
	//定数バッファビューのハンドル(CPU)
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleCBV;
	//定数バッファビューのハンドル(GPU)
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleCBV;
	//アフィン変換行列
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld;
	//親オブジェクトへのポインタ
	Object3d* parent = nullptr;
};
#pragma endregion

//パイプライン設定
PipelineSet create3Dpipeline(ID3D12Device* dev);
//オブジェクト初期化
void InitializeObject3d(Object3d* object, int index, ID3D12Device* dev, ID3D12DescriptorHeap* descHeap);
//オブジェクト更新処理
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection, XMFLOAT4 colors);
//オブジェクト描画処理
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* descHeap,
	D3D12_VERTEX_BUFFER_VIEW& vbview, D3D12_INDEX_BUFFER_VIEW& ibview, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV,
	UINT  numIndices);

