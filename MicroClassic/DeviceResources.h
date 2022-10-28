#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

namespace DX
{
    class IDeviceNotify
    {
    public:
        virtual void OnDeviceLost() = 0;
        virtual void OnDeviceRestored() = 0;

    protected:
        ~IDeviceNotify() = default;
    };

    class DeviceResources
    {
    public:
        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            UINT backBufferCount = 2,
            D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0) noexcept;
        ~DeviceResources() = default;

        DeviceResources(DeviceResources&&) = default;
        DeviceResources& operator= (DeviceResources&&) = default;

        DeviceResources(DeviceResources const&) = delete;
        DeviceResources& operator= (DeviceResources const&) = delete;

        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(HWND window, int width, int height) noexcept;
        bool WindowSizeChanged(int width, int height);
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept { m_deviceNotify = deviceNotify; }
        void Present();

        // Device Accessors.
        RECT GetOutputSize() const noexcept { return m_outputSize; }

        // Direct3D Accessors.
        auto                    GetD3DDevice() const noexcept { return m_d3dDevice.Get(); }
        auto                    GetD3DDeviceContext() const noexcept { return m_d3dContext.Get(); }
        auto                    GetSwapChain() const noexcept { return m_swapChain.Get(); }
        auto                    GetDXGIFactory() const noexcept { return m_dxgiFactory.Get(); }
        HWND                    GetWindow() const noexcept { return m_window; }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const noexcept { return m_d3dFeatureLevel; }
        ID3D11Texture2D* GetRenderTarget() const noexcept { return m_renderTarget.Get(); }
        ID3D11Texture2D* GetDepthStencil() const noexcept { return m_depthStencil.Get(); }
        ID3D11RenderTargetView* GetRenderTargetView() const noexcept { return m_d3dRenderTargetView.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const noexcept { return m_d3dDepthStencilView.Get(); }
        DXGI_FORMAT             GetBackBufferFormat() const noexcept { return m_backBufferFormat; }
        DXGI_FORMAT             GetDepthBufferFormat() const noexcept { return m_depthBufferFormat; }
        D3D11_VIEWPORT          GetScreenViewport() const noexcept { return m_screenViewport; }
        UINT                    GetBackBufferCount() const noexcept { return m_backBufferCount; }

    private:
        void CreateFactory();
        void GetHardwareAdapter(IDXGIAdapter** ppAdapter);

        Microsoft::WRL::ComPtr<IDXGIFactory>               m_dxgiFactory;
        Microsoft::WRL::ComPtr<ID3D11Device>               m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext>        m_d3dContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain>             m_swapChain;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_renderTarget;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_depthStencil;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_d3dRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_d3dDepthStencilView;
        D3D11_VIEWPORT                                  m_screenViewport;

        DXGI_FORMAT                                     m_backBufferFormat;
        DXGI_FORMAT                                     m_depthBufferFormat;
        UINT                                            m_backBufferCount;
        D3D_FEATURE_LEVEL                               m_d3dMinFeatureLevel;

        HWND                                            m_window;
        D3D_FEATURE_LEVEL                               m_d3dFeatureLevel;
        RECT                                            m_outputSize;

        IDeviceNotify* m_deviceNotify;
    };
}