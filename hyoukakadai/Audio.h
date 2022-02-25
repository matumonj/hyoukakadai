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

//チャンクヘッダ
struct ChunkHeader
{
	char id[4];
	int32_t size;
};
//RIFTヘッダチャンク
struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};
//FMTチャンク
struct FormatChunk
{
	ChunkHeader chunk;
	//ChunkHeader data;
	WAVEFORMATEX fmt;
};
//音声データ
struct SoundData
{
	//波型フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファサイズ
	unsigned int bufferSize;
};
//サウンド読み込みと再生の処理をまとめた関数
SoundData SoundLoadWave(const char* filename);

//xaudio初期化処理部分 資料にある2行をまとめたもの
void createXaudioEngine();
//サウンドデータの解放
void SoundUnload(SoundData* soundData);
//サウンド再生
void PlaySoundWave(IXAudio2* xAudio2, const SoundData& soundData);
