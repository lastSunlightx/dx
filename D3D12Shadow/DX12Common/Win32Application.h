
//-------------------------------------------------------------
//------------------- Win32 ���  -------------------------
//ʱ�䣺 2018-9-5     by:��һĨϦϼ
//˵����DX12��Win32��ܳ���
//-------------------------------------------------------------


#pragma once


#include "D3D12App.h"

using namespace std;


class D3D12App;


class Win32Application
{
public:
	static int Init(D3D12App *m_D3D12App, HINSTANCE hInstance, int nCmdShow);
	
	static int Run(D3D12App *m_D3D12App );

	static HWND GetHwnd() { return m_hwnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};