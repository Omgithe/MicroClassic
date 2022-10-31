#include "Overlay.h"

#include <dwmapi.h>
#include <algorithm>
#pragma comment(lib, "dwmapi.lib")

using namespace DirectX;
using namespace SimpleMath;
using Microsoft::WRL::ComPtr;

Overlay::Overlay()
{
	m_deviceResources = std::make_unique<DX::DeviceResources>(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_UNKNOWN);

	m_deviceResources->RegisterDeviceNotify(this);

	m_MSAA = std::make_unique<DX::MSAA>(
		m_deviceResources->GetBackBufferFormat(),
		DXGI_FORMAT_D32_FLOAT,
		16);
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
	
	m_WndClass = {
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
		"std::wdasdsa",
		NULL 
	};

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

		m_timer.SetFixedTimeStep(true);
		m_timer.SetTargetElapsedSeconds(1.0 / 60);
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
	m_MSAA->ReleaseDevice();
}

void Overlay::OnDeviceRestored()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void Overlay::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	Clear();

	auto context = m_deviceResources->GetD3DDeviceContext();


	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthNone(), 0);
	context->RSSetState(m_states->CullCounterClockwise());

	m_effect->Apply(context);

	context->IASetInputLayout(m_inputLayout.Get());

	m_batch->Begin();

	float x = 0;
	float y = 0;
	float r = 100;

	DrawRect({ x, y , 300, 100 }, 30, { 1,0,1,1 });
	DrawLine({x, y}, {300, 100});
	//DrawRectFilled({ x, y + 110, 300, 100 }, { 1,0,1,1 });
	//DrawRectFilled({ x, y + 110 }, { 300, 100 }, { 0,0,0,0.5f });
	

	m_batch->End();

	m_MSAA->Resolve(context, m_deviceResources->GetRenderTarget());
	m_deviceResources->Present();
}

void Overlay::Clear()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	
	auto renderTarget = m_MSAA->GetMSAARenderTargetView();
	auto depthStencil = m_MSAA->GetMSAADepthStencilView();

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

	m_MSAA->SetDevice(device);
}

void Overlay::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();

	Matrix proj = Matrix::CreateScale(2.f / float(size.right),
		-2.f / float(size.bottom), 1.f)
		* Matrix::CreateTranslation(-1.f, 1.f, 0.f);

	m_effect->SetProjection(proj);

	// Set window size for MSAA.
	m_MSAA->SetWindow(size);
}

void Overlay::DrawLine(Vector2 start, Vector2 end, Color color)
{
	VertexPositionColor v[]
	{
		{ {start.x,	start.y, 0.f}, color },
		{ {end.x, end.y, 0.f}, color }
	};

	m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST, v, std::size(v));
}

void Overlay::DrawTriangleFilled(Vector2 v1, Vector2 v2, Vector3 v3, Color c)
{
	VertexPositionColor vert1{ Vector3(v1.x, v1.y, 0), c };
	VertexPositionColor vert2{ Vector3(v2.x, v2.y, 0), c };
	VertexPositionColor vert3{ Vector3(v3.x, v3.y, 0), c };
	m_batch->DrawTriangle(vert1, vert2, vert3);
}

void Overlay::DrawRect(Vector4 rect, float strokeWidth, Color c)
{
	Vector4 tmp = rect;

	tmp.z = strokeWidth;
	DrawRectFilled(tmp, c);
	tmp.x = rect.x + rect.z - strokeWidth;
	DrawRectFilled(tmp, c);
	tmp.z = rect.z;
	tmp.x = rect.x;
	tmp.w = strokeWidth;
	DrawRectFilled(tmp, c);
	tmp.y = rect.y + rect.w - strokeWidth;
	DrawRectFilled(tmp, c);
}

void Overlay::DrawRectFilled(Vector4 rect, Color c)
{
	VertexPositionColor v[]
	{
		{ {rect.x,			rect.y,				0.f}, c },
		{ {rect.x + rect.z,	rect.y,				0.f}, c },
		{ {rect.x,			rect.y + rect.w,	0.f}, c },

		{ {rect.x + rect.z,	rect.y,				0.f}, c },
		{ {rect.x + rect.z,	rect.y + rect.w,	0.f}, c },
		{ {rect.x,			rect.y + rect.w,	0.f}, c }
	};

	m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, v, std::size(v));
}

void Overlay::DrawCircle(Vector2 v1, float r, Color c)
{
	const int num = (int)((2.f * XM_PI) / acosf(1.f - (2.f / (r * 2.f))));
	std::vector<VertexPositionColor> vertices;
	for (int i = 0; i <= num; i++)
		vertices.push_back({ { v1.x + r * cosf(XM_PI * ((float)i / (num / 2.f))) , v1.y + r * sinf(XM_PI * ((float)i / (num / 2.f))), 0 }, c });

	m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices.data(), vertices.size());
}

void Overlay::DrawCircleFilled(Vector2 v1, float r, Color c)
{
	int num = (int)((2.f * XM_PI) / acosf(1.f - (2.f / (r * 2.f))));
	VertexPositionColor mid { { v1.x, v1.y, 0 }, c };
	VertexPositionColor prev { { v1.x + r, v1.y, 0 }, c };
	for (int i = 1; i <= num; i++)
	{
		VertexPositionColor cur { { v1.x + r * cosf(XM_PI * ((float)i / (num / 2.f))) , v1.y + r * sinf(XM_PI * ((float)i / (num / 2.f))), 0 }, c };
		m_batch->DrawTriangle(prev, mid, cur);
		prev = cur;
	}
}

void Overlay::Tick()
{
	m_timer.Tick([&]()
		{
			Update(m_timer);
		});

	Render();
}

void Overlay::Update(DX::StepTimer const& timer)
{
	float elapsedTime = float(timer.GetElapsedSeconds());

}