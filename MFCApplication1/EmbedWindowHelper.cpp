#include "EmbedWindowHelper.h"
#include <windows.h>

std::hash_map<HWND, EmbedWindowHelper::ChildInfo> EmbedWindowHelper::s_parentMapToChildInfo;

LRESULT CALLBACK EmbedWindowHelper::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::hash_map<HWND, ChildInfo>::iterator itFinder = s_parentMapToChildInfo.find(hWnd);
	if (itFinder == s_parentMapToChildInfo.end())
	{
		return 0;
	}

	if (uMsg == WM_SIZE || uMsg == WM_PAINT)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		if (uMsg == WM_SIZE)
			SetWindowPos(itFinder->second.m_hChild, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);

		::InvalidateRect(itFinder->second.m_hChild, &rc, TRUE);
		::UpdateWindow(itFinder->second.m_hChild);
	}

	// 调用原来的回调函数
	LRESULT lReturn = (LONG)CallWindowProc(itFinder->second.m_OldProc, hWnd, uMsg, wParam, lParam);
	return lReturn;
}

bool EmbedWindowHelper::EmbedWindow(HWND hContainer, HWND hChildWindow)
{
	if (!hContainer || !hChildWindow)
		return false;

	ChildInfo ci;

	// 修改窗口样式
	ci.m_style = GetWindowLongPtr(hChildWindow, GWL_STYLE);
	LONG_PTR style = ci.m_style;
	style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME);
	style |= WS_CHILD;
	SetWindowLongPtr(hChildWindow, GWL_STYLE, style);

	ci.m_hChild = hChildWindow;
	ci.m_OldProc = (WNDPROC)SetWindowLongPtr(hContainer, GWLP_WNDPROC, (LONG_PTR)WndProc);

	if (SetParent(hChildWindow, hContainer))
	{
		s_parentMapToChildInfo.insert(std::make_pair(hContainer, std::move(ci)));

		RECT rc;
		GetClientRect(hContainer, &rc);
		SetWindowPos(hChildWindow, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
		return true;
	}

	SetWindowLongPtr(hChildWindow, GWL_STYLE, ci.m_style);
	return false;
}

HWND EmbedWindowHelper::UnembedWindow(HWND hContainer)
{
	if (!hContainer)
		return NULL;

	std::hash_map<HWND, ChildInfo>::iterator itFinder = s_parentMapToChildInfo.find(hContainer);
	if (itFinder == s_parentMapToChildInfo.end())
	{
		return NULL;
	}

	HWND hChild = itFinder->second.m_hChild;

	// restore修改窗口样式
	SetWindowLongPtr(itFinder->second.m_hChild, GWL_STYLE, itFinder->second.m_style);

	// resotore
	SetWindowLongPtr(hContainer, GWLP_WNDPROC, (LONG_PTR)itFinder->second.m_OldProc);

	SetParent(itFinder->second.m_hChild, ::GetDesktopWindow());

	s_parentMapToChildInfo.erase(itFinder);
	return hChild;
}
