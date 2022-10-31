#pragma once

#include "Process.h"
#include "Define.h"
#include "DeviceResources.h"
#include "MSAA.h"
#include "PrimitiveBatch.h"
#include "CommonStates.h"
#include "Effects.h"
#include "VertexTypes.h"
#include "DirectXHelpers.h"
#include "SimpleMath.h"
#include "Helpers.h"
#include "StepTimer.h"

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
	void Tick();
	void DrawLine(Vector2 v1, Vector2 v2, Color c = { 1,1,1,1 });
	void DrawTriangle(Vector2 v1, Vector2 v2, Vector3 v3, Color c = { 1,1,1,1 });
	void DrawTriangleFilled(Vector2 v1, Vector2 v2, Vector3 v3, Color c = { 1,1,1,1 });
	void DrawRect(Vector4 rect, float strokeWidth, Color c = { 1,1,1,1 });
	void DrawRectFilled(Vector4 rect, Color c = { 1,1,1,1 });
	void DrawCircle(Vector2 v1, float r, Color c = {1,1,1,1});
	void DrawCircleFilled(Vector2 v1, float r, Color c = { 1,1,1,1 });
private:
	void Update(DX::StepTimer const& timer);
	void Render();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void Clear();

	HWND m_hWindow;
	HWND m_hTargetWindow = NULL;
	WNDCLASSEX m_WndClass{ 0 };
	std::unique_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<DX::MSAA> m_MSAA;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	int m_iWidth = 800;
	int m_iHeight = 600;
	DX::StepTimer m_timer;
};