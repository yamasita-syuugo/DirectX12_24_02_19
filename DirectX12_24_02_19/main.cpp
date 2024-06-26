#include<Windows.h>
#ifdef _DEBUG
#include<iostream>
#endif // _DEBUG

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
#define HRESULT(function) result = function;if(result != S_OK)return 1

//頂点データ構造体
struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};
//テクスチャデータ構造体
struct TexRGBA
{
	unsigned char R, G, B, A;
};



#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif // _DEBUG

	//window生成-------------------------------------------------------------------------
	WNDCLASSEX w = {};
	{
		w.cbSize = sizeof(WNDCLASSEX);
		w.lpfnWndProc = (WNDPROC)WindowProcedure;
		w.lpszClassName = TEXT("DX12Sample");
		w.hInstance = GetModuleHandle(nullptr);
	}
	RegisterClassEx(&w);
	RECT wrc = { WINDOW_LEFT,WINDOW_TOP,WINDOW_WIDTH,WINDOW_HEIGHT };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	HWND hwnd = CreateWindow(w.lpszClassName,
		TEXT("DX12テスト"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);
	ShowWindow(hwnd, SW_SHOW);

	//d3d12変数作成-----------------------------------------------------------------------
	ComPtr<ID3D12Device> _dev = nullptr;
	ComPtr<IDXGIFactory7> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapchain = nullptr;

	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	vector<ComPtr<ID3D12GraphicsCommandList>> _cmdList = { nullptr,nullptr };

	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;

	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;

	//d3d12変数の中身作成-----------------------------------------------------------------
#if 0//グラボが複数ある場合の選択例
	//アダプターの列挙------------------------------------
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
			tmpAdapter = adpt;//D3D12CreateDeviceの第一引数で使用
			break;
		}
	}
#endif
	D3D_FEATURE_LEVEL featureLevel;
	{//ID3D12Device作成------------------------------------
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3 ,
			D3D_FEATURE_LEVEL_9_2 ,
			D3D_FEATURE_LEVEL_9_1 ,
		};
		for (auto lv : levels) {
			if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) {
				featureLevel = lv;
				break;
			}
		}
	}
	//IDXGIFactory7作成-----------------------------------
	HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));
	//ID3D12CommandAllocator作成--------------------------
	HRESULT(_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator)));
	//ID3D12CommandList作成-------------------------------
	HRESULT(_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList[0])));
	//ID3D12CommandQueue作成------------------------------
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT(_dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue)));
	//IDXGISwapChain4作成---------------------------------
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = WINDOW_WIDTH;
	swapchainDesc.Height = WINDOW_HEIGHT;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	HRESULT(_dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)_swapchain.GetAddressOf()));
	//ID3D12DescriptorHeap作成---------------------------
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT(_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps)));
	//IDXGISwapChain4と紐づけ
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	HRESULT(_swapchain->GetDesc(&swcDesc));
	vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {
		HRESULT(_swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx])));
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], &rtvDesc, handle);
	}
	//座標計算----------------------------------------------
	Vertex vertices[] = {
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}},
		{{-0.4f,0.7f,0.0f},	{0.0f,0.0f}},
		{{0.4f,-0.7f,0.0f},	{1.0f,1.0f}},
		{{0.4f,0.7f,0.0f},	{1.0f,0.0f}},
		{{0.8f,0.7f,0.0f},	{0.0f,0.0f}},
	};
	//インデックスの実装------------------------------------
	unsigned short indices[] = {
		0,1,2,
		2,1,3,
		3,2,4,
	};
	//テクスチャデータ作成----------------------------------
	//vector<TexRGBA> texturedata(256 * 256);
	//for (auto& rgba : texturedata) {
	//	rgba.R = rand() % 256;
	//	rgba.G = rand() % 256;
	//	rgba.B = rand() % 256;
	//	rgba.A = 255;
	//}
	TexMetadata metadate = {};
	ScratchImage scratchImg = {};
	HRESULT(LoadFromWICFile(L"img/textest.png", WIC_FLAGS_NONE, &metadate, scratchImg));
	auto img = scratchImg.GetImage(0, 0, 0);

	//頂点バッファー作成--------------------------------
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ID3D12Resource* vertBuff = nullptr;
	HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff)));
	ID3D12Resource* idxBuff = nullptr;
	HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff)));
	//頂点情報コピー------------------------------------
	//vertices----------------------
	Vertex* vertMap = nullptr;
	HRESULT(vertBuff->Map(0, nullptr, (void**)&vertMap));
	copy(begin(vertices), end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);
	D3D12_VERTEX_BUFFER_VIEW vdView = {};
	vdView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vdView.SizeInBytes = sizeof(vertices);
	vdView.StrideInBytes = sizeof(vertices[0]);
	_cmdList[0]->IASetVertexBuffers(0, 1, &vdView);
	//indices-----------------------
	unsigned short* mappedIdx = nullptr;
	HRESULT(idxBuff->Map(0, nullptr, (void**)&mappedIdx));
	copy(begin(indices), end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.SizeInBytes = sizeof(indices);
	ibView.Format = DXGI_FORMAT_R16_UINT;
	_cmdList[0]->IASetIndexBuffer(&ibView);
	//定数バッファー作成
	XMMATRIX matrix = XMMatrixIdentity();
	ID3D12Resource* constBuff = nullptr;
	_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuff));
	XMMATRIX* mapMatrix;
	HRESULT(constBuff->Map(0, nullptr, (void**)&mapMatrix));
	*mapMatrix = matrix;

	//テクスチャバッファー作成--------------------------
	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapprop.CreationNodeMask = 0;
	heapprop.VisibleNodeMask = 0;
	D3D12_RESOURCE_DESC resDesc = {};
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
	ID3D12Resource* texbuff = nullptr;
	HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texbuff)));
	HRESULT(texbuff->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch));
	//デスクリプタヒープを作る--------------------------
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;	//SRVとCBV
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ID3D12DescriptorHeap* basicDescHeap = nullptr;
	HRESULT(_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap)));
	//シェーダーリソースビューを作る--------------------164
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resDesc.Format;//DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView(texbuff, &srvDesc, basicDescHeap->GetCPUDescriptorHandleForHeapStart());




	//shader作成----------------------------------------
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT(D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf()));
	HRESULT(D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf()));
#if 0//エラーが出た時の確認用
	string errstr;
	errstr.resize(errorBlob->GetBufferSize());
	copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
	OutputDebugStringA(errstr.c_str());
#endif // 0
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};
	//グラフィックパイプラインステート作成--------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;
	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;
	ID3DBlob* rootSigBlob = nullptr;
	HRESULT(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob));
	ID3D12RootSignature* rootsignature;
	HRESULT(_dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature)));
	gpipeline.pRootSignature = rootsignature;
	rootSigBlob->Release();
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


	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	HRESULT(_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelinestate.GetAddressOf())));	//todo:E_INVALIDARG

	//ビューポート&シザー短形作成----------------------
	D3D12_VIEWPORT viewport = {};
	viewport.Width = WINDOW_WIDTH;
	viewport.Height = WINDOW_HEIGHT;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	D3D12_RECT scissorrect = {};
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + WINDOW_WIDTH;
	scissorrect.bottom = scissorrect.top + WINDOW_HEIGHT;



	//ループ作成--------------------------------------------------------------------------
	MSG msg = {};
	while (true) {

		//ループ内処理--------------------------------------------------------------------
		//レンダーターゲットの設定--------------------------86
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList[0]->OMSetRenderTargets(1, &rtvH, true, nullptr);
		float clearColor[] = { 0.5f,0.5f,0.5f,1.0f, };
		_cmdList[0]->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);//ウィンドウを塗りつぶす->描画命令後だと描画を塗りつぶす
		//描画命令------------------------------------------
		_cmdList[0]->SetPipelineState(_pipelinestate.Get());
		_cmdList[0]->SetGraphicsRootSignature(rootsignature);
		_cmdList[0]->SetDescriptorHeaps(1, &basicDescHeap);
		_cmdList[0]->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());
		_cmdList[0]->RSSetViewports(1, &viewport);
		_cmdList[0]->RSSetScissorRects(1, &scissorrect);
		_cmdList[0]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList[0]->IASetVertexBuffers(0, 1, &vdView);
		_cmdList[0]->IASetIndexBuffer(&ibView);
		_cmdList[0]->DrawIndexedInstanced(sizeof(indices)/*頂点数*/, 1, 0, 0, 0);

		//IDXGISwapChain4動作-------------------------------
		HRESULT(_cmdList[0]->Close());
		ID3D12CommandList* cmdlists[] = { _cmdList[0].Get() };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);
		HRESULT(_cmdAllocator->Reset());
		HRESULT(_cmdList[0]->Reset(_cmdAllocator.Get(), nullptr));
		HRESULT(_swapchain->Present(1, 0));



		//ループ終了----------------------------------------------------------------------
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
	}
	UnregisterClass(w.lpszClassName, w.hInstance);



	{//プログラム終了--------------------------------------------------------------------
		DebugOutputFormatString("Show window test.");
		//getchar();
		return 0;
	}
}
