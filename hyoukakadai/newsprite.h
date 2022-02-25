//#pragma once
//#include"WinApp.h"
//#include<d3d12.h>
//#include<d3dx12.h>
//#include<dxgi1_6.h>
//#include<vector>
//#include<string>
//#include<DirectXMath.h>
//#include<d3dcompiler.h>
//#define DIRECTiNPUT_VERSION 
//#include<dinput.h>
//
//#include<DirectXTex.h>
//#include<wrl.h>
////#include"pipeline.h"
////#include"Header1.hlsli"
//
//#pragma comment(lib,"d3dcompiler.lib")
//#pragma comment(lib,"d3d12.lib")
//
//#pragma comment(lib,"dxgi.lib")
//#pragma comment(lib,"dinput8.lib")
//#pragma comment(lib,"dxguid.lib")
////#include"object3D.h"
//#include"Windows.h"
//using namespace Microsoft::WRL;
//using namespace DirectX;
//class newsprite
//{
//private:
//#pragma region スプライト構造体
//
//	
//public:
//	struct Sprite
//	{
//		//頂点バッファ
//		ComPtr<ID3D12Resource> vertbuff;
//		//定数バッファ
//		ComPtr<ID3D12Resource> constbuff;
//		//頂点バッファビュー
//		D3D12_VERTEX_BUFFER_VIEW vbview{};
//		//ｚ軸周りの回転角
//		float rotation = 0.0f;
//		//アフィン変換行列
//		XMFLOAT3 scale = { 1,1,1 };
//		//座標
//		XMFLOAT3 position = { 0,0,0 };
//		//ワールド行列
//		XMMATRIX matWorld;
//		//テクスチャ番号
//		UINT texNumber = 0;
//		XMFLOAT4 color;
//	};
//#pragma endregion
//#pragma region 頂点構造体の定義
//	struct VertexPosUv
//	{
//		XMFLOAT3 pos;
//		XMFLOAT2 uv;
//	};
//#pragma endregion
//	struct PipelineSet {
//		ComPtr<ID3D12PipelineState>pipelinestate;
//		ComPtr<ID3D12RootSignature>rootsignature;
//	};
//#pragma region 共通データ構造体
//	//テクスチャの最大枚数
//	static const int spriteSRVCount = 512;
//	struct SpriteCommon
//	{
//
//		//パイプラインセット
//		PipelineSet pipelineSet;
//		//射影行列
//		XMMATRIX matProjection{};
//		//テクスチャ専用デスクリプタの生成
//		ComPtr<ID3D12DescriptorHeap>descHeap;
//		//テクスチャリソースの配列
//		ComPtr<ID3D12Resource>texBuff[spriteSRVCount];
//	};
//#pragma endregion
//	// デバイス
// ID3D12Device* device;
//	// デスクリプタサイズ
//	 UINT descriptorHandleIncrementSize;
//	// コマンドリスト
//	 ID3D12GraphicsCommandList* cmdList;
//
//	//スプライトのパイプライン設定
//	PipelineSet create3DpipelineSprite(ID3D12Device* dev);
//
//	//スプライト生成の初期化
//Sprite SpriteCreate(ID3D12Device* dev, int window_width, int window_height);
//
//	//スプライト単体更新
//	void SpriteUpdate(Sprite& sprite, const SpriteCommon& spriteCommon);
//
//	//単体グラフィックスコマンド
//	void SpriteDraw(const Sprite& sprite, ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon, ID3D12Device* dev);
//
//	//共通グラフィックスコマンド
//	void SpriteCommonBeginDraw(const SpriteCommon& spriteCommon, ID3D12GraphicsCommandList* cmdList);
//
//	//スプライト共通データ生成
//	static SpriteCommon SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height);
//
//	//スプライト共通テクスチャ読み込み
//	static HRESULT SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev);
//
//};
//
