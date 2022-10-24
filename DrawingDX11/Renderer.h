#pragma once

#include "Defines.h"

#include <chrono>
#include <d3d11.h>
#include <directxmath.h>
#include <directxpackedvector.h>
#include <d3dcompiler.h>


#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D11.lib")


using namespace DirectX;
using namespace DirectX::PackedVector;

using Vec4 = XMFLOAT4;
using Vec3 = XMFLOAT3;
using Vec2 = XMFLOAT2;

using Color = XMVECTORF32;

inline void throwIfFailed(HRESULT hr);

struct Vertex
{
	Vertex() = default;
	Vertex(float x, float y, float z, Color col)
		: pos(x, y, z), col(col)
	{}
	Vec3 pos;
	Color col;
};

struct Batch
{
	Batch(uint64 count, D3D11_PRIMITIVE_TOPOLOGY topology)
		: count(count), topology(topology)
	{}
	uint64 count;
	D3D11_PRIMITIVE_TOPOLOGY topology;
};

class RenderList
{
	friend class Renderer;
public:
	RenderList()
	{
		vertices.reserve(1024);
	}

	~RenderList()
	{
		clear();
	}

	void clear()
	{
		vertices.clear();
		batches.clear();
	}

protected:
	std::vector<Vertex>	vertices;
	std::vector<Batch>	batches;
};

class Renderer
{
public:
	Renderer();
	~Renderer();
	bool Initialize(HWND hWindow,int32 iWidth, int32 iHeight);
	void Resize(int32 w, int32 h);
	void BeginScense();
	void Present();
	float GetFps() { return m_fFps; }

	void begin();
	void end();
	void draw(RenderList& renderList);
	void draw();

	void addVertex(RenderList& renderList, Vertex& vertex, D3D11_PRIMITIVE_TOPOLOGY topology);
	void addVertex(Vertex& vertex, D3D11_PRIMITIVE_TOPOLOGY topology);

	template <uint64 N>
	void addVertices(RenderList& renderList, Vertex(&vertexArr)[N], D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		if (std::size(renderList.vertices) + N >= maxVertices)
			(&this->renderList == &renderList) ? draw(renderList) : throw std::exception(
				"Renderer::addVertex - Vertex buffer exhausted! Increase the size of the vertex buffer or add a custom implementation.");

		if (std::empty(renderList.batches) || renderList.batches.back().topology != topology)
			renderList.batches.emplace_back(0, topology);

		renderList.batches.back().count += N;
		renderList.vertices.resize(std::size(renderList.vertices) + N);

		std::memcpy(&renderList.vertices[std::size(renderList.vertices) - N], &vertexArr[0], N * sizeof(Vertex));

		switch (topology)
		{
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
		{
			Vertex seperator{};
			addVertex(seperator, D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
			break;
		}
		default:
			break;
		}
	}

	template <std::size_t N>
	void addVertices(Vertex(&vertexArr)[N], D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		addVertices(renderList, vertexArr, topology);
	}

	void drawPixel(const Vec2& pos, const Color& color);
	void drawPixel(RenderList& renderList, const Vec2& pos, const Color& color);
	void drawLine(const Vec2& from, const Vec2& to, const Color& color);
	void drawLine(RenderList& renderList, const Vec2& from, const Vec2& to, const Color& color);
	void drawFilledRect(const Vec4& rect, const Color& color);
	void drawFilledRect(RenderList& renderList, const Vec4& rect, const Color& color);
	void drawRect(const Vec4& rect, float strokeWidth, const Color& color);
	void drawRect(RenderList& renderList, const Vec4& rect, float strokeWidth, const Color& color);
	void drawOutlinedRect(const Vec4& rect, float strokeWidth, const Color& strokeColor, const Color& fillColor);
	void drawOutlinedRect(RenderList& renderList, const Vec4& rect, float strokeWidth, const Color& strokeColor, const Color& fillColor);
	void drawCircle(const Vec2& pos, float radius, const Color& color);
	void drawCircle(RenderList& renderList, const Vec2& pos, float radius, const Color& color);
private:
	bool CreateRenderTarget();

	ID3D11Device* m_pd3dDevice = NULL;
	ID3D11DeviceContext* m_pd3dDeviceContext = NULL;
	IDXGISwapChain* m_pSwapChain = NULL;
	ID3D11RenderTargetView* m_pRenderTargetView = NULL;

	ID3D11InputLayout* inputLayout = NULL;
	ID3D11BlendState* blendState = NULL;
	ID3D11VertexShader* vertexShader = NULL;
	ID3D11PixelShader* pixelShader = NULL;
	ID3D11Buffer* vertexBuffer = NULL;
	ID3D11Buffer* screenProjectionBuffer = NULL;


	RenderList renderList;
	XMMATRIX projection;




	float m_fFps = 0;
	uint32 m_iFrameCounter = 0;
	std::chrono::time_point<std::chrono::steady_clock> m_cFrameCounter;
};

inline void throwIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception("Crucial Direct3D 11 operation failed! ");
	}
}