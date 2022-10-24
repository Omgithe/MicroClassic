#pragma once

#include "Overlay.h"
#include "Graphics.h"

class WindowContainer
{
public:
	WindowContainer()
	{
		m_Overlay = new Overlay();
		m_Graphics = new Graphics();
	}

	~WindowContainer()
	{
		delete m_Overlay;
		delete m_Graphics;
	}

	bool Init(HWND hWindow)
	{
		if (m_Overlay->Init(hWindow, this))
		{
			if (m_Graphics->Init(m_Overlay->GetWindowHandle()))
			{
				return true;
			}
		}

		return false;
	}

	LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		GetGraphics()->OnWndProc(msg, wParam, lParam);

		switch (msg)
		{
		case WM_SIZE:
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU)
				return 0;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	inline bool ProcessMessages()
	{
		return m_Overlay->ProcessMessages();
	}

	inline Graphics* GetGraphics() { return m_Graphics; }
	inline Overlay* GetOverlay() { return m_Overlay; }

private:
	Overlay* m_Overlay = nullptr;
	Graphics* m_Graphics = nullptr;
};