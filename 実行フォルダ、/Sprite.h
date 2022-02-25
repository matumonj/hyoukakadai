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
#pragma region �X�v���C�g�\����
	struct Sprites
	{
		//���_�o�b�t�@
		ComPtr<ID3D12Resource> vertbuff;
		//�萔�o�b�t�@
		ComPtr<ID3D12Resource> constbuff;
		//���_�o�b�t�@�r���[
		D3D12_VERTEX_BUFFER_VIEW vbview{};
		//��������̉�]�p
		float rotation = 0.0f;
		//���W
		XMFLOAT3 position = { 0,0,0 };
		//���[���h�s��
		XMMATRIX matWorld;
		//�e�N�X�`���ԍ�
		UINT texNumber = 0;
	};
#pragma endregion

#pragma region ���ʃf�[�^�\����
	//�e�N�X�`���̍ő喇��
	
	struct SpriteCommon
	{
		//�p�C�v���C���Z�b�g
		//PipelineSet pipelineSet;
		//�ˉe�s��
		XMMATRIX matProjection{};
		//�e�N�X�`����p�f�X�N���v�^�̐���
		ComPtr<ID3D12DescriptorHeap>descHeap;
		//�e�N�X�`�����\�[�X�̔z��
		ComPtr<ID3D12Resource>texBuff[spriteSRVCount];
	};
#pragma endregion
	private:


};

