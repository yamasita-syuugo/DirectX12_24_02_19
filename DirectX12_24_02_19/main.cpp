#include"UseHeader.h"
#include"value.h"

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

#define HRESULT_(function){ HRESULT result = function;if(result != S_OK){ cout << hex/*16進数*/ << result << endl; getchar(); return 1;}}

//モデルのパスとテクスチャのパスから合成パスを得る
string GetTexturePathFromModelAndTexPath(const string& modelTath, const char* texPath);
//stringからwstringを得る
wstring GetWideStringFromString(const string& str);
ID3D12Resource* LoadTextureFromFile(string& texPath);
ID3D12Resource* CreateWhiteTexture(); 
ID3D12Resource* CreateBlackTexture();
string GetExtension(const string& path);
pair<string,string> SplitFileName(const string& path, const char splitter = '*');

ComPtr<ID3D12Device> _dev = nullptr;
ComPtr<IDXGIFactory7> _dxgiFactory = nullptr;
ComPtr<IDXGISwapChain3> _swapchain = nullptr;
ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;

int WINAPI WinMain(HINSTANCE hIns, HINSTANCE, LPSTR, int) {

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
	_dev; {
		D3D_FEATURE_LEVEL featureLevel;
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0,
			/*D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0,D3D_FEATURE_LEVEL_9_3 ,D3D_FEATURE_LEVEL_9_2 ,D3D_FEATURE_LEVEL_9_1 ,*/
		};
		for (auto lv : levels)if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) { featureLevel = lv; break; }
	}
	//IDXGIFactory7作成-----------------------------------
	_dxgiFactory; HRESULT_(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));
	//ID3D12CommandAllocator作成--------------------------
	_cmdAllocator; HRESULT_(_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator)));
	//ID3D12CommandList作成-------------------------------
	_cmdList; HRESULT_(_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList)));
	//ID3D12CommandQueue作成------------------------------
	_cmdQueue; {
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {}; cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		HRESULT_(_dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue)));
	}
	//IDXGISwapChain4作成---------------------------------
	_swapchain; {
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {}; swapchainDesc.Width = WINDOW_WIDTH; swapchainDesc.Height = WINDOW_HEIGHT; swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.Stereo = false; swapchainDesc.SampleDesc.Count = 1; swapchainDesc.SampleDesc.Quality = 0; swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swapchainDesc.BufferCount = 2; swapchainDesc.Scaling = DXGI_SCALING_STRETCH; swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		HRESULT_(_dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)_swapchain.GetAddressOf()));
	}
	//ID3D12DescriptorHeap作成---------------------------
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr; {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.NodeMask = 0; heapDesc.NumDescriptors = 2; heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT_(_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps)));
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
	DXGI_SWAP_CHAIN_DESC swcDesc = {}; HRESULT_(_swapchain->GetDesc(&swcDesc));
	vector<ComPtr<ID3D12Resource>> _backBuffers(swcDesc.BufferCount);
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart(); {
			for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {
				HRESULT_(_swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx])));
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
	string strModelPath = "Model/巡音ルカ.pmd";//初音ミク,初音ミクmetal,MEIKO
	PMDHeader pmdHeader;
	vector<PMDVertex> vertices;
	vector<unsigned short> indices;
	vector<Material> materials;
	vector<PMDMaterial> pmdMaterials; {

		auto a = []() { int a = 0; return a; };

		FILE* fp;
		fopen_s(&fp, strModelPath.c_str(), "rb");
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
		pmdMaterials.resize(materialNum);
		for (int i = 0; i < materialNum; i++) {
			fread(&pmdMaterials[i], sizeof(PMDMaterial), 1, fp);
			materials[i].indicesNum = pmdMaterials[i].indicesNum;
			materials[i].material.diffuse = pmdMaterials[i].diffuse;
			materials[i].material.alpha = pmdMaterials[i].alpha;
			materials[i].material.specular = pmdMaterials[i].specular;
			materials[i].material.specularity = pmdMaterials[i].specularity;
			materials[i].material.ambient = pmdMaterials[i].ambient;
		}

		fclose(fp);
	}


	TexMetadata metadate = {};
	ScratchImage scratchImg = {};
	vector<Image> img; {
		HRESULT_(LoadFromWICFile(L"img/ダウンロード.jfif", WIC_FLAGS_NONE, &metadate, scratchImg));	//C:\Users\syuugo_main\source\repos\DirectX12_24_02_19\DirectX12_24_02_19\img
		img.push_back(*scratchImg.GetImage(0, 0, 0));
	}


	//頂点情報コピー------------------------------------
	//vertices----------------------
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {}; {
		//頂点バッファー作成--------------------------------
		{
			ID3D12Resource* vertBuff = nullptr; {
				D3D12_HEAP_PROPERTIES heapprop = {}; heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				D3D12_RESOURCE_DESC resDescBuf = {}; {
					resDescBuf.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; resDescBuf.Width = vertices.size() * sizeof(vertices[0]); resDescBuf.Height = 1; resDescBuf.DepthOrArraySize = 1; resDescBuf.MipLevels = 1;
					resDescBuf.Format = DXGI_FORMAT_UNKNOWN; resDescBuf.SampleDesc.Count = 1; resDescBuf.Flags = D3D12_RESOURCE_FLAG_NONE; resDescBuf.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				}
				HRESULT_(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDescBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff)));
			}
			PMDVertex* vertMap = nullptr;
			HRESULT_(vertBuff->Map(0, nullptr, (void**)&vertMap));
			copy(vertices.begin(), vertices.end(), vertMap);//配列205以上でエラー→バッファーのDescのwidthが小さかった(32だった)
			vertBuff->Unmap(0, nullptr);
			vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
			vbView.SizeInBytes = vertices.size() * sizeof(PMDVertex);
			vbView.StrideInBytes = sizeof(PMDVertex);
			//_cmdList->IASetVertexBuffers(0, 1, &vbView);//ループ内で使用
		}
		//indices----------------------- 
		{
			ID3D12Resource* idxBuff = nullptr; {
				D3D12_HEAP_PROPERTIES heapprop = {}; heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				D3D12_RESOURCE_DESC resDescBuf = {}; {
					resDescBuf.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; resDescBuf.Width = sizeof(indices[0]) * indices.size(); resDescBuf.Height = 1; resDescBuf.DepthOrArraySize = 1; resDescBuf.MipLevels = 1;
					resDescBuf.Format = DXGI_FORMAT_UNKNOWN; resDescBuf.SampleDesc.Count = 1; resDescBuf.Flags = D3D12_RESOURCE_FLAG_NONE; resDescBuf.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				}
				HRESULT_(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDescBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff)));
			}
			unsigned short* mappedIdx = nullptr;
			HRESULT_(idxBuff->Map(0, nullptr, (void**)&mappedIdx));
			copy(begin(indices), end(indices), mappedIdx);
			idxBuff->Unmap(0, nullptr);
			ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
			ibView.SizeInBytes = indices.size() * sizeof(indices[0]);
			ibView.Format = DXGI_FORMAT_R16_UINT;
			//_cmdList->IASetIndexBuffer(&ibView);//ループ内で使用
		}
	}

	//depth--------------------------dsv:depthShaderView
	ID3D12DescriptorHeap* dsvHeap = {}; {
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {}; dsvHeapDesc.NumDescriptors = 1; dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			HRESULT_(_dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));
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
			HRESULT_(_dev->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer)));
		}
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {}; {
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		}
		_dev->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	SceneMatricexs* mapMatrix;
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
			HRESULT_(_dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texbuff)));
			HRESULT_(texbuff->WriteToSubresource(0, nullptr, img[0].pixels, img[0].rowPitch, img[0].slicePitch));
		}
		//定数バッファー作成 : 座標定数
		ID3D12Resource* constBuff = nullptr; {
			auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneMatricexs) + 0xff) & ~0xff);

			//XMMATRIX matrix = XMMatrixIdentity();
			worldMat = XMMatrixIdentity();
			XMFLOAT3 eye(0, 10, -15), target(0.0f, 10.0f, 0.0f), up(0.0f, 1.0f, 0.0f);
			viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			projMat = XMMatrixPerspectiveFovLH(
				XM_PIDIV2/*視野角*/, static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)/*縦横の倍率*/, 1.0f, 100.0f);

			HRESULT_(_dev->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuff)));
			HRESULT_(constBuff->Map(0, nullptr, (void**)&mapMatrix));
			mapMatrix->world = worldMat;
			mapMatrix->view = viewMat;
			mapMatrix->proj = projMat;
			mapMatrix->eye = eye;
		}
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {}; {
			descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descHeapDesc.NodeMask = 0;
			descHeapDesc.NumDescriptors = 2;	//SRVとCBV
			descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		}
		HRESULT_(_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap)));
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
		//HRESULT_(_dev->GetDeviceRemovedReason());//エラーチェック用->CreateConstantBufferViewを動かすと停止する->原因はCreateCommittedResourceの第3引数
	}
	ID3D12DescriptorHeap* materialDescHeap = nullptr; {//register(b1)が割り当てられる理由→CommandList->SetGraphicsRootDescriptorTableの第一引数
		{
			D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
			matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			matDescHeapDesc.NodeMask = 0;
			matDescHeapDesc.NumDescriptors = materials.size() * 4;
			matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			HRESULT_(_dev->CreateDescriptorHeap(&matDescHeapDesc, IID_PPV_ARGS(&materialDescHeap)));
		}
		auto materialBuffSize = sizeof(MaterialForHlsl);
		materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
		ID3D12Resource* materialBuff = nullptr; {
			auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materials.size());
			HRESULT_(_dev->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialBuff)));
		}
		{
			char* mapMaterial = nullptr;
			HRESULT_(materialBuff->Map(0, nullptr, (void**)&mapMaterial));
			for (auto& m : materials) {
				*((MaterialForHlsl*)mapMaterial) = m.material;
				mapMaterial += materialBuffSize;
			}
			materialBuff->Unmap(0, nullptr);
		}
		D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {}; {
			matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
			matCBVDesc.SizeInBytes = materialBuffSize;
		}
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {}; {
			srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
		}
		auto matDescHeapH = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
		auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		ID3D12Resource* whiteTex = CreateWhiteTexture();
		ID3D12Resource* blackTex = CreateBlackTexture();
		vector<ID3D12Resource*> textureResources(pmdMaterials.size()); 
		vector<ID3D12Resource*> sphResources(materials.size());
		vector<ID3D12Resource*> spaResources(materials.size());{
			for (int i = 0; i < pmdMaterials.size(); i++) {
				if (strlen(pmdMaterials[i].texFilePath) == 0) { textureResources[i] = nullptr; sphResources[i] = nullptr; }
				string texFileName = {};
				string sphFileName = {};
				string spaFileName = {};
				if (GetExtension(pmdMaterials[i].texFilePath) == "sph") {
					sphFileName = pmdMaterials[i].texFilePath;
				}
				else if (GetExtension(pmdMaterials[i].texFilePath) == "spa") {
					spaFileName = pmdMaterials[i].texFilePath;
				}
				else {
					texFileName = pmdMaterials[i].texFilePath;
				}
				if (count(texFileName.begin(), texFileName.end(), '*') > 0 || 
					count(sphFileName.begin(), sphFileName.end(), '*') > 0 || 
					count(spaFileName.begin(), spaFileName.end(), '*') > 0) {
					auto namepair = SplitFileName(pmdMaterials[i].texFilePath);
					if (GetExtension(namepair.first) == "sph") {
						texFileName = namepair.second;
						sphFileName = namepair.first;
					}
					else if (GetExtension(namepair.first) == "spa") {
						texFileName = namepair.second;
						spaFileName = namepair.first;
					}
					else if (GetExtension(namepair.second) == "sph") {
						texFileName = namepair.first;
						sphFileName = namepair.second;
					}
					else if (GetExtension(namepair.second) == "spa") {
						texFileName = namepair.first;
						spaFileName = namepair.second;
					}
				}

				auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFileName.c_str());
				auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
				auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
				ID3D12Resource* tmp = nullptr;
				if (strModelPath.size() < texFilePath.size())tmp = LoadTextureFromFile(texFilePath);//todo:ラムダ式導入で拡張子がない場合エラーが起こる
				textureResources[i] = tmp;
				if (strModelPath.size() < sphFilePath.size())tmp = LoadTextureFromFile(sphFilePath);
				sphResources[i] = tmp;
				if (strModelPath.size() < spaFilePath.size())tmp = LoadTextureFromFile(spaFilePath);
				spaResources[i] = tmp;
			}
			int a = 0;
		}
		for (int i = 0; i < materials.size(); i++) {
			_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
			matDescHeapH.ptr += inc;
			matCBVDesc.BufferLocation += materialBuffSize;

			if (textureResources[i] == nullptr) {
				srvDesc.Format = whiteTex->GetDesc().Format;
				_dev->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
			}
			else {
				srvDesc.Format = textureResources[i]->GetDesc().Format;
				_dev->CreateShaderResourceView(textureResources[i], &srvDesc, matDescHeapH);
			}
			matDescHeapH.ptr += inc;
			if (sphResources[i] == nullptr) {
				srvDesc.Format = whiteTex->GetDesc().Format;
				_dev->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
			}
			else {
				srvDesc.Format = sphResources[i]->GetDesc().Format;
				_dev->CreateShaderResourceView(sphResources[i], &srvDesc, matDescHeapH);
			}
			matDescHeapH.ptr += inc;
			if (spaResources[i] == nullptr) {
				srvDesc.Format = blackTex->GetDesc().Format;
				_dev->CreateShaderResourceView(blackTex, &srvDesc, matDescHeapH);
			}
			else {
				srvDesc.Format = spaResources[i]->GetDesc().Format;
				_dev->CreateShaderResourceView(spaResources[i], &srvDesc, matDescHeapH);
			}
			matDescHeapH.ptr += inc;
		}
	}

	//グラフィックパイプラインステート作成--------------:シェーダー情報や頂点情報
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
	ID3D12RootSignature* rootsignature; {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {}; {
			{
				D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};
				D3D12_DESCRIPTOR_RANGE MaterialTblRange[2] = {}; {
					//basicDescHeap
					descTblRange[0].NumDescriptors = 1;
					descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	//種別は定数
					descTblRange[0].BaseShaderRegister = 0;
					descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					descTblRange[1].NumDescriptors = 1;
					descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//種別はテクスチャ
					descTblRange[1].BaseShaderRegister = 0;
					descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					//materialDescHeap
					MaterialTblRange[0].NumDescriptors = 1;
					MaterialTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	//種別は定数
					MaterialTblRange[0].BaseShaderRegister = 1;
					MaterialTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					MaterialTblRange[1].NumDescriptors = 3;//テクスチャ2つ(基本とsph)
					MaterialTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//種別はテクスチャ
					MaterialTblRange[1].BaseShaderRegister = 1;//register1だと使えなかった
					MaterialTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}
				D3D12_ROOT_PARAMETER rootparam[2] = {}; {//[0]world,[1]material→違いはBaseShaderRegisterだけ
					//basicDescHeap
					rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
					rootparam[0].DescriptorTable.NumDescriptorRanges = 2;//2から1に変更→ここのテクスチャがあるとmaterialDescHeapのテクスチャが使えない?→このテクスチャがregister1なら問題なく通った
					//materialDescHeap
					rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					rootparam[1].DescriptorTable.pDescriptorRanges = &MaterialTblRange[0];
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
						HRESULT_(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob));
					}
				}
				HRESULT_(_dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature)));
				gpipeline.pRootSignature = rootsignature;
			}
			//rootSigBlob->Release();
			//shader作成---------------------------------------- 
			{
				ComPtr<ID3DBlob> errorBlob = nullptr;
				ID3DBlob* vsBlob = nullptr;
				HRESULT_(D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, errorBlob.GetAddressOf()));
				ID3DBlob* psBlob = nullptr;
				HRESULT_(D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, errorBlob.GetAddressOf()));
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

			HRESULT_(_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelineState.GetAddressOf())));
		}
	}

	//ビューポート&シザー短形作成----------------------
	D3D12_VIEWPORT viewport = {}; {
		viewport.Width = WINDOW_WIDTH; viewport.Height = WINDOW_HEIGHT; viewport.TopLeftX = 0; viewport.TopLeftY = 0; viewport.MaxDepth = 1.0f; viewport.MinDepth = 0.0f;
	}
	D3D12_RECT scissorrect = {}; {
		scissorrect.top = 0; scissorrect.left = 0; scissorrect.right = scissorrect.left + WINDOW_WIDTH; scissorrect.bottom = scissorrect.top + WINDOW_HEIGHT;
	}

	DirectInput keyboard(hIns, hwnd);
	//ループ作成--------------------------------------------------------------------------
	MSG msg = {};
	float angle = 0; bool a = false;
	while (true) {
		keyboard.Execute(hwnd);
		BYTE* control = keyboard.GetKBState();
		BYTE* oldControl = keyboard.GetOldKBState();
		BYTE tmp[256];
		memcpy(tmp, control, sizeof(BYTE) * 256);
		a = false;
		for (int i = 0; i < 256; i++)if (tmp[i] != 0)a = true;

		if (!a)angle += 0.1f;
		XMFLOAT3 eye(cos(angle), sin(angle) + 10, -10); XMFLOAT3 target(0.0f, 10.0f, 0.0f); XMFLOAT3 up(0, 1, 0);
		worldMat = XMMatrixRotationY(angle * 0.1f);
		viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		mapMatrix->world = worldMat;
		mapMatrix->view = viewMat;
		mapMatrix->proj = projMat;
		mapMatrix->eye = eye;


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

			_cmdList->IASetPrimitiveTopology(/*D3D_PRIMITIVE_TOPOLOGY_POINTLIST*/D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//頂点データの解釈
			_cmdList->IASetVertexBuffers(0, 1, &vbView);//毎フレームセットしないといけない
			_cmdList->IASetIndexBuffer(&ibView);//毎フレームセットしないといけない

			_cmdList->SetPipelineState(_pipelineState.Get());
			_cmdList->SetGraphicsRootSignature(rootsignature);//SetGraphicsRootDescriptorTableより前に書く→ヒープの設定情報？
			{
				_cmdList->SetDescriptorHeaps(1, &basicDescHeap);
				auto heapHandle = basicDescHeap/*materialDescHeap*/->GetGPUDescriptorHandleForHeapStart();//参考書との相違点->リソースと定数、どちらのバッファービューを先に入れるか次第
				heapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				_cmdList->SetGraphicsRootDescriptorTable(0, heapHandle);

				_cmdList->SetDescriptorHeaps(1, &materialDescHeap);
				auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
				unsigned int idxOffset = 0;
				auto cbvsrvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
				for (auto& m : materials) {
					_cmdList->SetGraphicsRootDescriptorTable(1, materialH);//第一引数(RootParameterIndex)
					_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
					materialH.ptr += cbvsrvIncSize;//0～19で色が変わる(水色、青、紺、薄い黄、白)それ以外で真っ黒→31白→関数SetPrimitive,SetVertex,SetIndexを上に移動で色ずれはある物の色分けはできた
					idxOffset += m.indicesNum;
				}
			}
			//_cmdList->DrawIndexedInstanced(indices.size()/*index数*/, 1, 0, 0, 0);

			HRESULT_(_cmdList->Close());
		}

		//IDXGISwapChain4動作-------------------------------
		{
			ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
			_cmdQueue->ExecuteCommandLists(1, cmdlists);

			HRESULT_(_cmdAllocator->Reset());
			HRESULT_(_cmdList->Reset(_cmdAllocator.Get(), nullptr));

			HRESULT_(_swapchain->Present(1, 0));
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
	return folderPath + "/" + texPath;
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

using LoadLambda_t = function<HRESULT(const wstring& path, TexMetadata*, ScratchImage&)>;
map<string, LoadLambda_t> loadLambdaTable;
map<string, ID3D12Resource*> _resourceTable;
ID3D12Resource* LoadTextureFromFile(string& texPath) {
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
#if 0
	auto result = LoadFromWICFile(GetWideStringFromString(texPath).c_str(), WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(result))return nullptr;
#else
	{
		auto it = _resourceTable.find(texPath);
		if (it != _resourceTable.end()) {
		
			return _resourceTable[texPath];
		}
	}
	
	//ラムダ式活用でエラー→ラムダ式を学ぶ→単純な引数の書き間違い
	loadLambdaTable["sph"] = loadLambdaTable["spa"] = loadLambdaTable["bmp"] = loadLambdaTable["png"] = loadLambdaTable["jpg"] =
		[](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT{return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img); };
	loadLambdaTable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT{return LoadFromTGAFile(path.c_str(), meta, img); };
	loadLambdaTable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT{return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, img); };
	auto wtexpath = GetWideStringFromString(texPath);
	auto ext = GetExtension(texPath);//拡張子を取得
	auto result = loadLambdaTable[ext](wtexpath, &metadata, scratchImg);
#endif 
	auto img = scratchImg.GetImage(0, 0, 0);

	//WriteToSubresourcceで転生する用のヒープ設定
	ID3D12Resource* texbuff = nullptr; {
		D3D12_HEAP_PROPERTIES texHeapProp = {}; {
			texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
			texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			texHeapProp.CreationNodeMask = 0;
			texHeapProp.VisibleNodeMask = 0;
		}
		D3D12_RESOURCE_DESC resDesc = {}; {
			resDesc.Format = metadata.format;
			resDesc.Width = metadata.width;
			resDesc.Height = metadata.height;
			resDesc.DepthOrArraySize = metadata.arraySize;
			resDesc.SampleDesc.Count = 1;
			resDesc.SampleDesc.Quality = 0;
			resDesc.MipLevels = metadata.mipLevels;
			resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
			resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		}
		result = _dev->CreateCommittedResource(&texHeapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texbuff));
		if (FAILED(result))return nullptr;
	}
	result = texbuff->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
	if (FAILED(result))return nullptr;
	_resourceTable[texPath] = texbuff;
	return texbuff;
}
ID3D12Resource* CreateWhiteTexture() {
	ID3D12Resource* whiteBuff = {}; {
		D3D12_HEAP_PROPERTIES texHeapProp = {}; {
			texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM; texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; texHeapProp.VisibleNodeMask = 0;// texHeapProp.CreationNodeMask = 0;
		}
		D3D12_RESOURCE_DESC resDesc = {}; {
			resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; resDesc.Width = 4; resDesc.Height = 4; resDesc.DepthOrArraySize = 1; resDesc.SampleDesc.Count = 1; resDesc.SampleDesc.Quality = 0;
			resDesc.MipLevels = 1; resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		}
		auto result = _dev->CreateCommittedResource(&texHeapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&whiteBuff));
		if (FAILED(result))return nullptr;
	}
	vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);
	auto result = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	return whiteBuff;
}
ID3D12Resource* CreateBlackTexture() {
	ID3D12Resource* whiteBuff = {}; {
		D3D12_HEAP_PROPERTIES texHeapProp = {}; {
			texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM; texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; texHeapProp.VisibleNodeMask = 0;// texHeapProp.CreationNodeMask = 0;
		}
		D3D12_RESOURCE_DESC resDesc = {}; {
			resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; resDesc.Width = 4; resDesc.Height = 4; resDesc.DepthOrArraySize = 1; resDesc.SampleDesc.Count = 1; resDesc.SampleDesc.Quality = 0;
			resDesc.MipLevels = 1; resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		}
		auto result = _dev->CreateCommittedResource(&texHeapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&whiteBuff));
		if (FAILED(result))return nullptr;
	}
	vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);
	auto result = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	return whiteBuff;
}
string GetExtension(const string& path) {
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}
pair<string, string> SplitFileName(const string& path, const char splitter) {
	int idx = path.find(splitter);
	pair<string, string> ret;;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}