#include "dxStructures.h"

using namespace mini;

SwapChainDescription::SwapChainDescription(HWND wndHwnd, SIZE wndSize)
	: DXGI_SWAP_CHAIN_DESC{}
{
	BufferDesc.Width = wndSize.cx;
	BufferDesc.Height = wndSize.cy;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//BufferDesc.RefreshRate.Numerator = 0;
	BufferDesc.RefreshRate.Denominator = 1;
	//BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED /* 0 */;
	BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED /* 0 */;
	//SampleDesc.Quality = 0;
	SampleDesc.Count = 1;
	BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	BufferCount = 1;
	OutputWindow = wndHwnd;
	Windowed = true;
	//SwapEffect = DXGI_SWAP_EFFECT_DISCARD /* 0 */;
	//Flags = 0;
}

Viewport::Viewport(SIZE size)
	: D3D11_VIEWPORT{}
{
	//this->TopLeftX = 0.0f;
	//this->TopLeftY = 0.0f;
	this->Width = static_cast<FLOAT>(size.cx);
	this->Height = static_cast<FLOAT>(size.cy);
	//this->MinDepth = 0.0f;
	this->MaxDepth = 1.0f;
}

Texture2DDescription::Texture2DDescription(UINT width, UINT height)
	: D3D11_TEXTURE2D_DESC{}
{
	Width = width;
	Height = height;
	//MipLevels = 0;
	ArraySize = 1;
	Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SampleDesc.Count = 1;
	//SampleDesc.Quality = 0;
	//Usage = D3D11_USAGE_DEFAULT;
	BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//CPUAccessFlags = 0;
	//MiscFlags = 0;
}

Texture2DDescription Texture2DDescription::DepthStencilDescription(UINT width, UINT height)
{
	Texture2DDescription desc{ width, height };
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	return desc;
}

BufferDescription::BufferDescription(UINT bindFlags, size_t byteWidth)
	: D3D11_BUFFER_DESC{}
{
	BindFlags = bindFlags;
	ByteWidth = byteWidth;
	//Usage = D3D11_USAGE_DEFAULT;
	//CPUAccessFlags = 0;
	//MiscFlags = 0;
	//StructureByteStride = 0;
}

BufferDescription BufferDescription::ConstantBufferDescription(size_t byteWidth)
{
	BufferDescription desc{ D3D11_BIND_CONSTANT_BUFFER, byteWidth };
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return desc;
}

BlendDescription::BlendDescription()
{
	ZeroMemory(this, sizeof(BlendDescription));
	RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

BlendDescription mini::BlendDescription::AlphaBlendDescription()
{
	BlendDescription desc;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	return desc;
}

DepthStencilDescription::DepthStencilDescription()
{
	DepthEnable = true;
	DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthFunc = D3D11_COMPARISON_LESS;
	StencilEnable = false;
	StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	FrontFace.StencilFunc = BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	FrontFace.StencilDepthFailOp = BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	FrontFace.StencilPassOp = BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	FrontFace.StencilFailOp = BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
}

RasterizerDescription::RasterizerDescription()
{
	FillMode = D3D11_FILL_SOLID; //Determines the solid fill mode (as opposed to wireframe)
	CullMode = D3D11_CULL_BACK; //Indicates that back facing triangles are not drawn
	FrontCounterClockwise = false; //Indicates if vertices of a front facing triangle are counter-clockwise on the render target
	DepthBias = 0;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 0.0f;
	DepthClipEnable = true;
	ScissorEnable = false;
	MultisampleEnable = false;
	AntialiasedLineEnable = false;
}

//********************* NEW *********************

mini::SamplerDescription::SamplerDescription()
{
	Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	MinLOD = -D3D11_FLOAT32_MAX;
	MaxLOD = D3D11_FLOAT32_MAX;
	MipLODBias = 0.0f;
	MaxAnisotropy = 1;
	ComparisonFunc = D3D11_COMPARISON_NEVER;
	BorderColor[0] = 1.0f;
	BorderColor[1] = 1.0f;
	BorderColor[2] = 1.0f;
	BorderColor[3] = 1.0f;
}

mini::ShaderResourceViewDescription::ShaderResourceViewDescription()
{
	ZeroMemory(this, sizeof(ShaderResourceViewDescription));
}
