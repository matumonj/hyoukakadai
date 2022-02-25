#pragma once
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>

#include<DirectXMath.h>
#include<d3dcompiler.h>
#define DIRECTiNPUT_VERSION 
#include<dinput.h>

#include<DirectXTex.h>
#include<wrl.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#include"object3D.h"



//pmd用パイプラインの設定
PipelineSet create3Dpipelinepmd(ID3D12Device* dev);
//pmd読み込み
void PMDModelRead();
