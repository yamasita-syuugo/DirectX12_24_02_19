#include"UseHeader.h"
#include"value.h"

using namespace std;

void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // _DEBUG
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
HRESULT result;
#define HRESULT(function) result = function;if(result != S_OK){ cout << hex/*16�i��*/ << result << endl; getchar(); return 1;}




#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif // _DEBUG

	//window����-------------------------------------------------------------------------
	HWND hwnd; 
	WNDCLASSEX w = {}; {
		w.cbSize = sizeof(WNDCLASSEX); w.lpfnWndProc = (WNDPROC)WindowProcedure; w.lpszClassName = TEXT("a"); w.hInstance = GetModuleHandle(nullptr); 
		RegisterClassEx(&w);
		RECT wrc = { WINDOW_LEFT,WINDOW_TOP,WINDOW_WIDTH,WINDOW_HEIGHT };
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
		hwnd = CreateWindow(w.lpszClassName, TEXT("DX12�e�X�g : 24/02/19"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, w.hInstance, nullptr);
		ShowWindow(hwnd, SW_SHOW);
	}

	//d3d12�ϐ��쐬-----------------------------------------------------------------------
	// //ID3D12Device�쐬------------------------------------
	ComPtr<ID3D12Device> _dev = nullptr; {
		D3D_FEATURE_LEVEL featureLevel;
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0,D3D_FEATURE_LEVEL_9_3 ,D3D_FEATURE_LEVEL_9_2 ,D3D_FEATURE_LEVEL_9_1 ,
		};
		for (auto lv : levels)if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) { featureLevel = lv; break; }
	}
	//IDXGIFactory7�쐬-----------------------------------
	ComPtr<IDXGIFactory7> _dxgiFactory = nullptr;
	HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));
	//ID3D12CommandAllocator�쐬--------------------------
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	HRESULT(_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator)));
	//ID3D12CommandList�쐬-------------------------------
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	HRESULT(_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList)));
	//ID3D12CommandQueue�쐬------------------------------
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr; {
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {}; cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		HRESULT(_dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue)));
	}
	//IDXGISwapChain4�쐬---------------------------------
	ComPtr<IDXGISwapChain3> _swapchain = nullptr; {
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {}; swapchainDesc.Width = WINDOW_WIDTH; swapchainDesc.Height = WINDOW_HEIGHT; swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.Stereo = false; swapchainDesc.SampleDesc.Count = 1; swapchainDesc.SampleDesc.Quality = 0; swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swapchainDesc.BufferCount = 2; swapchainDesc.Scaling = DXGI_SCALING_STRETCH; swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		HRESULT(_dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)_swapchain.GetAddressOf()));
	}
	//ID3D12DescriptorHeap�쐬---------------------------
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr; {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.NodeMask = 0; heapDesc.NumDescriptors = 2; heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT(_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps)));
	}

	//d3d12�ϐ��̒��g�쐬-----------------------------------------------------------------
#if 0//�O���{����������ꍇ�̑I���
	//�A�_�v�^�[�̗�------------------------------------
	vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != string::npos) {
			tmpAdapter = adpt;//D3D12CreateDevice�̑������Ŏg�p
			break;
		}
	}
#endif
	//IDXGISwapChain4�ƕR�Â� 
	DXGI_SWAP_CHAIN_DESC swcDesc = {}; { HRESULT(_swapchain->GetDesc(&swcDesc)); }
	vector<ComPtr<ID3D12Resource>> _backBuffers(swcDesc.BufferCount);
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart(); {
			for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {
				HRESULT(_swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx])));
				handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				_dev->CreateRenderTargetView(_backBuffers[idx].Get(), &rtvDesc, handle);
			}
		}
	}
	//�e�N�X�`���f�[�^�쐬----------------------------------
	//vector<TexRGBA> texturedata(256 * 256);
	//for (auto& rgba : texturedata) {
	//	rgba.R = rand() % 256;
	//	rgba.G = rand() % 256;
	//	rgba.B = rand() % 256;
	//	rgba.A = 255;
	//}

	//���f���擾--------------------------------------------
	//�E�w�b�_�[���
	//�E���_���
	//�E���_�Ή����
	//�E�}�e���A�����
	//..etcetc
	//	�y�w�b�_�[���z
	//	unsigned char magic[3];
	//	float version;
	//	char model_name[20];
	//	char comment[256];
	//�y���_���z
	//	float pos[3];
	//	float normal_vec[3];
	//	float uv[2];�@�@ //�e�N�X�`���}�b�s���O���W
	//	unsigned short born_num[2]; //�{�[���ԍ�
	//	unsigned char bone_weight; //�{�[���̏d�� (�{�[��1�ɗ^����e���x(0 ~ 100)�@�{�[��2�ɗ^����e���x 100 - bone_weight)
	//	unsigned char edge_flag; //�֊s�����L���̏ꍇ
	//�y���_�W�����z
	//	unsigned count;
	//	PMD_Vertex* vertexs; //���_�Q
	//	count: �|���S����
	//	indexes : unsigned)���_3�y�A* �|���S����
	//�y�}�e���A�����z
	//	float diffuse[4]; //�f�B�t���[�Y��
	//	float power; //���ˋ��x
	//	float specular[3]; //�X�y�L�����[��
	//	float emissive[3]; //�G�~�b�V�u��
	//	unsigned char toon_index; //�g�D�[���E�C���f�b�N�X�ԍ�
	//	unsigned char edge_flag; //�G�b�W�t���O
	//	unsigned long face_vert_count; //�ʒ��_��
	//	char texture_file_name[20]; //�e�N�X�`���t�@�C����
	char signature[3] = {};
	PMDHeader pmdHeader;
	FILE *fp;
	fopen_s(&fp,"Model/�����~�Nmetal.pmd", "rb");
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdHeader, sizeof(pmdHeader), 1, fp);
	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);
	constexpr size_t pmdvertex_size = 38;
	vector<PMDVertex> vertices(vertNum);
	for (int i = 0; i < vertNum; i++)fread(&vertices[i], pmdvertex_size, 1, fp);

	//unsigned int indicesNum;//�C���f�b�N�X��
	//fread(&indicesNum, sizeof(indicesNum), 1, fp);
	//vector<unsigned short> indices(indicesNum);
	//fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);
	fclose(fp);


	//���W�v�Z----------------------------------------------
	//Vertex vertices[] = {
	//	{{-1.0f,-1.0f,0.0f},{0.0f,1.0f}},
	//	{{-1.0f,1.0f,0.0f},	{0.0f,0.0f}},
	//	{{1.0f,-1.0f,0.0f},{1.0f,1.0f}},
	//	{{1.0f,1.0f,0.0f},	{1.0f,0.0f}},

	//	{{100.0f,200.0f,0.0f},{0.0f,1.0f}},
	//	{{100.0f,100.0f,0.0f},	{0.0f,0.0f}},
	//	{{200.0f,200.0f,0.0f},{1.0f,1.0f}},
	//	{{200.0f,100.0f,0.0f},	{1.0f,0.0f}},

	//};
	//�C���f�b�N�X�̎���------------------------------------
	unsigned short indices[] = {
		1,0,2,
		2,1,3,

		4,5,6,
		6,5,7,
	};
	TexMetadata metadate = {};
	ScratchImage scratchImg = {};
	HRESULT(LoadFromWICFile(L"img/�_�E�����[�h.jfif", WIC_FLAGS_NONE, &metadate, scratchImg));	//C:\Users\syuugo_main\source\repos\DirectX12_24_02_19\DirectX12_24_02_19\img
	vector<Image> img;
	img.push_back(*scratchImg.GetImage(0, 0, 0));

	D3D12_RESOURCE_DESC resDescBuf = {}; {
		resDescBuf.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; resDescBuf.Width = sizeof(vertices); resDescBuf.Height = 1; resDescBuf.DepthOrArraySize = 1; resDescBuf.MipLevels = 1;
		resDescBuf.Format = DXGI_FORMAT_UNKNOWN; resDescBuf.SampleDesc.Count = 1; resDescBuf.Flags = D3D12_RESOURCE_FLAG_NONE; resDescBuf.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	}

	//���_�o�b�t�@�[�쐬--------------------------------
	ID3D12Resource* vertBuff = nullptr;
	ID3D12Resource* idxBuff = nullptr; {
		D3D12_HEAP_PROPERTIES heapprop = {}; heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDescBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff)));
		HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDescBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff)));
	}
	//���_���R�s�[------------------------------------
	//vertices----------------------
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; {
		PMDVertex* vertMap = nullptr;
		HRESULT(vertBuff->Map(0, nullptr, reinterpret_cast<void**>(& vertMap)));
		PMDVertex tmp[15000];
		for (int i = 0; i < vertNum; i++)tmp[i] = vertices[i];
		//memcpy(vertMap, tmp, vertices.size());
		copy(&vertices[0], &vertices[204], vertMap);
		vertBuff->Unmap(0, nullptr);
		vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
		vbView.SizeInBytes = vertices.size();
		vbView.StrideInBytes = pmdvertex_size;
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
	}
	//indices-----------------------
	D3D12_INDEX_BUFFER_VIEW ibView = {};  {
		unsigned short* mappedIdx = nullptr;
		HRESULT(idxBuff->Map(0, nullptr, (void**)&mappedIdx));
		copy(/*&*/indices/*[0]*/, /*&*/end(indices)/*[2048]*/, mappedIdx);
		idxBuff->Unmap(0, nullptr);
		ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
		ibView.SizeInBytes = sizeof(indices);
		ibView.Format = DXGI_FORMAT_R16_UINT;
		_cmdList->IASetIndexBuffer(&ibView);
	}

	//�e�N�X�`���o�b�t�@�[�쐬--------------------------
	ID3D12Resource* texbuff = nullptr; {
		D3D12_RESOURCE_DESC resDesc = {}; {
			resDesc.Format = metadate.format;//DXGI_FORMAT_R8G8B8A8_UNORM;
			resDesc.Width = metadate.width;//256;
			resDesc.Height = metadate.height;//256;
			resDesc.DepthOrArraySize = metadate.arraySize;//1;
			resDesc.SampleDesc.Count = 1;
			resDesc.SampleDesc.Quality = 0;
			resDesc.MipLevels = metadate.mipLevels;//1;
			resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadate.dimension);//D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		}
		D3D12_HEAP_PROPERTIES heapprop = {}; {
			heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
			heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			heapprop.CreationNodeMask = 0;			heapprop.VisibleNodeMask = 0;
		}
		HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texbuff)));
		HRESULT(texbuff->WriteToSubresource(0, nullptr, img[0].pixels, img[0].rowPitch, img[0].slicePitch));
	}
	//�萔�o�b�t�@�[�쐬 : ���W�萔
	ID3D12Resource* constBuff = nullptr; 
	XMMATRIX worldMat,viewMat,projMat;
	XMMATRIX* mapMatrix; {
		auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);

		//XMMATRIX matrix = XMMatrixIdentity();
		worldMat = XMMatrixIdentity();
		XMFLOAT3 eye(0, 0, -10);XMFLOAT3 target(0, 0, 0);XMFLOAT3 up(0, 1, 0);
		viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		projMat = XMMatrixPerspectiveFovLH(
			XM_PIDIV2, static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 1.0f, 100.0f);

		HRESULT(_dev->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuff)));
		HRESULT(constBuff->Map(0, nullptr, (void**)&mapMatrix));
		*mapMatrix = worldMat * viewMat * projMat;
	}
	//�f�X�N���v�^�q�[�v�����--------------------------
	ID3D12DescriptorHeap* basicDescHeap = nullptr; {
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {}; {
			descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descHeapDesc.NodeMask = 0;
			descHeapDesc.NumDescriptors = 2;	//SRV��CBV
			descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		}
		HRESULT(_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap)));
		//�f�B�X�N���v�^�̐擪�n���h�����擾���Ă���
		auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
		//�V�F�[�_�[���\�[�X�r���[�����--------------------164
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {}; {
				srvDesc.Format = metadate.format;//DXGI_FORMAT_R8G8B8A8_UNORM;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; srvDesc.Texture2D.MipLevels = 1;
			}
			//�V�F�[�_�[���\�[�X�r���[�̍쐬
			_dev->CreateShaderResourceView(texbuff/*�r���[�Ɗ֘A�t����o�b�t�@�[*/, &srvDesc/*��قǐݒ肵���e�N�X�`���ݒ���*/, basicHeapHandle/*�擪�̏ꏊ�������n���h��*/);
		}
		//���̏ꏊ�Ɉړ�
		basicHeapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//�萔�o�b�t�@�[�r���[�̍쐬
		D3D12_CONSTANT_BUFFER_VIEW_DESC cdvDesc = {}; cdvDesc.BufferLocation = constBuff->GetGPUVirtualAddress(); cdvDesc.SizeInBytes = constBuff->GetDesc().Width;
		_dev->CreateConstantBufferView(&cdvDesc, basicHeapHandle);
		//HRESULT(_dev->GetDeviceRemovedReason());//�G���[�`�F�b�N�p->CreateConstantBufferView�𓮂����ƒ�~����->������CreateCommittedResource�̑�3����
	}

	//shader�쐬----------------------------------------
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr; {
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT(D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf()));
		HRESULT(D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf()));
#if 0//�G���[���o�����̊m�F�p
		string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		OutputDebugStringA(errstr.c_str());
#endif // 0
	}
	//�O���t�B�b�N�p�C�v���C���X�e�[�g�쐬--------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	ID3D12RootSignature* rootsignature; {
		D3D12_ROOT_PARAMETER rootparam = {};
		D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};
		descTblRange[0].NumDescriptors = 1;
		descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//��ʂ̓e�N�X�`��
		descTblRange[0].BaseShaderRegister = 0;
		descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		descTblRange[1].NumDescriptors = 1;
		descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	//��ʂ͒萔
		descTblRange[1].BaseShaderRegister = 0;
		descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootparam.DescriptorTable.pDescriptorRanges = descTblRange;
		rootparam.DescriptorTable.NumDescriptorRanges = 2;
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {}; {
			rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; rootSignatureDesc.pParameters = &rootparam;
			rootSignatureDesc.NumParameters = 1; rootSignatureDesc.NumStaticSamplers = 1;
		}
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {}; {
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK; samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			samplerDesc.MinLOD = 0.0f; samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			rootSignatureDesc.pStaticSamplers = &samplerDesc;
		}
		ComPtr<ID3DBlob> rootSigBlob = nullptr; {
			ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob));
		}
		HRESULT(_dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature)));
		gpipeline.pRootSignature = rootsignature;
		//rootSigBlob->Release();
		gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
		gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
		gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
		gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
		gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		gpipeline.RasterizerState.MultisampleEnable = false;
		gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		gpipeline.RasterizerState.DepthClipEnable = true;
		gpipeline.BlendState.AlphaToCoverageEnable = false;
		gpipeline.BlendState.IndependentBlendEnable = false;
		D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
		renderTargetBlendDesc.BlendEnable = false;
		renderTargetBlendDesc.LogicOpEnable = false;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			//{"EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		};/*D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		};*/
		gpipeline.InputLayout.pInputElementDescs = inputLayout;
		gpipeline.InputLayout.NumElements = _countof(inputLayout);
		gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gpipeline.NumRenderTargets = 1;
		gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpipeline.SampleDesc.Count = 1;
		gpipeline.SampleDesc.Quality = 0;

		gpipeline.DepthStencilState.DepthEnable = false;
		gpipeline.DepthStencilState.StencilEnable = false;
		gpipeline.RasterizerState.FrontCounterClockwise = false;
		gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		gpipeline.RasterizerState.AntialiasedLineEnable = false;
		gpipeline.RasterizerState.ForcedSampleCount = 0;
		gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr; {
		HRESULT(_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelinestate.GetAddressOf())));	//todo:E_INVALIDARG
	}

	//�r���[�|�[�g&�V�U�[�Z�`�쐬----------------------
	D3D12_VIEWPORT viewport = {}; {
		viewport.Width = WINDOW_WIDTH; viewport.Height = WINDOW_HEIGHT; viewport.TopLeftX = 0; viewport.TopLeftY = 0; viewport.MaxDepth = 200.0f; viewport.MinDepth = 0.0f;
	}
	D3D12_RECT scissorrect = {}; {
		scissorrect.top = 0; scissorrect.left = 0; scissorrect.right = scissorrect.left + WINDOW_WIDTH; scissorrect.bottom = scissorrect.top + WINDOW_HEIGHT;
	}


	//���[�v�쐬--------------------------------------------------------------------------
	MSG msg = {};
	float angle = 0;
	while (true) {
		angle += 0.1f;
		worldMat = XMMatrixScaling(sin(angle),sin(angle),sin(angle));
		*mapMatrix = worldMat * viewMat * projMat;


		//���[�v������--------------------------------------------------------------------
		//�����_�[�^�[�Q�b�g�̐ݒ�--------------------------86
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);
		float clearColor[] = { 0.0f,1.0f,1.0f,1.0f, };
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);//�E�B���h�E��h��Ԃ�->�`�施�ߌゾ�ƕ`���h��Ԃ�
		//�`�施��------------------------------------------
		_cmdList->SetPipelineState(_pipelinestate.Get());
		_cmdList->SetDescriptorHeaps(1, &basicDescHeap);
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);

		_cmdList->SetGraphicsRootSignature(rootsignature);//SetGraphicsRootDescriptorTable���O�ɏ���
		{
			auto heapHandle = basicDescHeap->GetGPUDescriptorHandleForHeapStart();//�Q�l���Ƃ̑���_->���\�[�X�ƒ萔�A�ǂ���̃o�b�t�@�[�r���[���ɓ���邩����
			//heapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			_cmdList->SetGraphicsRootDescriptorTable(/*1*/0, heapHandle);
		}

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);//���t���[���Z�b�g���Ȃ��Ƃ����Ȃ�
		_cmdList->IASetIndexBuffer(&ibView);//���t���[���Z�b�g���Ȃ��Ƃ����Ȃ�

		_cmdList->DrawIndexedInstanced(vertNum/*���_��*/, 1, 0, 0, 0);



		//IDXGISwapChain4����-------------------------------
		HRESULT(_cmdList->Close());
		ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		HRESULT(_cmdAllocator->Reset());
		HRESULT(_cmdList->Reset(_cmdAllocator.Get(), nullptr));

		HRESULT(_swapchain->Present(1, 0));

		//���[�v�I��----------------------------------------------------------------------
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
	}
	UnregisterClass(w.lpszClassName, w.hInstance);



	{//�v���O�����I��--------------------------------------------------------------------
		DebugOutputFormatString("Show window test.");
		getchar();
		return 0;
	}
}
