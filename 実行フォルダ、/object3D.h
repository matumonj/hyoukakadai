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

#pragma region 3D�I�u�W�F�N�g�̃p�C�v���C���\����
struct PipelineSet {
	ComPtr<ID3D12PipelineState>pipelinestate;
	ComPtr<ID3D12RootSignature>rootsignature;
};

//�萔�o�b�t�@�̃f�[�^�\����
struct ConstBufferData {
	XMFLOAT4 color;
	XMMATRIX mat;
};

#pragma region �萔�o�b�t�@����̍\����
struct Object3d
{
	//�萔�o�b�t�@
	ID3D12Resource* constBuff;
	//�萔�o�b�t�@�r���[�̃n���h��(CPU)
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleCBV;
	//�萔�o�b�t�@�r���[�̃n���h��(GPU)
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleCBV;
	//�A�t�B���ϊ��s��
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//���[���h�ϊ��s��
	XMMATRIX matWorld;
	//�e�I�u�W�F�N�g�ւ̃|�C���^
	Object3d* parent = nullptr;
};
#pragma endregion

//�p�C�v���C���ݒ�
PipelineSet create3Dpipeline(ID3D12Device* dev);
//�I�u�W�F�N�g������
void InitializeObject3d(Object3d* object, int index, ID3D12Device* dev, ID3D12DescriptorHeap* descHeap);
//�I�u�W�F�N�g�X�V����
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection, XMFLOAT4 colors);
//�I�u�W�F�N�g�`�揈��
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* descHeap,
	D3D12_VERTEX_BUFFER_VIEW& vbview, D3D12_INDEX_BUFFER_VIEW& ibview, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV,
	UINT  numIndices);

