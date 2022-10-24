#pragma once

#include "Define.h"
class WindowContainer;

class Overlay
{
public:
	~Overlay();
	bool Init(HWND, WindowContainer*);
	bool ProcessMessages();
	inline HWND GetWindowHandle() { return m_hWindow; }
	void TakeTargetShape(HWND);
private:
	HWND m_hWindow = NULL;
	WNDCLASSEX m_wcWndClass{0};
	int32 m_iWidth = 0;
	int32 m_iHeight = 0;
};