#pragma once
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#define DIRECTiNPUT_VERSION 

#include<wrl.h>
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#include"object3D.h"
#pragma endregion
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma region �X�v���C�g�\����
struct Sprite
{
	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertbuff;
	//�萔�o�b�t�@
	ComPtr<ID3D12Resource> constbuff;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbview{};
	//��������̉�]�p
	float rotation = 0.0f;
	//�A�t�B���ϊ��s��
	XMFLOAT3 scale = { 1,1,1 };
	//���W
	XMFLOAT3 position = { 0,0,0 };
	//���[���h�s��
	XMMATRIX matWorld;
	//�e�N�X�`���ԍ�
	UINT texNumber = 0;
	XMFLOAT4 color;
};
#pragma endregion

//�X�v���C�g�p3D�I�u�W�F�N�g�̃p�C�v���C���֐�
#pragma region ���_�\���̂̒�`
struct VertexPosUv
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};
#pragma endregion

#pragma region ���ʃf�[�^�\����
//�e�N�X�`���̍ő喇��
const int spriteSRVCount = 512;
struct SpriteCommon
{
	//�p�C�v���C���Z�b�g
	PipelineSet pipelineSet;
	//�ˉe�s��
	XMMATRIX matProjection{};
	//�e�N�X�`����p�f�X�N���v�^�̐���
	ComPtr<ID3D12DescriptorHeap>descHeap;
	//�e�N�X�`�����\�[�X�̔z��
	ComPtr<ID3D12Resource>texBuff[spriteSRVCount];
};
#pragma endregion

//�X�v���C�g�̃p�C�v���C���ݒ�
PipelineSet create3DpipelineSprite(ID3D12Device* dev);

//�X�v���C�g�����̏�����
Sprite SpriteCreate(ID3D12Device* dev, int window_width, int window_height);

//�X�v���C�g�P�̍X�V
void SpriteUpdate(Sprite& sprite, const SpriteCommon& spriteCommon);

//�P�̃O���t�B�b�N�X�R�}���h
void SpriteDraw(const Sprite& sprite, ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon, ID3D12Device* dev);

//���ʃO���t�B�b�N�X�R�}���h
void SpriteCommonBeginDraw(const SpriteCommon& spriteCommon, ID3D12GraphicsCommandList* cmdList);

//�X�v���C�g���ʃf�[�^����
SpriteCommon SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height);

//�X�v���C�g���ʃe�N�X�`���ǂݍ���
HRESULT SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev);
