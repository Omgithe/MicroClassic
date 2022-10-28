#pragma once

#include "Process.h"
#include "Define.h"
#include "DeviceResources.h"
#include "PrimitiveBatch.h"
#include "CommonStates.h"
#include "Effects.h"
#include "VertexTypes.h"
#include "DirectXHelpers.h"
#include "SimpleMath.h"
#include "Helpers.h"

class Overlay
	: public DX::IDeviceNotify
{
public:
	Overlay();
	~Overlay();
	void Attach(Process* pProcess);
	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool ProcessMessages();
	int GetWidth() { return m_iWidth; }
	int GetHeight() { return m_iHeight; }
	HWND GetWindowHandle() { return m_hWindow; }
	void OnDeviceLost() override;
	void OnDeviceRestored() override;
	void Render();
private:
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void Clear();

	HWND m_hWindow;
	HWND m_hTargetWindow = NULL;
	WNDCLASSEX m_WndClass{ 0 };
	std::unique_ptr<DX::DeviceResources> m_deviceResources;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	int m_iWidth = 800;
	int m_iHeight = 600;
};