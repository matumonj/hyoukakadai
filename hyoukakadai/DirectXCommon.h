#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dx12.h>
#include <cstdlib>

#include "WinApp.h"
//using namespace DirectX;
using namespace Microsoft::WRL;

class DirectXCommon
{
private:
	WinApp* winapp;
	ComPtr<IDXGIFactory6>dxgiFactory;
	ComPtr<IDXGISwapChain4> swapchain;
	ComPtr<IDXGISwapChain1> swapchain1;
	ComPtr<ID3D12CommandAllocator>cmdAllocator;
	ID3D12GraphicsCommandList* cmdList;
	ComPtr<ID3D12CommandQueue>cmdQueue;
	ComPtr<ID3D12DescriptorHeap>rtvHeaps;
	ComPtr<ID3D12Fence>fence;
	UINT64 fenceVal = 0;
public:

};

