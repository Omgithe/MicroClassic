#include "Overlay.h"
#include "WindowContainer.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

Overlay::~Overlay()
{
	if (m_hWindow != NULL)
	{
		DestroyWindow(m_hWindow);
		UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
	}
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	return reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->WindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowContainer* pWindow = reinterpret_cast<WindowContainer*>(pCreate->lpCreateParams);

		if (pWindow == nullptr)
			exit(-1);
		
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Overlay::Init(HWND hWindow, WindowContainer* pWndContainer)
{
	std::string szName = std::to_string((uint64)hWindow);
	std::string szClassName = "C" + szName;
	m_wcWndClass = { 
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		HandleMessageSetup,
		0L,
		0L,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		szClassName.c_str(),
		NULL
	};

	RegisterClassEx(&m_wcWndClass);

	std::string szWndName = "W" + szName;

	m_hWindow = CreateWindowExA(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		m_wcWndClass.lpszClassName,
		szWndName.c_str(),
		WS_POPUP,
		100,
		100,
		800,
		600,
		NULL,
		NULL,
		m_wcWndClass.hInstance,
		pWndContainer
	);

	if (m_hWindow == NULL)
	{
		UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
		return false;
	}

	if (!SetLayeredWindowAttributes(m_hWindow, 0x00FFFFFF, 255, LWA_ALPHA))
	{
		UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
		return false;
	}

	MARGINS margins = { -1 , -1, -1, -1 };
	if (FAILED(DwmExtendFrameIntoClientArea(m_hWindow, &margins)))
	{
		DestroyWindow(m_hWindow);
		UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
		return false;
	}

	ShowWindow(m_hWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_hWindow);

	return true;
}

void Overlay::TakeTargetShape(HWND hTargetWindow)
{
	RECT wr;
	GetWindowRect(hTargetWindow, &wr);

	RECT cr;
	GetClientRect(hTargetWindow, &cr);

	int ww = (wr.right - wr.left);
	int wh = (wr.bottom - wr.top);

	int cw = (cr.right - cr.left);
	int ch = (cr.bottom - cr.top);

	m_iWidth = cw;
	m_iHeight = ch;

	int dw = ww - cw;
	int dh = wh - ch;
	int border = dw / 2;

	MoveWindow(m_hWindow, wr.left + border, wr.top + dh - border, cw, ch, false);
}

bool Overlay::ProcessMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (PeekMessage(&msg, this->m_hWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_NULL)
	{
		if (!IsWindow(this->m_hWindow))
		{
			this->m_hWindow = NULL;
			UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
			return false;
		}
	}

	return true;
}