#include "Overlay.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

using namespace DirectX;
using namespace SimpleMath;
using Microsoft::WRL::ComPtr;

Overlay::Overlay()
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
}

Overlay::~Overlay()
{
	if (m_hWindow != NULL)
	{
		UnregisterClass(m_WndClass.lpszClassName, m_WndClass.hInstance);
		DestroyWindow(m_hWindow);
	}
}

bool Overlay::ProcessMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (PeekMessage(&msg, m_hWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_NULL)
	{
		if (!IsWindow(m_hTargetWindow) || !IsWindow(m_hWindow))
		{
			if (m_hWindow != NULL)
				DestroyWindow(m_hWindow);
			
			UnregisterClass(m_WndClass.lpszClassName, m_WndClass.hInstance);
			return false;
		}
	}

	POINT lp{0};
	ClientToScreen(m_hTargetWindow, &lp);

	RECT cr{0};
	GetClientRect(m_hTargetWindow, &cr);

	MoveWindow(m_hWindow, lp.x, lp.y, cr.right - cr.left, cr.bottom - cr.top, false);

	return true;
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	return reinterpret_cast<Overlay*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->WindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Overlay* pWindow = reinterpret_cast<Overlay*>(pCreate->lpCreateParams);
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

void Overlay::Attach(Process* pProcess)
{
	m_hTargetWindow = pProcess->GetWindowHandle();
	
	m_WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, HandleMsgSetup, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "std::wdasdsa", NULL };
	RegisterClassEx(&m_WndClass);

	RECT cr;
	GetClientRect(m_hTargetWindow, &cr);
	int m_iWidth = cr.right - cr.left;
	int m_iHeight = cr.bottom - cr.top;

	m_hWindow = CreateWindowExA(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, //WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		m_WndClass.lpszClassName,
		"dsadsaswqedqfdsv",
		WS_POPUP,
		100,
		100,
		m_iWidth,
		m_iHeight,
		NULL,
		NULL,
		m_WndClass.hInstance,
		this
	);

	if (!SetLayeredWindowAttributes(m_hWindow, RGB(0, 0, 0), 255, LWA_ALPHA))
	{
		UnregisterClass(m_WndClass.lpszClassName, m_WndClass.hInstance);
	}

	MARGINS margins = { -1 , -1, -1, -1 };
	DwmExtendFrameIntoClientArea(m_hWindow, &margins);

	ShowWindow(m_hWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_hWindow);
}

LRESULT Overlay::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		m_deviceResources->SetWindow(hWnd, m_iWidth, m_iHeight);
		m_deviceResources->CreateDeviceResources();
		CreateDeviceDependentResources();
		m_deviceResources->CreateWindowSizeDependentResources();
		CreateWindowSizeDependentResources();
		break;
	case WM_MOVE:
		auto const r = m_deviceResources->GetOutputSize();
		m_deviceResources->WindowSizeChanged(r.right, r.bottom);
		break;
	case WM_SIZE:
		m_iWidth = (UINT)LOWORD(lParam);
		m_iHeight = (UINT)HIWORD(lParam);

		if (!m_deviceResources->WindowSizeChanged(m_iWidth, m_iHeight))
			break;

		CreateWindowSizeDependentResources();

		break;
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

void Overlay::OnDeviceLost()
{
	m_states.reset();
	m_effect.reset();
	m_batch.reset();
	m_inputLayout.Reset();
}

void Overlay::OnDeviceRestored()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void Overlay::Render()
{
	Clear();

	auto context = m_deviceResources->GetD3DDeviceContext();


	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthNone(), 0);
	context->RSSetState(m_states->CullNone());

	m_effect->Apply(context);

	context->IASetInputLayout(m_inputLayout.Get());

	m_batch->Begin();

	VertexPositionColor v1(Vector3(400.f, 150.f, 0.f), Colors::Red);
	VertexPositionColor v2(Vector3(600.f, 450.f, 0.f), Colors::Green);
	VertexPositionColor v3(Vector3(200.f, 450.f, 0.f), Colors::Blue);

	//for (int i = 0; i< 100000; i++)
	m_batch->DrawTriangle(v1, v2, v3);

	//for (int i = 0; i < 100000;i++)
	//	m_batch->DrawLine(v1, v2);

	float x = 150.f;
	float y = 150.f;
	float w = 150.f;
	float h = 150.f;

	const int segments = 128;
	float radius = 45.f;

	VertexPositionColor v[segments];
	uint16_t Indices[segments * 2];
	ZeroMemory(Indices, std::size(Indices));

	int offset = 0;
	int k = 0;
	for (int i = 0; i < segments; i++)
	{
		float theta = 2.f * XM_PI * static_cast<float>(i) / static_cast<float>(segments);
		v[i] = { Vector3(x + radius * std::cos(theta), y + radius * std::sin(theta), 0.f), Colors::Green };
		Indices[k] = offset;
		Indices[k + 1] = offset;
		offset++;
		k += i == 0 ? 1 : 2;
	}
	
	Indices[(segments * 2) - 1] = 0;
	

	m_batch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST, Indices, segments * 2, v, segments);

	m_batch->End();

	m_deviceResources->Present();
}

void Overlay::Clear()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

	float clear_color[4] = {0, 0.1f, 0.25f, .2f};
	context->ClearRenderTargetView(renderTarget, clear_color);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	auto const viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);
}

void Overlay::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

	m_states = std::make_unique<CommonStates>(device);

	m_effect = std::make_unique<BasicEffect>(device);
	m_effect->SetVertexColorEnabled(true);

	DX::ThrowIfFailed(
		CreateInputLayoutFromEffect<DirectX::VertexPositionColor>(device, m_effect.get(),
			m_inputLayout.ReleaseAndGetAddressOf())
	);

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_batch = std::make_unique<PrimitiveBatch<DirectX::VertexPositionColor>>(context);
}

void Overlay::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();

	Matrix proj = Matrix::CreateScale(2.f / float(size.right),
		-2.f / float(size.bottom), 1.f)
		* Matrix::CreateTranslation(-1.f, 1.f, 0.f);
	m_effect->SetProjection(proj);
}