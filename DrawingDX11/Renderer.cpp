#include "Renderer.h"

const char shader[] = R"(
cbuffer screenProjectionBuffer : register(b0)
{
	matrix projection;
};
 
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};
 
struct VS_INPUT
{
	float4 pos : POSITION;
	float4 col : COLOR;
};
 
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
 
	output.pos = mul(projection, float4(input.pos.xy, 0.f, 1.f));
	output.col = input.col;
 
	return output;
}
 
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return input.col;
}
)";

Renderer::Renderer()
{
    m_cFrameCounter = std::chrono::steady_clock::now();
}

Renderer::~Renderer()
{
    SAFE_RELEASE(m_pRenderTargetView);
    SAFE_RELEASE(m_pSwapChain);
    SAFE_RELEASE(m_pd3dDeviceContext);
    SAFE_RELEASE(m_pd3dDevice);

    safeRelease(vertexShader);
    safeRelease(pixelShader);
    safeRelease(vertexBuffer);
    safeRelease(screenProjectionBuffer);
    safeRelease(inputLayout);
    safeRelease(blendState);
}

bool Renderer::Initialize(HWND hWindow, int32 iWidth, int32 iHeight)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWindow;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &m_pSwapChain,
        &m_pd3dDevice,
        &featureLevel,
        &m_pd3dDeviceContext
    );

    D3D11_VIEWPORT m_Viewport{};
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = static_cast<float>(iWidth);
    m_Viewport.Height = static_cast<float>(iHeight);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_pd3dDeviceContext->RSSetViewports(1, &m_Viewport);

    if (FAILED(hr))
        return false;

    if (!CreateRenderTarget())
        return false;
    ////////////////////////////////////////////////////////////////////////////////////


    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16 ,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    maxVertices = (1024 * 4 * 3);

    // renderList = RenderList(maxVertices); //std::make_unique<RenderList>(maxVertices);

    throwIfFailed(D3DCompile(shader, std::size(shader), nullptr, nullptr, nullptr, "VS", "vs_4_0", 0, 0, &vsBlob, nullptr));
    throwIfFailed(D3DCompile(shader, std::size(shader), nullptr, nullptr, nullptr, "PS", "ps_4_0", 0, 0, &psBlob, nullptr));

    throwIfFailed(m_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader));
    throwIfFailed(m_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader));

    throwIfFailed(m_pd3dDevice->CreateInputLayout(layout, static_cast<UINT>(std::size(layout)), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout));

    safeRelease(vsBlob);
    safeRelease(psBlob);


    D3D11_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget->BlendEnable = TRUE;
    blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    throwIfFailed(m_pd3dDevice->CreateBlendState(&blendDesc, &blendState));

    D3D11_BUFFER_DESC bufferDesc{};

    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(Vertex) * static_cast<UINT>(maxVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;

    throwIfFailed(m_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &vertexBuffer));

    bufferDesc = {};

    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(XMMATRIX);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;

    throwIfFailed(m_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &screenProjectionBuffer));

    D3D11_VIEWPORT viewport{};
    UINT numViewports = 1;

    m_pd3dDeviceContext->RSGetViewports(&numViewports, &viewport);

    printf("%f %f %f %f %f %f\n", viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY,
        viewport.MinDepth, viewport.MaxDepth);

    projection = XMMatrixOrthographicOffCenterLH(viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY,
        viewport.MinDepth, viewport.MaxDepth);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    throwIfFailed(m_pd3dDeviceContext->Map(screenProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    {
        std::memcpy(mappedResource.pData, &projection, sizeof(XMMATRIX));
    }
    m_pd3dDeviceContext->Unmap(screenProjectionBuffer, 0);

    return true;
}

void Renderer::Resize(int32 w, int32 h)
{
    if (m_pd3dDevice != NULL)
    {
        SAFE_RELEASE(m_pRenderTargetView);
        m_pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }
}

void Renderer::Present()
{
    float clearColor[4] = { 1.0f,0.0f,1.0f,0.0f };
    float blend[4] = { 0 };

    const float clear_color_with_alpha[4] = { 1, 1, 1, 0 };
    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
    m_pd3dDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clear_color_with_alpha);
    m_pd3dDeviceContext->OMSetBlendState(blendState, blend, 0xffffffff);

    begin();

    XMFLOAT4 rect{ 200.f, 200.f, 200.f, 200.f };
    drawFilledRect(rect, {0,1,1,1});

    draw();
    end();

    m_pd3dDeviceContext->OMSetBlendState(blendState, blend, 0xffffffff);
    m_pSwapChain->Present(1, 0);
}

bool Renderer::CreateRenderTarget()
{
    HRESULT hr = 0;
    ID3D11Texture2D* pBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    if (FAILED(hr))
        return false;

    if (pBackBuffer)
    {
        hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);

        if (FAILED(hr))
        {
            SAFE_RELEASE(pBackBuffer);
            return false;
        }
    }

    pBackBuffer->Release();

    return true;
}

void Renderer::begin()
{
    m_pd3dDeviceContext->VSSetShader(vertexShader, nullptr, 0);
    m_pd3dDeviceContext->PSSetShader(pixelShader, nullptr, 0);

    m_pd3dDeviceContext->OMSetBlendState(blendState, nullptr, 0xffffffff);

    m_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &screenProjectionBuffer);

    m_pd3dDeviceContext->IASetInputLayout(inputLayout);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_pd3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
}

void Renderer::end()
{
    renderList.clear();
}

void Renderer::draw(RenderList& renderList)
{
    if (std::size(renderList.vertices) > 0)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        throwIfFailed(m_pd3dDeviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        {
            std::memcpy(mappedResource.pData, renderList.vertices.data(), sizeof(Vertex) * std::size(renderList.vertices));
        }
        m_pd3dDeviceContext->Unmap(vertexBuffer, 0);
    }

    std::size_t pos = 0;

    for (const auto& batch : renderList.batches)
    {
        m_pd3dDeviceContext->IASetPrimitiveTopology(batch.topology);
        m_pd3dDeviceContext->Draw(static_cast<UINT>(batch.count), static_cast<UINT>(pos));

        pos += batch.count;
    }
}

void Renderer::draw()
{
    draw(renderList);
}

void Renderer::addVertex(RenderList& renderList, Vertex& vertex, D3D11_PRIMITIVE_TOPOLOGY topology)
{
    assert(topology != D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
        && "addVertex >Use addVertices to draw line/triangle strips!");
    assert(topology != D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
        && "addVertex >Use addVertices to draw line/triangle strips!");
    assert(topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
        && "addVertex >Use addVertices to draw line/triangle strips!");
    assert(topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ
        && "addVertex >Use addVertices to draw line/triangle strips!");


    if (std::size(renderList.vertices) >= maxVertices)
        (&this->renderList == &renderList) ? draw(renderList) : throw std::exception(
            "Renderer::addVertex - Vertex buffer exhausted! Increase the size of the vertex buffer or add a custom implementation.");

    if (std::empty(renderList.batches) || renderList.batches.back().topology != topology)
        renderList.batches.emplace_back(0, topology);

    renderList.batches.back().count += 1;
    renderList.vertices.push_back(vertex);
}

void Renderer::addVertex(Vertex& vertex, D3D11_PRIMITIVE_TOPOLOGY topology)
{
    addVertex(renderList, vertex, topology);
}

void Renderer::drawPixel(RenderList& renderList, const Vec2& pos, const Color& color)
{
    drawFilledRect(renderList, XMFLOAT4{ pos.x, pos.y, 1.f, 1.f }, color);
}

void Renderer::drawPixel(const Vec2& pos, const Color& color)
{
    drawFilledRect(renderList, XMFLOAT4{ pos.x, pos.y, 1.f, 1.f }, color);
}

void Renderer::drawLine(RenderList& renderList, const Vec2& from, const Vec2& to, const Color& color)
{
    Vertex v[]
    {
        { from.x,	from.y,	0.f, color },
        { to.x,		to.y,	0.f, color }
    };

    addVertices(renderList, v, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void Renderer::drawLine(const Vec2& from, const Vec2& to, const Color& color)
{
    drawLine(renderList, from, to, color);
}

void Renderer::drawFilledRect(RenderList& renderList, const Vec4& rect, const Color& color)
{
    Vertex v[]
    {
        { rect.x,			rect.y,				0.f, color },
        { rect.x + rect.z,	rect.y,				0.f, color },
        { rect.x,			rect.y + rect.w,	0.f, color },

        { rect.x + rect.z,	rect.y,				0.f, color },
        { rect.x + rect.z,	rect.y + rect.w,	0.f, color },
        { rect.x,			rect.y + rect.w,	0.f, color }
    };

    addVertices(renderList, v, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::drawFilledRect(const Vec4& rect, const Color& color)
{
    drawFilledRect(renderList, rect, color);
}

void Renderer::drawRect(RenderList& renderList, const Vec4& rect, float strokeWidth, const Color& color)
{
    XMFLOAT4 tmp = rect;
    tmp.z = strokeWidth;
    drawFilledRect(renderList, tmp, color);
    tmp.x = rect.x + rect.z - strokeWidth;
    drawFilledRect(renderList, tmp, color);
    tmp.z = rect.z;
    tmp.x = rect.x;
    tmp.w = strokeWidth;
    drawFilledRect(renderList, tmp, color);
    tmp.y = rect.y + rect.w;
    drawFilledRect(renderList, tmp, color);
}

void Renderer::drawRect(const Vec4& rect, float strokeWidth, const Color& color)
{
    drawRect(renderList, rect, strokeWidth, color);
}

void Renderer::drawOutlinedRect(RenderList& renderList, const Vec4& rect, float strokeWidth, const Color& strokeColor, const Color& fillColor)
{
    drawFilledRect(renderList, rect, fillColor);
    drawRect(renderList, rect, strokeWidth, strokeColor);
}

void Renderer::drawOutlinedRect(const Vec4& rect, float strokeWidth, const Color& strokeColor, const Color& fillColor)
{
    drawOutlinedRect(renderList, rect, strokeWidth, strokeColor, fillColor);
}

void Renderer::drawCircle(RenderList& renderList, const Vec2& pos, float radius, const Color& color)
{
    const int segments = 24;

    Vertex v[segments + 1];

    for (int i = 0; i <= segments; i++)
    {
        float theta = 2.f * XM_PI * static_cast<float>(i) / static_cast<float>(segments);

        v[i] = Vertex{
            pos.x + radius * std::cos(theta),
            pos.y + radius * std::sin(theta),
            0.f, color
        };
    }

    addVertices(renderList, v, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void Renderer::drawCircle(const Vec2& pos, float radius, const Color& color)
{
    drawCircle(renderList, pos, radius, color);
}