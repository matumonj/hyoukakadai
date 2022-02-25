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
//#include"Header1.hlsli"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
//#include"GameObject.h"
using namespace DirectX;
using namespace Microsoft::WRL;
const int spriteSRVCount = 512;
class Sprite
{
#pragma region スプライト構造体
	struct Sprites
	{
		//頂点バッファ
		ComPtr<ID3D12Resource> vertbuff;
		//定数バッファ
		ComPtr<ID3D12Resource> constbuff;
		//頂点バッファビュー
		D3D12_VERTEX_BUFFER_VIEW vbview{};
		//ｚ軸周りの回転角
		float rotation = 0.0f;
		//座標
		XMFLOAT3 position = { 0,0,0 };
		//ワールド行列
		XMMATRIX matWorld;
		//テクスチャ番号
		UINT texNumber = 0;
	};
#pragma endregion

#pragma region 共通データ構造体
	//テクスチャの最大枚数
	
	struct SpriteCommon
	{
		//パイプラインセット
		//PipelineSet pipelineSet;
		//射影行列
		XMMATRIX matProjection{};
		//テクスチャ専用デスクリプタの生成
		ComPtr<ID3D12DescriptorHeap>descHeap;
		//テクスチャリソースの配列
		ComPtr<ID3D12Resource>texBuff[spriteSRVCount];
	};
#pragma endregion
	private:


};

