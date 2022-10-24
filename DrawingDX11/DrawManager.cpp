#include "DrawManager.h"

DrawManager::DrawManager()
{
	
}

DrawManager::~DrawManager()
{
	
}

LRESULT DrawManager::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			m_Renderer.Resize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool DrawManager::ProcessMessages()
{
	m_Renderer.Present();
	return m_RenderWindow.ProcessMessages();
}

bool DrawManager::Initialize()
{
	if (!m_RenderWindow.Initialize(this, "Test", 800, 600))
		return false;
	
	if (!m_Renderer.Initialize(m_RenderWindow.GetHandle(), 800, 600))
		return false;
	
	return true;
}