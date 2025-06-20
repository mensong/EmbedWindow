#pragma once
#ifndef __AFX_H__
#include <windows.h>
#endif // !AFX
#include <hash_map>

class EmbedWindowHelper
{
public:
	struct ChildInfo
	{
		WNDPROC m_OldProc = NULL;
		HWND m_hChild = NULL;
		LONG_PTR m_style = 0;
	};
	
public:
	static bool EmbedWindow(HWND hContainer, HWND hChildWindow);
	static HWND UnembedWindow(HWND hContainer);

protected:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static std::hash_map<HWND, ChildInfo> s_parentMapToChildInfo;
};

