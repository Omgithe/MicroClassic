#include "RenderWindow.h"
#include "DrawManager.h"

bool RenderWindow::ProcessMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (PeekMessage(&msg, m_hWindow, 0,	0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_NULL)
	{
		if (!IsWindow(m_hWindow))
		{
			m_hWindow = NULL;
			UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
			return false;
		}
	}

	return true;
}

RenderWindow::~RenderWindow()
{
	if (m_hWindow != NULL)
	{
		UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
		DestroyWindow(m_hWindow);
	}
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	return reinterpret_cast<DrawManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->WindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		DrawManager* pWindow = reinterpret_cast<DrawManager*>(pCreate->lpCreateParams);
		if (pWindow == nullptr)
		{
			exit(-1);
		}

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool RenderWindow::Initialize(DrawManager* pDrawManager, std::string szName, int32 w, int32 h)
{
	std::string szClassName = "C" + szName;
	m_wcWndClass = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		HandleMsgSetup,
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
		NULL,
		m_wcWndClass.lpszClassName,
		szWndName.c_str(),
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		100,
		100,
		w,
		h,
		NULL,
		NULL,
		m_wcWndClass.hInstance,
		pDrawManager
	);

	if (m_hWindow == NULL)
	{
		UnregisterClass(m_wcWndClass.lpszClassName, m_wcWndClass.hInstance);
		return false;
	}

	ShowWindow(m_hWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_hWindow);

	return true;
}