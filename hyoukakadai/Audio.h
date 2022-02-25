#pragma once
#include"xaudio2.h"
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"xaudio2.lib")
#pragma endregion

#include<fstream>

//ComPtr<IXAudio2>xAudio2;
//IXAudio2MasteringVoice* masterVoice;
//HRESULT result;

//�`�����N�w�b�_
struct ChunkHeader
{
	char id[4];
	int32_t size;
};
//RIFT�w�b�_�`�����N
struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};
//FMT�`�����N
struct FormatChunk
{
	ChunkHeader chunk;
	//ChunkHeader data;
	WAVEFORMATEX fmt;
};
//�����f�[�^
struct SoundData
{
	//�g�^�t�H�[�}�b�g
	WAVEFORMATEX wfex;
	//�o�b�t�@�̐擪�A�h���X
	BYTE* pBuffer;
	//�o�b�t�@�T�C�Y
	unsigned int bufferSize;
};
//�T�E���h�ǂݍ��݂ƍĐ��̏������܂Ƃ߂��֐�
SoundData SoundLoadWave(const char* filename);

//xaudio�������������� �����ɂ���2�s���܂Ƃ߂�����
void createXaudioEngine();
//�T�E���h�f�[�^�̉��
void SoundUnload(SoundData* soundData);
//�T�E���h�Đ�
void PlaySoundWave(IXAudio2* xAudio2, const SoundData& soundData);
