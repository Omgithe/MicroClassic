#pragma once

#include "Vector.h"

class DrawManager;

class RenderWindow
{
public:
	~RenderWindow();

	bool Initialize(DrawManager*, std::string, int32, int32);
	bool ProcessMessages();

	inline int32 GetWidth() { return m_iWidth; }
	inline int32 GetHeight() { return m_iHeight; }
	inline HWND GetHandle() { return m_hWindow; }
private:
	int32 m_iWidth = 0;
	int32 m_iHeight = 0;
	WNDCLASSEX m_wcWndClass{ 0 };
	HWND m_hWindow = NULL;
};