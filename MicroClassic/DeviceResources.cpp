#include "DeviceResources.h"
#include "Helpers.h"

using namespace DirectX;
using namespace DX;
using Microsoft::WRL::ComPtr;

DeviceResources::DeviceResources(
	DXGI_FORMAT backBufferFormat,
	DXGI_FORMAT depthBufferFormat,
	UINT backBufferCount,
	D3D_FEATURE_LEVEL minFeatureLevel) noexcept :
	m_screenViewport{},
	m_backBufferFormat(backBufferFormat),
	m_depthBufferFormat(depthBufferFormat),
	m_backBufferCount(backBufferCount),
	m_d3dMinFeatureLevel(minFeatureLevel),
	m_window(nullptr),
	m_d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	m_outputSize{ 0, 0, 1, 1 },
	m_deviceNotify(nullptr)
{
}

void DeviceResources::CreateDeviceResources()
{
	UINT creationFlags = 0;

	CreateFactory();

	static const D3D_FEATURE_LEVEL s_featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	UINT featLevelCount = 0;
	for (; featLevelCount < static_cast<UINT>(std::size(s_featureLevels)); ++featLevelCount)
	{
		if (s_featureLevels[featLevelCount] < m_d3dMinFeatureLevel)
			break;
	}

	if (!featLevelCount)
	{
		throw std::out_of_range("minFeatureLevel too high");
	}

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = E_FAIL;

	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		s_featureLevels,
		featLevelCount,
		D3D11_SDK_VERSION,
		device.GetAddressOf(),  // Returns the Direct3D device created.
		&m_d3dFeatureLevel,     // Returns feature level of device created.
		context.GetAddressOf()  // Returns the device immediate context.
	);

	ThrowIfFailed(hr);
	ThrowIfFailed(device.As(&m_d3dDevice));
	ThrowIfFailed(context.As(&m_d3dContext));
}

void DeviceResources::CreateWindowSizeDependentResources()
{
	if (!m_window)
	{
		throw std::logic_error("Call SetWindow with a valid Win32 window handle");
	}

	m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_d3dRenderTargetView.Reset();
	m_d3dDepthStencilView.Reset();
	m_renderTarget.Reset();
	m_depthStencil.Reset();
	m_d3dContext->Flush();

	const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_outputSize.right - m_outputSize.left), 1u);
	const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_outputSize.bottom - m_outputSize.top), 1u);
	const DXGI_FORMAT backBufferFormat = m_backBufferFormat;

	if (m_swapChain)
	{
		HRESULT hr = m_swapChain->ResizeBuffers(
			m_backBufferCount,
			backBufferWidth,
			backBufferHeight,
			backBufferFormat,
			0
		);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
			return;
		}
		else
		{
			ThrowIfFailed(hr);
		}
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = backBufferWidth;
		sd.BufferDesc.Height = backBufferHeight;
		sd.BufferDesc.Format = backBufferFormat;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_window;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
			m_d3dDevice.Get(),
			&sd,
			m_swapChain.ReleaseAndGetAddressOf()
		));

		// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
		ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
	}

	// Create a render target view of the swap chain back buffer.
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_renderTarget.ReleaseAndGetAddressOf())));

	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, m_backBufferFormat);
	ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(
		m_renderTarget.Get(),
		NULL,
		m_d3dRenderTargetView.ReleaseAndGetAddressOf()
	));

	if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		// Create a depth stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			m_depthBufferFormat,
			backBufferWidth,
			backBufferHeight,
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);

		ThrowIfFailed(m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			m_depthStencil.ReleaseAndGetAddressOf()
		));

		ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(
			m_depthStencil.Get(),
			nullptr,
			m_d3dDepthStencilView.ReleaseAndGetAddressOf()
		));
	}

	m_screenViewport = { 0.0f, 0.0f, static_cast<float>(backBufferWidth), static_cast<float>(backBufferHeight), 0.f, 1.f };
}

void DeviceResources::SetWindow(HWND window, int width, int height) noexcept
{
	m_window = window;

	m_outputSize.left = m_outputSize.top = 0;
	m_outputSize.right = width;
	m_outputSize.bottom = height;
}

bool DeviceResources::WindowSizeChanged(int width, int height)
{
	RECT newRc;
	newRc.left = newRc.top = 0;
	newRc.right = width;
	newRc.bottom = height;
	if (m_outputSize.right == width && m_outputSize.bottom == height)
	{
		return false;
	}

	m_outputSize = newRc;
	CreateWindowSizeDependentResources();
	return true;
}

void DeviceResources::HandleDeviceLost()
{
	if (m_deviceNotify)
	{
		m_deviceNotify->OnDeviceLost();
	}

	m_d3dDepthStencilView.Reset();
	m_d3dRenderTargetView.Reset();
	m_renderTarget.Reset();
	m_depthStencil.Reset();
	m_swapChain.Reset();
	m_d3dContext.Reset();

	m_d3dDevice.Reset();
	m_dxgiFactory.Reset();

	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	if (m_deviceNotify)
	{
		m_deviceNotify->OnDeviceRestored();
	}
}

void DeviceResources::Present()
{
	HRESULT hr = E_FAIL;

	hr = m_swapChain->Present(3, 0);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	
  //  m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());
	
	if (m_d3dDepthStencilView)
	{
		// Discard the contents of the depth stencil.
		// 
	   // m_d3dContext->DiscardResource(m_d3dDepthStencilView.Get());
	}

	// If the device was removed either by a disconnection or a driver upgrade, we
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HandleDeviceLost();
	}
	else
	{
		ThrowIfFailed(hr);
	}
}

void DeviceResources::CreateFactory()
{
	ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
}