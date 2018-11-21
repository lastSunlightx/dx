#include "D3D12App.h"
#include "Win32Application.h"

using Microsoft::WRL::ComPtr;
using namespace::DirectX;
using namespace std;

D3D12App::D3D12App(UINT width, UINT height, std::wstring name):
	m_width(width),
	m_height(height),
	m_title(name),
	m_useWarpDevice(false)
{
	
}

D3D12App::~D3D12App()
{
	if (m_d3dDevice != nullptr)
		FlushCommandQueue();
}

int D3D12App::Run()
{
	// Main sample loop.
	MSG msg = { 0 };

	m_Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else//û��ϵͳ��Ϣ��������Ϸѭ��
		{
			m_Timer.Tick();

			if (!m_AppPaused)
			{
				CalculateFrameStats();//ʱ����Ϣ����
				OnUpdate(m_Timer);
				OnRender(m_Timer);
			}
			else
			{
				Sleep(100);
			}
		}

	}


	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}



void D3D12App::OnResize()
{
	//��Դ���
	assert(m_d3dDevice);
	assert(m_SwapChain);
	assert(m_DirectCmdListAlloc);
	
	// ������Դǰȷ��gpuִ����������ָ��,����resut����ǰ����ִ��
	FlushCommandQueue();

	//���Alloc��ָ�������Ҫ��������
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(),nullptr));

	//reset buffers
	for (int i = 0; i < SwapChainBufferCount; ++i)
		m_SwapChainBuffer[i].Reset();
	m_DepthStencilBuffer.Reset();

	//resize swap chain
	ThrowIfFailed(m_SwapChain->ResizeBuffers(
		SwapChainBufferCount,
		m_width,m_height,
		m_BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	));

	m_CurrBackBuffer = 0;

	//-----------------------------------------//
	//************** DX12��ʼ�����߲� *****************//
	//˵��������RTV��DSV
	//-----------------------------------------//
	//create render target view
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_HeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_d3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtv_HeapHandle);
		rtv_HeapHandle.Offset(1, m_RtvDescriptorSize);
	}

	//Create depth/stencli buffer and view
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	//ע������������������߱���Ӧ����RSVbuffer��������
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),//��Դ����
		D3D12_HEAP_FLAG_NONE,//��Դ��־
		&depthStencilDesc,//��Դ��Ϣ�ṹ��
		D3D12_RESOURCE_STATE_COMMON,//��Դ״̬
		&optClear,//������� D3D12_CLEAR_VALUE
		IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));//��Ҫ��������Դ

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_d3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition(����) the resource from its initial state to be used as a depth buffer.
	//����Դ״̬��commonת����depthwrite
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	//-----------------------------------------//
	//************** DX12��ʼ���ڰ˲� *****************//
	//˵���������ӿ�
	//-----------------------------------------//
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_width);
	m_ScreenViewport.Height = static_cast<float>(m_height);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_width, m_height };
}

void D3D12App::OnInit()
{
	//���ó�����Կ��ƣ�dx12����
	UINT dxgiFactoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
	// Enable additional debug layers.
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif


	//-----------------------------------------//
	//************** DX12��ʼ����һ�� *****************//
	//˵������ȡdxFactory������Device
	//-----------------------------------------//
	
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	if(m_useWarpDevice)//������ʹ��warp�豸
	{ 
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,//��С��Ҫ֧�ֵ������ȼ�
			IID_PPV_ARGS(&m_d3dDevice)
		);
	}
	else
	{
		//�����豸
		ThrowIfFailed(D3D12CreateDevice(
		    nullptr,//default adapter
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_d3dDevice)
		));
		
	}

	//-----------------------------------------//
	//************** DX12��ʼ���ڶ��� *****************//
	//˵�������� Fence �� Desciptor size
	//-----------------------------------------//
	ThrowIfFailed(m_d3dDevice->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&m_Fence))
	);
	m_RtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_CbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	//-----------------------------------------//
	//************** DX12��ʼ�������� *****************//
	//˵����4X��� ��ȡ adapter��Ϣ
	//-----------------------------------------//
	
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_QualityLevels;
	ms_QualityLevels.Format = m_BackBufferFormat;
	ms_QualityLevels.SampleCount = 4;
	ms_QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	ms_QualityLevels.NumQualityLevels = 0;

	ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&ms_QualityLevels,
		sizeof(ms_QualityLevels)
	));
	m_4xMsaaQuality = ms_QualityLevels.NumQualityLevels;
	//��ȡ��������Ϣ

	LogAdapters();


	//-----------------------------------------//
	//************** DX12��ʼ�����Ĳ� *****************//
	//˵����ָ���齨���� command queue  allocator  command list
	//-----------------------------------------//
	CreateCommandObjects();

	//-----------------------------------------//
	//************** DX12��ʼ�����岽 *****************//
	//˵��������������
	//-----------------------------------------//
	CreateSwapChain();

	//-----------------------------------------//
	//************** DX12��ʼ�������� *****************//
	//˵��������RTV��DSV�� DescriptorHeaps
	//-----------------------------------------//
	CreateRtvAndDsvDescriptorHeaps();


	//���߰˲�������OnRisize�����У���Ϊ����������Ҫ�ڻ����������ı�ʱ��������

	//�ߣ�����rtv ��  dsv
	//�ˣ������ӿ�
	OnResize();
}

void D3D12App::OnDestroy()
{
}

float D3D12App::AspectRatio() const
{
	//�ص�  ����λ��Ҫ�����,���������͸���������ܴ� һ��Ҫϸ�ġ�
	return static_cast<float>(m_width)/m_height;
}


void D3D12App::CreateCommandObjects()
{

	//����GPUָ�����
	//-------��дָ����нṹ------------//
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//��һ��Ϊbundle
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateCommandQueue(
	     &queueDesc,
         IID_PPV_ARGS(&m_CommandQueue)
	));


	//����ָ������������gpu��ָ�������������������
	ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
	    D3D12_COMMAND_LIST_TYPE_DIRECT,
	    IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())
	));


	//����cpuָ������
	ThrowIfFailed(m_d3dDevice->CreateCommandList(
	    0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_DirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(&m_CommandList)
	));


	m_CommandList->Close();
	
}



void D3D12App::CreateSwapChain()
{
	//����������
	m_SwapChain.Reset();

	//��д�����������ṹ,�ýṹ��dx11����ͬ��
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_width;//��̨���������
	sd.BufferDesc.Height = m_height;//��̨�������߶�
	sd.BufferDesc.RefreshRate.Numerator = 60;//��ʾģʽ�ĸ���Ƶ��  ����
	sd.BufferDesc.RefreshRate.Denominator = 1;//��ĸ
	sd.BufferDesc.Format = m_BackBufferFormat;//��ʽ
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;//ɨ����ģʽ δָ��
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;//���� δָ��
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = Win32Application::GetHwnd();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
		m_CommandQueue.Get(),
		&sd,
		m_SwapChain.GetAddressOf()
	));
	
}

void D3D12App::CreateRtvAndDsvDescriptorHeaps()
{
	//������Դ����
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void D3D12App::FlushCommandQueue()
{
	
	//˵����
	/*
	   ��������Ҫ������ͬ��gpu��cpu��m_CurrentFence�Ǽ�¼cpu��ֵ����fence�е�ֵ����gpu����Signal��������cpu������ָ�gpu�������һ��ָ�
	   ����gpu��fence�е�ֵ��1�����gpuִ��������ָ����fence�е�ֵ��m_CurrentFence��ͬ��˵��ָ��ȫ��ִ����ϡ�����Ϊִ����
	   ����ָ�fence�е�ֵ��m_CurrentFenceС1��
	*/

	m_CurrentFence++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));//Sets the fence to the specified value.
	UINT FenceValue = (UINT)m_Fence->GetCompletedValue();
	bool isEndCommand = FenceValue < m_CurrentFence;

	// Wait until the GPU has completed commands up to this fence point.
	if (isEndCommand)//Gets the current value of the fence.
	{
		m_CurrentFence;
		FenceValue;
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));//Specifies an event that should be fired when the fence reaches a certain value.				
		WaitForSingleObject(eventHandle, INFINITE);// Wait until the GPU hits current fence event is fired.
		CloseHandle(eventHandle);
	}
}

ID3D12Resource * D3D12App::CurrentBackBuffer() const
{
	return m_SwapChainBuffer[m_CurrBackBuffer].Get();;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::DepthStencilView() const
{
	return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void D3D12App::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		DXGI_ADAPTER_DESC desc;
		if (m_AdapterList[0])
		{
			m_AdapterList[0]->GetDesc(&desc);
		}
		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = m_title +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"   Adaptor: " + desc.Description +
			L"   cWidth:" + to_wstring(m_width) +
			L"   cHeight:" + to_wstring(m_height);

		SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
		
		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void D3D12App::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;

	while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());
		m_AdapterList.push_back(adapter);

		++i;
	}
}

void D3D12App::LogAdapterOutputs(IDXGIAdapter * adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_BackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void D3D12App::LogOutputDisplayModes(IDXGIOutput * output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}

}

std::wstring D3D12App::GetAssetFullPath(LPCWSTR assetName)
{
	return std::wstring();
}

void D3D12App::GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
}

void D3D12App::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}