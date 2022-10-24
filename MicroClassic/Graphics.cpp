#include "Graphics.h"

#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

Graphics::Graphics()
{

}

Graphics::~Graphics()
{

}


bool Graphics::Init(HWND hWnd)
{

    return true;
}

void Graphics::BeginScense()
{

}

void Graphics::EndScense()
{
    
}

void Graphics::OnWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_SIZE)
    {
        if (wParam != SIZE_MINIMIZED)
        {
            
        }
    }
}