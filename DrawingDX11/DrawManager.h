#pragma once

#include "RenderWindow.h"
#include "Renderer.h"

class DrawManager
{
public:
	DrawManager();
	~DrawManager();
	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool ProcessMessages();
	bool Initialize();
	void OnPresent() { m_Renderer.Present(); }
private:
	RenderWindow m_RenderWindow;
	Renderer m_Renderer;

};