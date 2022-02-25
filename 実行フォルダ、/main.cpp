#pragma region include����̐錾
#include<Windows.h>
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

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include"object3D.h"
#include"Sprites.h"
#include"pmdmodel.h"
//#include"Device.h"
#pragma endregion
using namespace DirectX;
using namespace Microsoft::WRL;

static ID3D12Device* g_pd3dDevice = NULL;
static int const NUM_FRAMES_IN_FLIGHT = 3;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#pragma region PMD�w�b�_�[�\����
struct PMDHeader
{
	float version;
	char modelname[20];
	char comment[256];
};
#pragma endregion


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#pragma region DirectX�̏�����
	const int window_width = 900;
	const int window_height = 720;

	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = L"DirectX";
	w.hInstance = GetModuleHandle(nullptr);
	w.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&w);

	RECT wrc = { 0,0,window_width,window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(w.lpszClassName,
		L"DirectX",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr
	);
	ShowWindow(hwnd, SW_SHOW);
	MSG msg{};
	//�Ńo�b�O���C���[�̂��
#ifdef _DEBUG
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();

	}
#endif

#pragma endregion
	//directx������

	HRESULT result;
	ID3D12Device* dev = nullptr;
	//ID3D12Device* dev;
	//IDXGIFactory6* dxgiFactory = nullptr;
	//�ύX
	ComPtr<IDXGIFactory6>dxgiFactory;
	ComPtr<IDXGISwapChain4> swapchain;
	ComPtr<IDXGISwapChain1> swapchain1;
	ComPtr<ID3D12CommandAllocator>cmdAllocator;
	ID3D12GraphicsCommandList* cmdList;
	ComPtr<ID3D12CommandQueue>cmdQueue;
	ComPtr<ID3D12DescriptorHeap>rtvHeaps;

	//dxgi�t�@�N�g���[�̐���
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	//�A�_�v�^�̗񋓗p
	std::vector<ComPtr<IDXGIAdapter1>>adapters;

	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	ComPtr<IDXGIAdapter1>tmpAdapter;
	for (int i = 0; dxgiFactory->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	//�O���t�B�b�N�{�[�h�̃A�_�v�^���2
	for (int i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC1 adesc;
		adapters[i]->GetDesc1(&adesc);
		if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"Intel") == std::wstring::npos) {
			tmpAdapter = adapters[i];
			break;
		}
	}

	//�f�o�C�X�̐���
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;
	for (int i = 0; i < _countof(levels); i++) {
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK) {
			featureLevel = levels[i];
			break;
		}
	}

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));

	//�R�}���h���X�g�̐���
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));

	//�R�}���h�L���[�̐���
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

	//�X���b�v�`�F�[���̐���
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 900;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	dxgiFactory->CreateSwapChainForHwnd(cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, &swapchain1);
	swapchain1.As(&swapchain);

	//�ŃX�N���v�^�q�[�v�̐���
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 2;
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	//�����_�[�^�[�Q�b�gview�̐���
	std::vector<ComPtr<ID3D12Resource>>backBuffers(2);
	for (int i = 0; i < 2; i++) {
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
			i, dev->GetDescriptorHandleIncrementSize(heapDesc.Type));
		//D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//handle.ptr += i * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		dev->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
	}
	//���\�[�X�ݒ�
	ComPtr<ID3D12Resource>depthBuffer;

	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT, window_width, window_height, 1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	//���\�[�X����
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0), IID_PPV_ARGS(&depthBuffer));

	//�[�xview�p�ŃX�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ComPtr<ID3D12DescriptorHeap>dsvHeap;
	result = dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//�t�F���X�̐���
	ComPtr<ID3D12Fence>fence;
	UINT64 fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

#pragma region �L�[�{�[�h�Ȃǂ̓��͏���
	//////�L�[�{�[�h�̏�����
	ComPtr<IDirectInput8>dinput;
	result = DirectInput8Create(w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	////
	//�L�[�{�[�h�f�o�C�X�̍쐬
	////
	ComPtr<IDirectInputDevice8>devkeyboard;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//////�ɂイ��傭�f�[�^�`���̃Z�b�g
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);

	//�r�����䃌�x���̃Z�b�g
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
#pragma endregion

	//�`�揉��������
	//���_�f�[�^3�_���̍��W
#pragma region �`�揉��������
#pragma region ���_�f�[�^�\���̂Ə�����(vertices)	
//���_�f�[�^�\����
	//�O�p���p
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	//�X�v���C�g�p
	struct VertexPosUv
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	//�w�i�p
	struct BackGround
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	//PMD���_�\����
	struct PMDVertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		unsigned  short boneNo[2];
		unsigned char boneWeight;
		unsigned char edgeFlg;
	};

	const float topHeight = 10.0f;
	const int Divs = 3;
	const float radius = 10.0f;
	//�O�p���̒��_�f�[�^
	Vertex vertices[Divs + 1 + 1] = {};

	//for (int i = 0; i < 3; i++) {
	vertices[0].pos.x = radius * sinf(XM_2PI * 0 / Divs);
	vertices[0].pos.y = radius * cosf(XM_2PI * 0 / Divs);
	vertices[0].pos.z = 0;

	vertices[1].pos.x = radius * sinf(XM_2PI * 1 / Divs);
	vertices[1].pos.y = radius * cosf(XM_2PI * 1 / Divs);
	vertices[1].pos.z = 0;

	vertices[2].pos.x = radius * sinf(XM_2PI * 2 / Divs);
	vertices[2].pos.y = radius * cosf(XM_2PI * 2 / Divs);
	vertices[2].pos.z = 0;
	//}eeeewwwww

	vertices[3].pos = { 0,0,0 };

	vertices[4].pos = { 0,0,-topHeight };

	//�w�i�p�̔|���S��
	BackGround background[] = {
		{{-20.4f,0.0f,0.0f},{}, {0.0f,3.0f}},
		{{-20.4f,50.7f,0.0f},{}, {0.0f,0.0f}},
		{{20.0f,0.0f,0.0f},{}, {3.0f,3.0f}},
		{{20.0f,50.7f,0.0f},{}, {3.0f,0.0f}},
	};


#pragma endregion

	
#pragma region �C���f�b�N�X�f�[�^
	//�C���f�b�N�X�f�[�^
	unsigned short indices[3 * Divs * 2];
	indices[0] = 1;
	indices[1] = 0;
	indices[2] = 3;

	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	indices[6] = 0;
	indices[7] = 2;
	indices[8] = 3;

	indices[9] = 0;
	indices[10] = 1;
	indices[11] = 4;

	indices[12] = 1;
	indices[13] = 2;
	indices[14] = 4;

	indices[15] = 2;
	indices[16] = 0;
	indices[17] = 4;

	for (int i = 0; i < _countof(indices) / 3; i++)
	{
		unsigned short indices0 = indices[i * 3 + 0];
		unsigned short indices1 = indices[i * 3 + 1];
		unsigned short indices2 = indices[i * 3 + 2];

		XMVECTOR p0 = XMLoadFloat3(&vertices[indices0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices2].pos);

		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);

		XMVECTOR normal = XMVector3Cross(v1, v2);
		normal = XMVector3Normalize(normal);
		XMStoreFloat3(&vertices[indices0].normal, normal);
		XMStoreFloat3(&vertices[indices1].normal, normal);
		XMStoreFloat3(&vertices[indices2].normal, normal);
	}
#pragma endregion

	//�w�i�p�̔|���S���@�C���f�b�N�X�f�[�^
	unsigned short backgroundindices[] = {
		0,1,2,
		1,2,3,
	};
	//���_�o�b�t�@�̃T�C�Y
	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * _countof(vertices));
	//���_�o�b�t�@�ւ̐���
	ComPtr<ID3D12Resource>vertBuff;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));

	//���z���������擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i];
	}
	//�}�b�v����
	vertBuff->Unmap(0, nullptr);

#pragma region �w�i�p�̔|���S��
	//���_�o�b�t�@�̃T�C�Y
	UINT backsizeVB = static_cast<UINT>(sizeof(BackGround) * _countof(background));
	//���_�o�b�t�@�ւ̐���
	ComPtr<ID3D12Resource>backvertBuff;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&backvertBuff));

	//���z���������擾
	BackGround* backvertMap = nullptr;
	result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
	for (int i = 0; i < _countof(background); i++) {
		backvertMap[i] = background[i];
	}
	//�}�b�v����
	backvertBuff->Unmap(0, nullptr);

#pragma endregion
#pragma region pmd���f��
	//pmd���X
	PMDHeader pmdheader;
	char signatures[3] = {};
	FILE* fp;
	fopen_s(&fp, "Model/directtosentouki2.pmd", "rb");

	fread(signatures, sizeof(signatures), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	
	constexpr size_t pmdvertex_size = 38;
	unsigned int vertnum;
	fread(&vertnum, sizeof(vertnum), 1, fp);

	std::vector<unsigned char>verticess(vertnum* pmdvertex_size);
	fread(verticess.data(), verticess.size(), 1, fp);

	//���_�o�b�t�@�̃T�C�Y
	//UINT pmdsizeVB = static_cast<UINT>(sizeof(PMDVertex) * _countof(pmdvertex_size));
	//���_�o�b�t�@�ւ̐���
	ComPtr<ID3D12Resource>pmdvertBuff;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(verticess.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pmdvertBuff));

	//���z���������擾
	unsigned char* pmdvertMap = nullptr;
	result = pmdvertBuff->Map(0, nullptr, (void**)&pmdvertMap);
	std::copy(std::begin(verticess), std::end(verticess), pmdvertMap);
	//pmdvertBuff->Unmap(0, nullptr);
#pragma region pmd���f��
//���_�o�b�t�@view�̍쐬
	D3D12_VERTEX_BUFFER_VIEW pmdvbview{};

	pmdvbview.BufferLocation = pmdvertBuff->GetGPUVirtualAddress();
	pmdvbview.SizeInBytes = verticess.size();
	pmdvbview.StrideInBytes = pmdvertex_size;;

	//pmdindices
	std::vector<unsigned short>indicess;
	unsigned int indicesNum;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	indicess.resize(indicesNum);
	fread(indicess.data(), indicess.size() * sizeof(indicess[0]), 1, fp);

	//fclose(fp);
	ComPtr<ID3D12Resource>pmdindexBuff;
	UINT pmdsizeIB = sizeof(indicess);

	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indicess.size() * sizeof(indicess[0])), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&pmdindexBuff));


	unsigned short* pmdindexMap = nullptr;
	result = pmdindexBuff->Map(0, nullptr, (void**)&pmdindexMap);

	//�S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < indicess.size(); i++) {
		pmdindexMap[i] = indicess[i];
	}
	pmdindexBuff->Unmap(0, nullptr);

	D3D12_INDEX_BUFFER_VIEW pmdibview = {};
	pmdibview.BufferLocation = pmdindexBuff->GetGPUVirtualAddress();
	pmdibview.Format = DXGI_FORMAT_R16_UINT;
	pmdibview.SizeInBytes = indicess.size() * sizeof(indicess[0]);

#pragma endregion
	//���_�o�b�t�@view�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbview{};

	vbview.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbview.SizeInBytes = sizeVB;
	vbview.StrideInBytes = sizeof(Vertex);

	//�C���f�b�N�X�o�b�t�@�̐ݒ�	
	ComPtr<ID3D12Resource>indexBuff;
	UINT sizeIB = sizeof(indices);

	//�C���f�b�N�X�o�b�t�@�̐���
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeIB), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&indexBuff));

	//���z���������擾
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);

	//�S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(indices); i++) {
		indexMap[i] = indices[i];
	}
	indexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@view�̍쐬
	D3D12_INDEX_BUFFER_VIEW ibview{};
	ibview.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibview.Format = DXGI_FORMAT_R16_UINT;
	ibview.SizeInBytes = sizeof(indices);

#pragma region �w�i�p�̔|���S��
	//���_�o�b�t�@view�̍쐬
	D3D12_VERTEX_BUFFER_VIEW backvbview{};

	backvbview.BufferLocation = backvertBuff->GetGPUVirtualAddress();
	backvbview.SizeInBytes = backsizeVB;
	backvbview.StrideInBytes = sizeof(BackGround);

	//�C���f�b�N�X�o�b�t�@�̐ݒ�	
	ComPtr<ID3D12Resource>backindexBuff;
	UINT backsizeIB = sizeof(backgroundindices);

	//�C���f�b�N�X�o�b�t�@�̐���
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeIB), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&backindexBuff));

	//���z���������擾
	unsigned short* backindexMap = nullptr;
	result = backindexBuff->Map(0, nullptr, (void**)&backindexMap);

	//�S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(backgroundindices); i++) {
		backindexMap[i] = backgroundindices[i];
	}
	backindexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@view�̍쐬
	D3D12_INDEX_BUFFER_VIEW backibview{};
	backibview.BufferLocation = backindexBuff->GetGPUVirtualAddress();
	backibview.Format = DXGI_FORMAT_R16_UINT;
	backibview.SizeInBytes = sizeof(backgroundindices);

#pragma endregion

	//���z���������擾

	//�ˉe�ϊ��s��̍��
	XMMATRIX matprojection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)window_width / window_height, 0.1f, 1000.0f);

	//�r���[�ϊ��s��
	XMMATRIX matview;
	XMFLOAT3 eye(0, 0, -10);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//�J�����̉�]�p
	float angle = 0.0f;

	//���[���h�ϊ��s��
	XMMATRIX matworld0;

	XMFLOAT3 position;
	position = { 0.0f,0.0f,0.0f };

	XMMATRIX matworld1;


	//�f�X�N���v�^�q�[�v�g���p�̕ϐ�
	const int constantBufferNum = 128;
	//�萔�o�b�t�@�p�ŃX�N���v�^�q�[�v�̐���
	ID3D12DescriptorHeap* basicDescHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = constantBufferNum + 1;

	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	auto basicHeapHandle0 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		0, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//2
	auto basicHeapHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		1, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//3d�I�u�W�F�N�g�̐�
	const int object_num = 20;

	Object3d object3ds[object_num];//�G�@�̂悤�Ȃ���
	Object3d player[2];//�v���C���[
	Object3d backgrounds;//�ۗ�
	Object3d backgroundsleft;//�ۗ�
	Object3d backgroundsright;//�ۗ�
	Object3d bullet[3];//�e
	Object3d lazer[20];//���[�U�[�|�C���^�݂����Ȃ��́@���C�Ə����ł����炢��Ȃ����
	Object3d pmdmodel;
#pragma region �I�u�W�F�N�g�̏���������
	//player�p
	for (int i = 0; i < _countof(player); i++) {
		InitializeObject3d(&player[i], i, dev, basicDescHeap);
	}
	InitializeObject3d(&pmdmodel, 1, dev, basicDescHeap);
	//bullet�p
	for (int i = 0; i < _countof(bullet); i++) {
		InitializeObject3d(&bullet[i], i, dev, basicDescHeap);
	}
	//lazer�p
	for (int i = 0; i < _countof(lazer); i++) {
		InitializeObject3d(&lazer[i], i, dev, basicDescHeap);
	}
	//�w�i�p�@���͕ۗ�
	InitializeObject3d(&backgrounds, 1, dev, basicDescHeap);
	InitializeObject3d(&backgroundsleft, 1, dev, basicDescHeap);
	InitializeObject3d(&backgroundsright, 1, dev, basicDescHeap);
	//player����̃I�u�W�F�N�g�p
	for (int i = 0; i < _countof(object3ds); i++) {
		InitializeObject3d(&object3ds[i], i, dev, basicDescHeap);
		if (i >= 0) {
			//�e�q�֌W�؂�
			//object3ds[i].parent = &object3ds[i - 1];
			object3ds[i].scale = { 1.3f,1.3f,1.3f };
			object3ds[i].rotation = { 0.0f,0.0f,0.0f };
		}
	}
#pragma endregion

#pragma region �I�u�W�F�N�g�z�u�Ƃ��X�P�[���ύX
	//�I�u�W�F�N�g�̔z�u
	//�v���C���[�I�u�W�F�N�g
	for (int i = 0; i < _countof(player); i++) {
		player[i].position = { 0,0,0.0f };
	}

	//player[1]��player[0]�̎q�Ƃ��Ď��e�q�֌W ����݂����Ȃ���
	player[1].parent = &player[0];
	player[1].rotation = { 0,0,180 };
	player[1].position = { 0,0,-0 };
	player[0].scale = { 0.3f,0.3f,0.3f };
	//player[1].scale = { 0.3f,0.3f,0.3f };
	//�����z�uz
	object3ds[0].position = { -30.0f,0.0f,110.0f };
	object3ds[1].position = { 50.0f,-50.0f,158.0f };
	object3ds[2].position = { 10.0f,-20.0f,198.0f };
	object3ds[3].position = { -30.0f,0.0f,258.0f };
	object3ds[4].position = { -20.0f,-30.0f,328.0f };
	object3ds[5].position = { -10.0f,40.0f,128.0f };
	object3ds[6].position = { 30.0f,0.0f,298.0f };
	object3ds[7].position = { -20.0f,-5.0f,338.0f };
	object3ds[8].position = { -10.0f,0.0f,388.0f };
	object3ds[9].position = { -30.0f,0.0f,438.0f };
	object3ds[10].position = { -20.0f,0.0f,408.0f };
	object3ds[11].position = { 30.0f,0.0f,528.0f };
	object3ds[12].position = { 50.0f,0.0f,468.0f };
	object3ds[13].position = { 40.0f,0.0f,708.0f };
	object3ds[14].position = { 30.0f,0.0f,528.0f };
	object3ds[15].position = { -50.0f,0.0f,768.0f };
	object3ds[16].position = { 80.0f,0.0f,638.0f };
	object3ds[17].position = { -30.0f,0.0f,828.0f };
	object3ds[18].position = { 100.0f,-30.0f,868.0f };
	object3ds[19].position = { -80.0f,70.0f,688.0f };
	//�E���z�u
	
	//�w�i�̐ݒ�
	backgrounds.position = { -40,-800,800 };
	backgrounds.scale = { 150,150,1 };

	backgroundsleft.position = { -400,-400,80 };
	backgroundsleft.scale = { 100,100,1 };
	backgroundsleft.rotation = { 90,0,0 };
	backgroundsright.position = { -400,-410,4580 };
	backgroundsright.scale = { 100,100,1 };
	backgroundsright.rotation = { 90,0,0 };
	//backgrounds.rotation = { 0,90,0 };
	//lazer�p�̂�@����ɂȂ�悤��
	for (int i = 0; i < _countof(lazer); i++) {
		lazer[i].parent = &player[0];
		lazer[i].scale = { 0.05f,0.05f,2.0f };
		lazer[i].position = { player[0].position.x,player[0].position.y,player[0].position.z + i * 16 };
	}

	//�G�@�������Ă邩����ł邩�ǂ����̃t���O
	int objectarrive[object_num];
	for (int i = 0; i < _countof(objectarrive); i++) {
		objectarrive[i] = 1;
	}
	//�e�p�̃I�u�W�F�N�g�z�u
	bullet[0].position = { player[0].position.x,player[0].position.y,player[0].position.z };
	bullet[1].position = { player[0].position.x,player[0].position.y,player[0].position.z };
	bullet[2].position = { player[0].position.x,player[0].position.y,player[0].position.z };

	for (int i = 0; i < 3; i++) {
		bullet[i].scale = { 0.3f,0.3f,1.0f };
	}

#pragma endregion

	//�V�F�[�_���\�[�X�r���[�̃A�h���X�v�Z����
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	//�n���h���̃A�h���X��i�߂�
	cpuDescHandleSRV.ptr += constantBufferNum * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	gpuDescHandleSRV.ptr += constantBufferNum * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV2 = basicDescHeap->GetGPUDescriptorHandleForHeapStart();
	gpuDescHandleSRV2.ptr += constantBufferNum * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//���ʃf�[�^�̊֐��Ăяo��
	SpriteCommon spritecommon;
	//�X�v���C�g���ʃf�[�^����
	spritecommon = SpriteCommonCreate(dev, window_width, window_height);
	//�X�v���C�g�p�e�N�X�`���ǂݍ��݂̊֐��Ăяo��
	SpriteCommonLoadTexture(spritecommon, 0, L"Resources/hart.png", dev);
	SpriteCommonLoadTexture(spritecommon, 1, L"Resources/DQIX_-_Cruelcumber.png", dev);
	SpriteCommonLoadTexture(spritecommon, 2, L"Resources/gameoversprite.png", dev);
	SpriteCommonLoadTexture(spritecommon, 3, L"Resources/tittlesprite.png", dev);
	SpriteCommonLoadTexture(spritecommon, 4, L"Resources/clearsprite.png", dev);
	//�X�v���C�g�������֐��̌Ăяo��
	Sprite sprite[7];
	//�X�v���C�g�̐���
	for (int i = 0; i < 7; i++) {
		sprite[i] = SpriteCreate(dev, window_width, window_height);
	}
	//WIC�e�N�X�`���̃��[�h
	//�摜�t�@�C���̗p��
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(L"Resources/Tabizine_160808_1.jpg", WIC_FLAGS_NONE, &metadata, scratchImg);

	const Image* img = scratchImg.GetImage(0, 0, 0);


	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, metadata.width,
		static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	////3
	ComPtr<ID3D12Resource>texBuff = nullptr;
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0), D3D12_HEAP_FLAG_NONE, &texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texBuff));

	//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	result = texBuff->WriteToSubresource(0, nullptr, img->pixels, static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch));

	////3
	auto basicHeapHandle2 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		2, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));


	////�V�F�[�_���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	srvDesc.Format = metadata.format;
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	dev->CreateShaderResourceView(texBuff.Get(), &srvDesc, cpuDescHandleSRV);

	auto gpuDescHandle0 = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		0, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	////2
	auto gpuDescHandle1 = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		1, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	////3
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle2 = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		2, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	);
	float angles = 0;
	float angles2 = 0;
	//3D�I�u�W�F�N�g�p�p�C�v���C������
	PipelineSet obj3Dpipelineset = create3Dpipeline(dev);
	//�X�v���C�g�p�p�C�v���C���ݒ�
	PipelineSet spriteobj3Dpipelineset = create3DpipelineSprite(dev);
	//for (int i = 0; i < 4; i++) {
	PipelineSet pmdcreatepipeline = create3Dpipelinepmd(dev);
	//	sprite[i].rotation = 45;
	//}
	pmdmodel.position = { 10,-15,-50 };
	pmdmodel.rotation = { 0,180,0 };
	pmdmodel.scale = { 0.5f,0.5f,0.5f };
	//�X�v���C�g�̔z�u
	sprite[0].position = { 20,12,0 };
	sprite[1].position = { 84,12,0 };
	sprite[2].position = { 148,12,0 };
	sprite[3].position = { 702,12,0 };
	sprite[4].position = { 0,0,0 };
	sprite[4].scale = { 14.5,12,0 };
	sprite[5].position = { 0,0,0 };
	sprite[5].scale = { 14.5,12,0 };
	sprite[6].position = { 0,0,0 };
	sprite[6].scale = { 14.5,12,0 };
	//�X�v���g���ƂɃe�N�X�`�����w��
	//�e�N�X�`��0�����C�t�̉摜
	//�e�N�X�`��1���Y�b�L�[�j���̉摜
	sprite[0].texNumber = 0;
	sprite[1].texNumber = 0;
	sprite[2].texNumber = 0;
	sprite[3].texNumber = 1;
	sprite[4].texNumber = 2;
	sprite[5].texNumber = 3;
	sprite[6].texNumber = 4;
	//SpriteUpdate(sprite, spritecommon);
#pragma endregion

	int bulletflag[3] = { 0,0,0 };
	int bulletflag2 = 0;
	int bulletflag3 = 0;
	float colorr = 0.0f;
	float backcolor1 = 0.1f;
	float backcolor2 = 0.1f;
	float backcolor3 = 0.1f;
	int retry = 0;
	//�I�u�W�F�N�g�̍X�V�����Ŏg���������̐F���p
	XMFLOAT4 RedColor = { 1,0.6,0.6,0.7f };
	XMFLOAT4 BlueColor = { 0.5,0.5f,0.5f,1 };
	XMFLOAT4 GreenColor = { 0,1,0,1 };
	int timer = 0;
	int playerHP = 3;
	float lazertoobj[20];
	float BulletSpeed = 9.6f;
	float angle2 = 80.110001f;
	float angle3 = 80.110001f;
	float angle4 = 80.110001f;
	float d = 20;
	int gameoverflag = 0;
	XMVECTOR v0 = { 0,1,-10,0 };
	XMMATRIX rotM;// = XMMatrixIdentity();
	//XMVECTOR v;
	XMVECTOR eye2;
	XMVECTOR target2 = { player[0].position.x, player[0].position.y, player[0].position.z, 0 };

	XMVECTOR up2 = { 0,0.3f,0,0 };

	int scene = 1;
	
	while (true) {
		//�E�B���h�E���b�Z�[�W����
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		//�O���t�B�b�N�̂�e�Ƃ�
		//���t���[������
		//�L�[�{�[�h����
		result = devkeyboard->Acquire();
		BYTE key[256] = {};

		result = devkeyboard->GetDeviceState(sizeof(key), key);
		

		if (gameoverflag == 1) {
			scene = 3;
		}
		if (scene == 3) {
			playerHP = 3;
			//timer++;
			//sprite[4].position = { 900,900,900 };
			backcolor1 = 0.1f;
			backcolor2 = 0.1f;
			backcolor3 = 0.1f;
			pmdmodel.position.x = 10;
			pmdmodel.position.y = -15;
			pmdmodel.position.z = -50 ;
			//�����z�uz
			object3ds[0].position.z =90.0f ;
			object3ds[1].position.z = 158.0f ;
			object3ds[2].position.z = 138.0f ;
			object3ds[3].position.z = 158.0f ;
			object3ds[4].position.z = 178.0f ;
			object3ds[5].position.z = 198.0f ;
			object3ds[6].position.z = 218.0f ;
			object3ds[7].position.z = 238.0f;
			object3ds[8].position.z = 258.0f;
			object3ds[9].position.z = 278.0f ;
			object3ds[10].position.z =298.0f ;
			//�E���z�u
			for (int i = 0; i < object_num; i++) {
				objectarrive[i] = 1;
			}
			//�w�i�̐ݒ�
			backgrounds.position.z = 800;
			//backgrounds.scale = { 150,150,1 };

			backgroundsleft.position.z =80 ;
		//	backgroundsleft.scale = { 100,100,1 };
			//backgroundsleft.rotation = { 90,0,0 };
			backgroundsright.position.z = 4580 ;
			//backgroundsright.scale = { 100,100,1 };
			//backgroundsright.rotation = { 90,0,0 };
		//	gameoverflag = 0;
			//retry = 0;
			
		}
		if (scene == 1) {
			if (key[DIK_C]) {
				scene = 2;
				
			}
			
		}
		if (scene==3) {
			if (key[DIK_R]) {
				scene = 2;
			}
				//scene = 2;
		}
		
		////�w�i�F�̐ݒ�
		float clearColor[] = { 1.0f,1.0f,1.0f,0.0f };
		//�w�i������������悤��
		if (scene == 2) {
			if (backcolor1 <= 1.0) {
				backcolor1 += 0.005f;
			}
			if (backcolor2 <= 1.0) {
				backcolor2 += 0.005f;
			}
			if (backcolor3 <= 1.0) {
				backcolor3 += 0.005f;
			}

			if (backgrounds.position.z > 100) {
				//backgrounds.position.z -= 0.02f;
			}
			pmdmodel.position.z += 0.02f;
			pmdmodel.position.x += angles / 15;
			if (pmdmodel.position.y > -205) {
				pmdmodel.position.y += angles2 / 15;
			}
			//if (backgroundsleft.position.z > -300) {
			backgroundsleft.position.z -= 3.32f;
			//}
		//	if (backgroundsright.position.z > -300) {
			backgroundsright.position.z -= 3.32f;
			//	}
			if (backgroundsleft.position.z < -4500) {
				backgroundsleft.position.z = 800;
			}
			if (backgroundsright.position.z < -4500) {
				backgroundsright.position.z = 800;
			}
#pragma region �ړ������Ƃ�
			pmdmodel.rotation.z = angles;
			pmdmodel.rotation.x = angles2;
			//�ړ��̂��

			//pmdmodel.position.z += 0.4f;
			if (key[DIK_UP] || key[DIK_DOWN] || key[DIK_LEFT] || key[DIK_RIGHT]) {
				if (key[DIK_UP]) {

					pmdmodel.position.y += 1.0f;
					angles2 += XMConvertToRadians(15);//���@�̉�]
				}
				if (key[DIK_DOWN]) {
					angles2 -= XMConvertToRadians(15);//���@�̉�]
					//if (pmdmodel.position.y > -205) {
						pmdmodel.position.y -= 1.0f;
					//}
				}
				if (key[DIK_LEFT]) {
					pmdmodel.position.x -= 1.0f;
					angles -= XMConvertToRadians(15.0f);//���@�̉�]
				}
				if (key[DIK_RIGHT]) {
					pmdmodel.position.x += 1.0f;
					angles += XMConvertToRadians(15.0f);//���@�̉�]
				}

				
				//���@�̉�]�ɐ�������
				angles = min(angles, XMConvertToRadians(1800));
				angles = max(angles, XMConvertToRadians(-1800));

				angles2 = min(angles2, XMConvertToRadians(900));
				angles2 = max(angles2, XMConvertToRadians(-900));
			}
			pmdmodel.position.x = min(pmdmodel.position.x, 336);
			pmdmodel.position.x = max(pmdmodel.position.x, -450);
			pmdmodel.position.y = min(pmdmodel.position.y, 1336);
			pmdmodel.position.y = max(pmdmodel.position.y, -205);

			//�J�����̒����_���v���C���[�̈ʒu�ɌŒ�
			target2.m128_f32[2] = pmdmodel.position.z;
			target2.m128_f32[0] = pmdmodel.position.x;
			target2.m128_f32[1] = pmdmodel.position.y;
			//�s�����蒼��
			rotM = XMMatrixRotationX(XMConvertToRadians(angle));
			XMVECTOR v;
			v = XMVector3TransformNormal(v0, rotM);
			eye2 = target2 + v;
			matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

#pragma endregion 

#pragma region �I�u�W�F�N�g�X�V����
			//����̃I�u�W�F�N�g���������ɂ���ė���悤��
			for (int i = 0; i < _countof(object3ds); i++) {
				if (objectarrive[i] == 1) {
					object3ds[i].position.z -= 0.5f;
				}
			}

			//�G�p�̍X�V����
			for (int i = 0; i < _countof(object3ds); i++) {
				if (object3ds[i].position.z > pmdmodel.position.z + 300.0f) {
					UpdateObject3d(&object3ds[i], matview, matprojection, RedColor);
				}
				//�˒��͈͂ɓ�������F�Z��
				else if (object3ds[i].position.z <= pmdmodel.position.z + 300.0f) {
					UpdateObject3d(&object3ds[i], matview, matprojection, { 1,0,0,1 });
				}
			}

			//player�p�̍X�V����
			for (int i = 0; i < _countof(player); i++) {
				UpdateObject3d(&player[i], matview, matprojection, BlueColor);
			}

			//bullet�p�̍X�V����
			for (int i = 0; i < _countof(bullet); i++) {
				UpdateObject3d(&bullet[i], matview, matprojection, BlueColor);
			}

			//�w�i�p�̍X�V�����@���͕ۗ�
			UpdateObject3d(&backgrounds, matview, matprojection, { backcolor1,backcolor2,backcolor3,1 });
			UpdateObject3d(&backgroundsleft, matview, matprojection, { 1,1,1,0.04 });
			UpdateObject3d(&backgroundsright, matview, matprojection, { 1,1,1,0.05 });
			//pmdmodel�\��
			UpdateObject3d(&pmdmodel, matview, matprojection, { 0,0,0,0 });

			//�X�v���C�g�̍X�V�����Ăяo��
			for (int i = 0; i < 5; i++) {
				SpriteUpdate(sprite[i], spritecommon);
			}
			

#pragma endregion

#pragma region �e�ł����Ɠ����蔻��
			//object3ds[20],[22],[23]�������ł͒e�p�I�u�W�F�N�g
			//�e�P�̔��ˏ���
			for (int i = 0; i < 3; i++) {
				if (bulletflag[i] == 0) {
					//�����ʒu���v���C���[�ʒu��
					bullet[i].position.x = pmdmodel.position.x;
					bullet[i].position.y = pmdmodel.position.y - 5;
					bullet[i].position.z = pmdmodel.position.z + 20;
					//bullet[i].position.z = bullet[i - 1].position.z - 50;
					bulletflag[i] = 1;
					break;
				}
				if (bulletflag[i] == 1) {
					//�e�I�u�W�F�N�g��z���W�����ȏ㒴������t���O���O��

					bullet[i].position.z += BulletSpeed;
					bullet[i].position.x += cos(angle2) * BulletSpeed;//�e�̈ړ�

					if (bullet[i].position.z > pmdmodel.position.z + 300.0f) {
						bulletflag[i] = 0;
					}
				}
			}

#pragma region �Փ˔��蕔��
			//�G�@�ƒe[0]�̏Փ˔���
			for (int i = 0; i < _countof(objectarrive); i++) {
				if (objectarrive[i] == 1 && bulletflag[0] == 1)
				{
					float length = sqrtf(((object3ds[i].position.x - bullet[0].position.x) * (object3ds[i].position.x - bullet[0].position.x)) + ((object3ds[i].position.y - bullet[0].position.y) * (object3ds[i].position.y - bullet[0].position.y)) + ((object3ds[i].position.z - bullet[0].position.z) * (object3ds[i].position.z - bullet[0].position.z)));
					if (length <= 10)
					{
						objectarrive[i] = 0;
						bulletflag[0] = 0;
					}
				}
			}
			//�G�@�ƒe[1]�̏Փ˔���
			for (int i = 0; i < _countof(objectarrive); i++) {
				if (objectarrive[i] == 1 && bulletflag[1] == 1)
				{
					float length2 = sqrtf(((object3ds[i].position.x - bullet[1].position.x) * (object3ds[i].position.x - bullet[1].position.x)) + ((object3ds[i].position.y - bullet[1].position.y) * (object3ds[i].position.y - bullet[1].position.y)) + ((object3ds[i].position.z - bullet[1].position.z) * (object3ds[i].position.z - bullet[1].position.z)));
					if (length2 <= 10)
					{
						objectarrive[i] = 0;
						bulletflag[1] = 0;
					}
				}
			}
			//�G�@�ƒe[2]�̏Փ˔���
			for (int i = 0; i < _countof(objectarrive); i++) {
				if (objectarrive[i] == 1 && bulletflag[2] == 1)
				{
					float length3 = sqrtf(((object3ds[i].position.x - bullet[2].position.x) * (object3ds[i].position.x - bullet[2].position.x)) + ((object3ds[i].position.y - bullet[2].position.y) * (object3ds[i].position.y - bullet[2].position.y)) + ((object3ds[i].position.z - bullet[2].position.z) * (object3ds[i].position.z - bullet[2].position.z)));
					if (length3 <= 10)
					{
						objectarrive[i] = 0;

						bulletflag[2] = 0;
					}
				}
			}
#pragma endregion
#pragma region �G�̒e�����̒l�������烉�C�t���P���炷
			for (int i = 0; i < _countof(objectarrive); i++) {
				if (object3ds[i].position.z <= pmdmodel.position.z - 50) {
					object3ds[i].position.z = 790;
					playerHP -= 1;
				}
			}
		}
#pragma endregion
			//background->pos.z -= 0.5f;
#pragma endregion

		//���z���������擾
			Vertex* vertMap = nullptr;
			result = vertBuff->Map(0, nullptr, (void**)&vertMap);
			for (int i = 0; i < _countof(vertices); i++) {
				vertMap[i] = vertices[i];
			}
			vertBuff->Unmap(0, nullptr);

#pragma endregion
#pragma region �w�i�p�̔|���S��
			//���z���������擾
			BackGround* backvertMap = nullptr;
			result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
			for (int i = 0; i < _countof(background); i++) {
				backvertMap[i] = background[i];
			}
			backvertBuff->Unmap(0, nullptr);

			//pragma region �w�i�p�̔|���S��
			//���z���������擾
			unsigned char* pmdvertMap = nullptr;
			result = pmdvertBuff->Map(0, nullptr, (void**)&pmdvertMap);
			std::copy(std::begin(verticess), std::end(verticess), pmdvertMap);

			//pmdvertBuff->Unmap(0, nullptr);
			//x,y���W�̃f�o�b�O���O
			wchar_t str[256];
			//for (int i = 0; i < _countof(vertices4); i++) {
			swprintf_s(str, L"position:%f\n", pmdmodel.position.y);
			//}
			OutputDebugString(str);
			swprintf_s(str, L"objectZ:%f\n", object3ds[3].position.z);
			OutputDebugString(str);


			UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
			//���s
			cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT));
			//�`���w��
			auto rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, dev->GetDescriptorHandleIncrementSize(heapDesc.Type));

			//�[�x�X�e���V���r���[�p�ŃX�N���v�^�q�[�v
			D3D12_CPU_DESCRIPTOR_HANDLE dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
			cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

			//��ʃN���A
			cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
			cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

#pragma region �`��R�}���h
			//�`��R�}���h
			cmdList->SetPipelineState(obj3Dpipelineset.pipelinestate.Get());
			cmdList->SetGraphicsRootSignature(obj3Dpipelineset.rootsignature.Get());

			//�X�v���C�g���ʃR�}���h
			//cmdList->SetPipelineState(pmdcreatepipeline.pipelinestate.Get());
			//cmdList->SetGraphicsRootSignature(pmdcreatepipeline.rootsignature.Get());
			cmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, window_width, window_height));

			cmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, window_width, window_height));

			//�v���e�B���`��̐ݒ�R�}���h
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//�`�揈���̌Ăяo��

			if (scene != 3) {
				for (int i = 0; i < _countof(object3ds); i++) {
					if (objectarrive[i] == 1) {
						DrawObject3d(&object3ds[i], cmdList, basicDescHeap, vbview, ibview, gpuDescHandleSRV, _countof(indices));
					}
				}
				//bullet�̕`��
				for (int i = 0; i < _countof(bullet); i++) {
					DrawObject3d(&bullet[i], cmdList, basicDescHeap, vbview, ibview, gpuDescHandleSRV, _countof(indices));
				}
			}

			//player�̕`��
			for (int i = 0; i < _countof(player); i++) {
			//	DrawObject3d(&player[i], cmdList, basicDescHeap, vbview, ibview, gpuDescHandleSRV, _countof(indices));
			}


			//lazer�̕`��
			for (int i = 0; i < _countof(lazer); i++) {
				//DrawObject3d(&lazer[i], cmdList, basicDescHeap, vbview, ibview, gpuDescHandleSRV, _countof(indices));
			}
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			DrawObject3d(&backgrounds, cmdList, basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
			DrawObject3d(&backgroundsleft, cmdList, basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
			DrawObject3d(&backgroundsright, cmdList, basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);

			//�X�v���C�g�`��
			SpriteCommonBeginDraw(spritecommon, cmdList);//,basicDescHeap);

			//�v���C���[�̗͕̑\��
			if (playerHP != 0) {
				if (playerHP == 3) {
					//if (playerHP <= 180&& playerHP > 120) {
					SpriteDraw(sprite[0], cmdList, spritecommon, dev);
					SpriteDraw(sprite[1], cmdList, spritecommon, dev);
					SpriteDraw(sprite[2], cmdList, spritecommon, dev);
				}//
				else if (playerHP == 2) {
					//else if (playerHP <= 120&&playerHP > 60) {
					SpriteDraw(sprite[0], cmdList, spritecommon, dev);
					SpriteDraw(sprite[1], cmdList, spritecommon, dev);
				} else if (playerHP == 1) {
					//else if (playerHP <= 60 && playerHP > 0) {
					SpriteDraw(sprite[0], cmdList, spritecommon, dev);
				}

			}



			//�v���C���[�̗̑͂��Ȃ��Ȃ�����w�i�F�ύX
			if (playerHP <= 0) {
				scene = 3;
			}
			if (scene==3) {
				backcolor1 = 1.0f;
				backcolor2 = 0.25f;
				backcolor3 = 0.12f;
				SpriteDraw(sprite[4], cmdList, spritecommon, dev);
				
			}
			if (scene == 1)
			{
				SpriteUpdate(sprite[5], spritecommon);
				SpriteDraw(sprite[5], cmdList, spritecommon, dev);
			}
			if (objectarrive[0] == 0 && objectarrive[1] == 0 && objectarrive[2] == 0 && objectarrive[3] == 0 && objectarrive[4] == 0 && objectarrive[5] == 0 && objectarrive[6] == 0 &&
				objectarrive[7] == 0 && objectarrive[8] == 0 && objectarrive[9] == 0 && objectarrive[10] == 0 && objectarrive[11] == 0 && objectarrive[12] == 0 &&
				objectarrive[13] == 0&&objectarrive[14] == 0 && objectarrive[15] == 0 && objectarrive[16] == 0 && objectarrive[17] == 0 &&
				objectarrive[18] == 0 && objectarrive[19] == 0 ) {
				scene = 4;
			}
			if (scene == 4) {
				SpriteUpdate(sprite[6], spritecommon);
				SpriteDraw(sprite[6], cmdList, spritecommon, dev);
				//if (DIK_H) {
					//scene = 1;
					//sprite[6].position = { 900,900,900 };
				//}
			}

			//�O���t�B�b�N�X�R�}���h�֐��̌Ăяo��
			cmdList->SetPipelineState(pmdcreatepipeline.pipelinestate.Get());
			cmdList->SetGraphicsRootSignature(pmdcreatepipeline.rootsignature.Get());
			//�v���e�B���`��̐ݒ�R�}���h
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			DrawObject3d(&pmdmodel, cmdList, basicDescHeap, pmdvbview, pmdibview, gpuDescHandleSRV, indicesNum);
		
		
		//SpriteCommonBeginDraw(spritecommon, cmdList,basicDescHeap);//,basicDescHeap);
		//�`��R�}���h�@�����܂�
#pragma endregion
		//���\�[�X�o���A��߂�
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

		cmdList->Close();

		ID3D12CommandList* cmdLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(1, cmdLists);
		cmdQueue->Signal(fence.Get(), ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		cmdAllocator->Reset();
		cmdList->Reset(cmdAllocator.Get(), nullptr);
		swapchain->Present(1, 0);

	}

	UnregisterClass(w.lpszClassName, w.hInstance);
	OutputDebugStringA("Hello,jjjj\n");

	return 0;
}

