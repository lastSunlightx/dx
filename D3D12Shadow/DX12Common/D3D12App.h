
//-------------------------------------------------------------
//------------------- D3D12������  -------------------------
//ʱ�䣺2018-9-6    by:   ��һĨϦϼ
//˵����D3D����ʹ�õĻ������
//-------------------------------------------------------------
#pragma once
#include "D3D12Util.h"



class D3D12App
{
public:
	//��ʼ������
	D3D12App(UINT width,UINT height,std::wstring name);
	D3D12App(const D3D12App& rhs) = delete;
	D3D12App& operator=(const D3D12App& rhs) = delete;
	virtual ~D3D12App();
	
	int Run();
public:
	//���Ը��������Ƶĺ���
	virtual void CreateRtvAndDsvDescriptorHeaps();
	virtual void OnResize();
	virtual void OnInit();//�豸��ʼ������Դ��ʼ��
	virtual void OnDestroy();
	//ʱ����ƺ�fps����
	virtual void CalculateFrameStats();

	//������º���Ⱦ����
	virtual void OnUpdate(const GameTimer& gt) = 0;
	virtual void OnRender(const GameTimer& gt) = 0;

	//������Ϣ������
	virtual void OnKeyDown(UINT8 /*key*/) {}
	virtual void OnKeyUp(UINT8 /*key*/) {}


	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }



public:
	//��ȡ������Ϣ
	float  AspectRatio()const;
	int  GetWidth() const { return m_width; }
	int  GetHeight() const { return m_height; }

	const WCHAR* GetTitle() const { return m_title.c_str(); }
	GameTimer &GetTimer() { return m_Timer; }

	bool      getPaused() const { return m_AppPaused; };
	bool      getMinimized() const { return m_Minimized; };
	bool      getMaximized() const { return m_Maximized; };
	bool      getResizing() const { return m_Resizing; };
	bool      get4xMsaaState() const { return m_4xMsaaState; }
	bool      IsDevice() { return m_d3dDevice; }
	//���ô�����Ϣ
	void      setWidth(int width) { m_width = width; }
	void      setHeight(int height) { m_height = height; }

	void      setPaused(bool isPaused) { m_AppPaused = isPaused; }
	void      setMinimized(bool isMinimized) { m_Minimized = isMinimized; }
	void      setMaximized(bool isMaximized) { m_Maximized = isMaximized; }
	void      setResizing(bool isResizing) { m_Resizing = isResizing; }
	void      set4xMsaaState(bool is4xMsaaState) { m_4xMsaaState = is4xMsaaState; }
	//��ȡ��������Ϣ
	void GetAdpterInfo() {};

	
protected:

	void CreateCommandObjects();

	void CreateSwapChain();

	void FlushCommandQueue();//�ȴ�gpuִ��������ָ�ʹcpu��gpu����

	//��ȡ������Ϣ
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	

	//��������Ϣ
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);


	std::wstring GetAssetFullPath(LPCWSTR assetName);//��ȡ�ʲ�·��
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
	void SetCustomWindowText(LPCWSTR text);


protected:

	//�봰����Ϣ��ʱ�亯�����ʹ��
	bool      m_AppPaused = false;  // is the application paused?
	bool      m_Minimized = false;  // is the application minimized?
	bool      m_Maximized = false;  // is the application maximized?
	bool      m_Resizing = false;   // are the resize bars being dragged?
	bool      m_FullscreenState = false;// fullscreen enabled

	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool      m_4xMsaaState = false;    // 4X MSAA enabled
	UINT      m_4xMsaaQuality = 0;      // quality level of 4X MSAA

	//��������ӿ�
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;

	//cpu��gpu��ͬ��
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_CurrentFence = 0;

	

	//d3d12ָ��ӿ�
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	//��Ⱦ�ܵ���ؽӿ�
	static const int SwapChainBufferCount = 2;
	int m_CurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;//UI�������򣬸����򲻻����ͼ�λ���

	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;
	UINT m_CbvSrvDescriptorSize = 0;
	//�Ƿ��������������
	bool m_useWarpDevice = false;
	
	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//��������Ϣ
	std::vector<IDXGIAdapter*> m_AdapterList;

	//�ӿڲ���
	int m_width;
	int m_height;
	
	

	// Used gamrTimer
	GameTimer m_Timer;
	
	

private:
	//�ʲ�·��������hlsl�ļ���
	std::wstring m_assetsPath;

	//window ����
	std::wstring m_title;

};
