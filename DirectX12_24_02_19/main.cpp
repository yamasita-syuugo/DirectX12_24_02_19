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
#define HRESULT(function) result = function;if(result != S_OK){ cout << hex/*16進数*/ << result << endl; getchar(); return 1;}

//モデルのパスとテクスチャのパスから合成パスを得る
string GetTexturePathFromModelAndTexPath(const string& modelTath, const char* texPath);
//stringからwstringを得る
wstring GetWideStringFromString(const string& str);


#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hIns, HINSTANCE, LPSTR, int) {
#endif // _DEBUG

	//window生成-------------------------------------------------------------------------
	HWND hwnd; 
	WNDCLASSEX w = {}; {
		w.cbSize = sizeof(WNDCLASSEX); w.lpfnWndProc = (WNDPROC)WindowProcedure; w.lpszClassName = TEXT("a"); w.hInstance = GetModuleHandle(nullptr); 
		RegisterClassEx(&w);
		RECT wrc = { WINDOW_LEFT,WINDOW_TOP,WINDOW_WIDTH,WINDOW_HEIGHT };
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
		hwnd = CreateWindow(w.lpszClassName, TEXT("DX12テスト : 24/02/19"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, w.hInstance, nullptr);
		ShowWindow(hwnd, SW_SHOW);
	}

	//d3d12変数作成-----------------------------------------------------------------------
	// //ID3D12Device作成------------------------------------
	ComPtr<ID3D12Device> _dev = nullptr; {
		D3D_FEATURE_LEVEL featureLevel;
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0,D3D_FEATURE_LEVEL_9_3 ,D3D_FEATURE_LEVEL_9_2 ,D3D_FEATURE_LEVEL_9_1 ,
		};
		for (auto lv : levels)if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) { featureLevel = lv; break; }
	}
	//IDXGIFactory7作成-----------------------------------
	ComPtr<IDXGIFactory7> _dxgiFactory = nullptr;
	HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));
	//ID3D12CommandAllocator作成--------------------------
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	HRESULT(_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator)));
	//ID3D12CommandList作成-------------------------------
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	HRESULT(_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList)));
	//ID3D12CommandQueue作成------------------------------
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr; {
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {}; cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		HRESULT(_dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue)));
	}
	//IDXGISwapChain4作成---------------------------------
	ComPtr<IDXGISwapChain3> _swapchain = nullptr; {
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {}; swapchainDesc.Width = WINDOW_WIDTH; swapchainDesc.Height = WINDOW_HEIGHT; swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.Stereo = false; swapchainDesc.SampleDesc.Count = 1; swapchainDesc.SampleDesc.Quality = 0; swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swapchainDesc.BufferCount = 2; swapchainDesc.Scaling = DXGI_SCALING_STRETCH; swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		HRESULT(_dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)_swapchain.GetAddressOf()));
	}
	//ID3D12DescriptorHeap作成---------------------------
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr; {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.NodeMask = 0; heapDesc.NumDescriptors = 2; heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT(_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps)));
	}

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
	//IDXGISwapChain4と紐づけ 
	DXGI_SWAP_CHAIN_DESC swcDesc = {}; HRESULT(_swapchain->GetDesc(&swcDesc));
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

	//モデル取得--------------------------------------------
	//・ヘッダー情報
	//・頂点情報
	//・頂点対応情報
	//・マテリアル情報
	//..etcetc
	//	【ヘッダー情報】
	//	unsigned char magic[3];
	//	float version;
	//	char model_name[20];
	//	char comment[256];
	//【頂点情報】
	//	float pos[3];
	//	float normal_vec[3];
	//	float uv[2];　　 //テクスチャマッピング座標
	//	unsigned short born_num[2]; //ボーン番号
	//	unsigned char bone_weight; //ボーンの重み (ボーン1に与える影響度(0 ~ 100)　ボーン2に与える影響度 100 - bone_weight)
	//	unsigned char edge_flag; //輪郭線が有効の場合
	//【頂点集合情報】
	//	unsigned count;
	//	PMD_Vertex* vertexs; //頂点群
	//	count: ポリゴン数
	//	indexes : unsigned)頂点3ペア* ポリゴン数
	//【マテリアル情報】
	//	float diffuse[4]; //ディフューズ光
	//	float power; //反射強度
	//	float specular[3]; //スペキュラー光
	//	float emissive[3]; //エミッシブ光
	//	unsigned char toon_index; //トゥーン・インデックス番号
	//	unsigned char edge_flag; //エッジフラグ
	//	unsigned long face_vert_count; //面頂点数
	//	char texture_file_name[20]; //テクスチャファイル名
	PMDHeader pmdHeader; 
	vector<PMDVertex> vertices;
	vector<unsigned short> indices; 
	vector<Material> materials; {
		FILE* fp;
		fopen_s(&fp, "Model/初音ミク.pmd", "rb");
		char signature[3] = {};
		fread(signature, sizeof(signature), 1, fp);
		fread(&pmdHeader, sizeof(pmdHeader), 1, fp);

		unsigned int vertNum;//バーテックス
		fread(&vertNum, sizeof(vertNum), 1, fp);
		vertices.resize(vertNum);
		constexpr size_t pmdvertex_size = 38;
		for (int i = 0; i < vertNum; i++)fread(&vertices[i], pmdvertex_size, 1, fp);

		unsigned int indicesNum;//インデックス
		fread(&indicesNum, sizeof(indicesNum), 1, fp);
		indices.resize(indicesNum);
		fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

		unsigned int materialNum;//マテリアル
		fread(&materialNum, sizeof(materialNum), 1, fp);
		materials.resize(materialNum);
		PMDMaterial pmdMaterials;
		for (int i = 0; i < materialNum; i++) {
			fread(&pmdMaterials, sizeof(pmdMaterials), 1, fp);
			materials[i].indicesNum = pmdMaterials.indicesNum;
			materials[i].material.diffuse = pmdMaterials.diffuse;
			materials[i].material.alpha = pmdMaterials.alpha;
			materials[i].material.specular = pmdMaterials.specular;
			materials[i].material.specularity = pmdMaterials.specularity;
			materials[i].material.ambient = pmdMaterials.ambient;
		}
		fclose(fp);
	}


	TexMetadata metadate = {};
	ScratchImage scratchImg = {};
	HRESULT(LoadFromWICFile(L"img/ダウンロード.jfif", WIC_FLAGS_NONE, &metadate, scratchImg));	//C:\Users\syuugo_main\source\repos\DirectX12_24_02_19\DirectX12_24_02_19\img
	vector<Image> img;
	img.push_back(*scratchImg.GetImage(0, 0, 0));


	//頂点情報コピー------------------------------------
	//vertices----------------------
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; {
		//頂点バッファー作成--------------------------------
		ID3D12Resource* vertBuff = nullptr; {
			D3D12_HEAP_PROPERTIES heapprop = {}; heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			D3D12_RESOURCE_DESC resDescBuf = {}; {
				resDescBuf.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; resDescBuf.Width = vertices.size() * sizeof(vertices[0]); resDescBuf.Height = 1; resDescBuf.DepthOrArraySize = 1; resDescBuf.MipLevels = 1;
				resDescBuf.Format = DXGI_FORMAT_UNKNOWN; resDescBuf.SampleDesc.Count = 1; resDescBuf.Flags = D3D12_RESOURCE_FLAG_NONE; resDescBuf.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			}
			HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDescBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff)));
		}
		PMDVertex* vertMap = nullptr;
		HRESULT(vertBuff->Map(0, nullptr, (void**)&vertMap));
		copy(vertices.begin(), vertices.end(), vertMap);//配列205以上でエラー→バッファーのDescのwidthが小さかった(32だった)
		vertBuff->Unmap(0, nullptr);
		vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
		vbView.SizeInBytes = vertices.size() * sizeof(PMDVertex);
		vbView.StrideInBytes = sizeof(PMDVertex);
		//_cmdList->IASetVertexBuffers(0, 1, &vbView);//ループ内で使用
	}
	//indices-----------------------
	D3D12_INDEX_BUFFER_VIEW ibView = {}; {
		ID3D12Resource* idxBuff = nullptr; {
			D3D12_HEAP_PROPERTIES heapprop = {}; heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			D3D12_RESOURCE_DESC resDescBuf = {}; {
				resDescBuf.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; resDescBuf.Width = sizeof(indices[0]) * indices.size(); resDescBuf.Height = 1; resDescBuf.DepthOrArraySize = 1; resDescBuf.MipLevels = 1;
				resDescBuf.Format = DXGI_FORMAT_UNKNOWN; resDescBuf.SampleDesc.Count = 1; resDescBuf.Flags = D3D12_RESOURCE_FLAG_NONE; resDescBuf.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			}
			HRESULT(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDescBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff)));
		}
		unsigned short* mappedIdx = nullptr;
		HRESULT(idxBuff->Map(0, nullptr, (void**)&mappedIdx));
		copy(begin(indices), end(indices), mappedIdx);
		idxBuff->Unmap(0, nullptr);
		ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
		ibView.SizeInBytes = indices.size() * sizeof(indices[0]);
		ibView.Format = DXGI_FORMAT_R16_UINT;
		//_cmdList->IASetIndexBuffer(&ibView);//ループ内で使用
	}
	//depth--------------------------dsv:depthShaderView
	ID3D12DescriptorHeap* dsvHeap = {}; {
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1; dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			HRESULT(_dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));
		}
		ID3D12Resource* depthBuffer = nullptr; {
			D3D12_RESOURCE_DESC depthResDesc = {}; {
				depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; depthResDesc.Width = WINDOW_WIDTH; depthResDesc.Height = WINDOW_HEIGHT; depthResDesc.DepthOrArraySize = 1;
				depthResDesc.Format = DXGI_FORMAT_D32_FLOAT; depthResDesc.SampleDesc.Count = 1; depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				depthResDesc.MipLevels = 1;
				depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				depthResDesc.Alignment = 0;
			}
			D3D12_HEAP_PROPERTIES depthHeapProp = {}; {
				depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT; depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			}
			D3D12_CLEAR_VALUE depthClearValue = {}; {
				depthClearValue.DepthStencil.Depth = 1.0f; depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			}
			HRESULT(_dev->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer)));
		}
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {}; {
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		}
		_dev->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}
	ID3D12DescriptorHeap* materialDescHeap = nullptr; {
		{
			D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
			matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			matDescHeapDesc.NodeMask = 0;
			matDescHeapDesc.NumDescriptors = materials.size();
			matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			HRESULT(_dev->CreateDescriptorHeap(&matDescHeapDesc, IID_PPV_ARGS(&materialDescHeap)));
		}
		auto materialBuffSize = sizeof(MaterialForHlsl);
		materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
		ID3D12Resource* materialBuff = nullptr; {
			auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materials.size());
			HRESULT(_dev->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialBuff)));
		}
		{
			char* mapMaterial = nullptr;
			MaterialForHlslDummy* tmpMat = nullptr;
			HRESULT(materialBuff->Map(0, nullptr, (void**)&mapMaterial));
			tmpMat = (MaterialForHlslDummy*)mapMaterial;
			for (auto& m : materials) {
				*((MaterialForHlsl*)mapMaterial) = m.material; mapMaterial += materialBuffSize; }
			vector<MaterialForHlslDummy> tmp(materialBuffSize);
			for(int i  = 0;i< materialBuffSize;i++)tmp[i] = tmpMat[i];
			materialBuff->Unmap(0, nullptr);
		}
		D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {}; {
			matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
			matCBVDesc.SizeInBytes = materialBuffSize;
		}
		auto matDescHeapH = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (int i = 0; i < materials.size(); i++) {
			_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
			matDescHeapH.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			matCBVDesc.BufferLocation += materialBuffSize;
		}
	}

	MatricexsData* mapMatrix; 
	XMMATRIX worldMat, viewMat, projMat; //ループ内で使用するためスコープ外
	//デスクリプタヒープを作る--------------------------
	ID3D12DescriptorHeap* basicDescHeap = nullptr; {
		//テクスチャバッファー作成--------------------------
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
		//定数バッファー作成 : 座標定数
		ID3D12Resource* constBuff = nullptr; {
			auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricexsData) + 0xff) & ~0xff);

			//XMMATRIX matrix = XMMatrixIdentity();
			worldMat = XMMatrixIdentity();
			XMFLOAT3 eye(0, 10, -15), target(0.0f, 10.0f, 0.0f), up(0.0f, 1.0f, 0.0f);
			viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			projMat = XMMatrixPerspectiveFovLH(
				XM_PIDIV2/*視野角*/, static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)/*縦横の倍率*/, 1.0f, 100.0f);

			HRESULT(_dev->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuff)));
			HRESULT(constBuff->Map(0, nullptr, (void**)&mapMatrix));
			mapMatrix->world = worldMat;
			mapMatrix->viewproj = viewMat * projMat;
		}
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {}; {
			descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descHeapDesc.NodeMask = 0;
			descHeapDesc.NumDescriptors = 2;	//SRVとCBV
			descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		}
		HRESULT(_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap)));
		//ディスクリプタの先頭ハンドルを取得しておく
		auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
		//シェーダーリソースビューを作る--------------------164
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {}; {
				srvDesc.Format = metadate.format;//DXGI_FORMAT_R8G8B8A8_UNORM;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; srvDesc.Texture2D.MipLevels = 1;
			}
			//シェーダーリソースビューの作成
			_dev->CreateShaderResourceView(texbuff/*ビューと関連付けるバッファー*/, &srvDesc/*先ほど設定したテクスチャ設定情報*/, basicHeapHandle/*先頭の場所を示すハンドル*/);
		}
		//次の場所に移動
		basicHeapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//定数バッファービューの作成
		D3D12_CONSTANT_BUFFER_VIEW_DESC cdvDesc = {}; cdvDesc.BufferLocation = constBuff->GetGPUVirtualAddress(); cdvDesc.SizeInBytes = constBuff->GetDesc().Width;
		_dev->CreateConstantBufferView(&cdvDesc, basicHeapHandle);
		//HRESULT(_dev->GetDeviceRemovedReason());//エラーチェック用->CreateConstantBufferViewを動かすと停止する->原因はCreateCommittedResourceの第3引数
	}

	//グラフィックパイプラインステート作成--------------:シェーダー情報や頂点情報
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
	ID3D12RootSignature* rootsignature; {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {}; {
			{
				D3D12_DESCRIPTOR_RANGE descTblRange[4] = {}; {
					descTblRange[0].NumDescriptors = 1;
					descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	//種別は定数
					descTblRange[0].BaseShaderRegister = 0;
					descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					descTblRange[1].NumDescriptors = 1;
					descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//種別はテクスチャ
					descTblRange[1].BaseShaderRegister = 0;
					descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					descTblRange[2].NumDescriptors = 1;
					descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	//種別は定数
					descTblRange[2].BaseShaderRegister = 1;
					descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					descTblRange[3].NumDescriptors = 1;
					descTblRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//種別はテクスチャ
					descTblRange[3].BaseShaderRegister = 1;
					descTblRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}
				D3D12_ROOT_PARAMETER rootparam[2] = {}; {
					rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
					rootparam[0].DescriptorTable.NumDescriptorRanges = 2;
					rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[2];
					rootparam[1].DescriptorTable.NumDescriptorRanges = 2;
				}
				ComPtr<ID3DBlob> rootSigBlob = nullptr; {
					D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {}; {
						rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; rootSignatureDesc.pParameters = rootparam;
						rootSignatureDesc.NumParameters = 2; rootSignatureDesc.NumStaticSamplers = 1;
						D3D12_STATIC_SAMPLER_DESC samplerDesc = {}; {
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK; samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
							samplerDesc.MinLOD = 0.0f; samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
							rootSignatureDesc.pStaticSamplers = &samplerDesc;
						}
						ComPtr<ID3DBlob> errorBlob = nullptr;
						HRESULT(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob));
					}
				}
				HRESULT(_dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature)));
				gpipeline.pRootSignature = rootsignature;
			}
			//rootSigBlob->Release();
			//shader作成----------------------------------------
			ComPtr<ID3DBlob> vsBlob = nullptr;
			ComPtr<ID3DBlob> psBlob = nullptr; {
				ComPtr<ID3DBlob> errorBlob = nullptr;
				HRESULT(D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf()));
				HRESULT(D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf()));
#if 0//エラーが出た時の確認用
				string errstr;
				errstr.resize(errorBlob->GetBufferSize());
				copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
				OutputDebugStringA(errstr.c_str());
#endif // 0
				gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
				gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
				gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
				gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
			}
			gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
			gpipeline.RasterizerState.MultisampleEnable = false;
			gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			gpipeline.RasterizerState.DepthClipEnable = true;
			gpipeline.BlendState.AlphaToCoverageEnable = false;
			gpipeline.BlendState.IndependentBlendEnable = false;

			D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {}; {
				renderTargetBlendDesc.BlendEnable = false;
				renderTargetBlendDesc.LogicOpEnable = false;
				renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
			}
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

			gpipeline.DepthStencilState.DepthEnable = true;
			gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			gpipeline.RasterizerState.FrontCounterClockwise = false;
			gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			gpipeline.RasterizerState.AntialiasedLineEnable = false;
			gpipeline.RasterizerState.ForcedSampleCount = 0;
			gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			HRESULT(_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelineState.GetAddressOf())));
		}
	}

	//ビューポート&シザー短形作成----------------------
	D3D12_VIEWPORT viewport = {}; {
		viewport.Width = WINDOW_WIDTH; viewport.Height = WINDOW_HEIGHT; viewport.TopLeftX = 0; viewport.TopLeftY = 0; viewport.MaxDepth = 1.0f; viewport.MinDepth = 0.0f;
	}
	D3D12_RECT scissorrect = {}; {
		scissorrect.top = 0; scissorrect.left = 0; scissorrect.right = scissorrect.left + WINDOW_WIDTH; scissorrect.bottom = scissorrect.top + WINDOW_HEIGHT;
	}

	DirectInput keyboard(hIns);
	//ループ作成--------------------------------------------------------------------------
	MSG msg = {};
	float angle = 0;
	int control = 0;
	while (true) {
		keyboard.Execute();

		angle += 0.1f;
		XMFLOAT3 eye(cos(angle), sin(angle) + 10, -10); XMFLOAT3 target(0.0f, 10.0f, 0.0f); XMFLOAT3 up(0, 1, 0);
		worldMat = XMMatrixRotationY(angle * 0.1f);
		viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		mapMatrix->world = worldMat;
		mapMatrix->viewproj = viewMat * projMat;

		control = 1;

		//レンダーターゲットの設定--------------------------86
		{
			{//描画範囲の指定？
				auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart(); {
					auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
					rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				}
				auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
				_cmdList->OMSetRenderTargets(1, &rtvH, false,/*nullptr*/ &dsvH);
				//depthハンドルを入れるとオブジェクトが消える->ClearDepthStencilView第三引数が1000で解決:0.0f～1.0fに正規化出来てない->ビューポートのMaxDepthが200だった
				float clearColor[] = { 0.0f,1.0f,1.0f,1.0f, };
				_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);//ウィンドウを塗りつぶす->描画命令後だと描画を塗りつぶす
				_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			}

			//描画命令------------------------------------------
			_cmdList->RSSetViewports(1, &viewport);
			_cmdList->RSSetScissorRects(1, &scissorrect);

			_cmdList->SetPipelineState(_pipelineState.Get());

			_cmdList->SetGraphicsRootSignature(rootsignature);//SetGraphicsRootDescriptorTableより前に書く
			{
				_cmdList->SetDescriptorHeaps(1, &basicDescHeap);
				auto heapHandle = basicDescHeap/*materialDescHeap*/->GetGPUDescriptorHandleForHeapStart();//参考書との相違点->リソースと定数、どちらのバッファービューを先に入れるか次第
				heapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				_cmdList->SetGraphicsRootDescriptorTable(/*1*/0, heapHandle);

				_cmdList->SetDescriptorHeaps(1, &materialDescHeap);

				auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
				unsigned int idxOffset = 0;

				auto cbvsrvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;
				for (auto& m : materials) {
					_cmdList->SetGraphicsRootDescriptorTable(1, materialH);
					materialH.ptr += 0;//0～19で色が変わる(水色、青、紺、薄い黄、白)それ以外で真っ黒→31白
					_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
					idxOffset += m.indicesNum;
				}
			}

			_cmdList->IASetPrimitiveTopology(/*D3D_PRIMITIVE_TOPOLOGY_POINTLIST*/D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//頂点データの解釈

			_cmdList->IASetVertexBuffers(0, 1, &vbView);//毎フレームセットしないといけない
			_cmdList->IASetIndexBuffer(&ibView);//毎フレームセットしないといけない
			_cmdList->DrawIndexedInstanced(indices.size()/*index数*/, 1, 0, 0, 0);

			HRESULT(_cmdList->Close());
		}

		//IDXGISwapChain4動作-------------------------------
		{
			ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
			_cmdQueue->ExecuteCommandLists(1, cmdlists);

			HRESULT(_cmdAllocator->Reset());
			HRESULT(_cmdList->Reset(_cmdAllocator.Get(), nullptr));

			HRESULT(_swapchain->Present(1, 0));
		}

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
		getchar();
		return 0;
	}
}

string GetTexturePathFromModelAndTexPath(const string& modelPath, const char* texPath) {
	auto folderPath = modelPath.substr(0, modelPath.rfind('/'));
	return folderPath + texPath;
}
wstring GetWideStringFromString(const string& str) {
	//呼び出し1回目(文字列数を得る)
	auto num1 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);
	wstring wstr;
	wstr.resize(num1);

	//呼び出し2回目(確保済みのwstrに変換文字列をコピー)
	auto num2 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num1);
	assert(num1 == num2);
	return wstr;
}