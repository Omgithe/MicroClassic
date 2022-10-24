#pragma once

#include "Define.h"
#include "Vector.h"

#include <dxgiformat.h> // DXGI_FORMAT
#include <d3d11.h>
#pragma comment(lib, "D3D11.lib")

struct ImDrawVert
{
    Vec2  pos;
    Vec2  uv;
    uint32   col;
};

class Graphics
{
public:
	Graphics();
	~Graphics();

    bool Init(HWND);

    void BeginScense();
    void EndScense();
    void OnWndProc(UINT, WPARAM, LPARAM);
private:
    
    Vec2 m_vDisplaySize{0, 0};
};