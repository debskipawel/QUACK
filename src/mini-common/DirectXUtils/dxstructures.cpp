#include "dxstructures.h"
using namespace mini;
using namespace utils;

SwapChainDescription::SwapChainDescription(HWND hWnd, UINT width, UINT height)
{
	ZeroMemory(this, sizeof(SwapChainDescription));
	BufferCount = 1;
	BufferDesc.Width = width;
	BufferDesc.Height = height;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	BufferDesc.RefreshRate.Numerator = 144;
	BufferDesc.RefreshRate.Denominator = 1;
	BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	OutputWindow = hWnd;
	Windowed = true;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;
}

Texture2DDescription::Texture2DDescription(UINT width, UINT height, DXGI_FORMAT format, UINT mipLevels)
{
	ZeroMemory(this, sizeof(Texture2DDescription));
	Width = width;
	Height = height;
	MipLevels = mipLevels;
	ArraySize = 1;
	Format = format;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;
	Usage = D3D11_USAGE_DEFAULT;
	BindFlags = D3D11_BIND_SHADER_RESOURCE;
	CPUAccessFlags = 0;
	MiscFlags = 0;
}

Texture2DDescription Texture2DDescription::DepthTextureDescription(UINT width, UINT height)
{
	Texture2DDescription desc(width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, 1);
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	return desc;
}

Texture3DDescription::Texture3DDescription(UINT width, UINT height, UINT depth, DXGI_FORMAT format, UINT mipLevels)
{
	ZeroMemory(this, sizeof(Texture3DDescription));
	Width = width;
	Height = height;
	Depth = depth;
	MipLevels = mipLevels;
	Format = format;
	Usage = D3D11_USAGE_DEFAULT;
	BindFlags = D3D11_BIND_SHADER_RESOURCE;
	CPUAccessFlags = 0;
	MiscFlags = 0;
}

DepthViewDescription::DepthViewDescription()
{
	ZeroMemory(this, sizeof(DepthViewDescription));
	Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	Flags = 0;
	Texture2D.MipSlice = 0;
}

ViewportDescription::ViewportDescription(UINT width, UINT height)
{
	Width = static_cast<float>(width);
	Height = static_cast<float>(height);
	MinDepth = 0.0f;
	MaxDepth = 1.0f;
	TopLeftX = 0.0f;
	TopLeftY = 0.0f;
}

BufferDescription::BufferDescription(UINT byteWidth, D3D11_BIND_FLAG flags, D3D11_USAGE usage)
{
	ZeroMemory(this, sizeof(BufferDescription));
	Usage = usage;
	BindFlags = flags;
	ByteWidth = byteWidth;
	if ((usage & D3D11_USAGE_DYNAMIC) != 0 || (usage & D3D11_USAGE_STAGING) != 0)
		CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
}

BufferDescription BufferDescription::ConstantBufferDescription(UINT byteWidth)
{
	if (byteWidth & 0xfU)
	{
		byteWidth &= ~0xfU;
		byteWidth += 0x10U;
	}
	return{ byteWidth, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC };
}


RasterizerDescription::RasterizerDescription(bool counterClockwise)
{
	FillMode = D3D11_FILL_SOLID; //Determines the solid fill mode (as opposed to wireframe)
	CullMode = D3D11_CULL_BACK; //Indicates that back facing triangles are not drawn
	FrontCounterClockwise = counterClockwise; //Indicates if vertices of a front facing triangle are counter-clockwise on the render target
	DepthBias = 0;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 0.0f;
	DepthClipEnable = true;
	ScissorEnable = false;
	MultisampleEnable = false;
	AntialiasedLineEnable = false;
}

SamplerDescription::SamplerDescription()
{
	Filter = D3D11_FILTER_ANISOTROPIC;
	AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	MinLOD = -D3D11_FLOAT32_MAX;
	MaxLOD = D3D11_FLOAT32_MAX;
	MipLODBias = 0.0f;
	MaxAnisotropy = 16;
	ComparisonFunc = D3D11_COMPARISON_NEVER;
	BorderColor[0] = 0.0f;
	BorderColor[1] = 0.0f;
	BorderColor[2] = 0.0f;
	BorderColor[3] = 0.0f;
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

BlendDescription BlendDescription::AlphaBlendDescription()
{
	BlendDescription desc;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	return desc;
}

BlendDescription BlendDescription::AdditiveBlendDescription()
{
	BlendDescription desc;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
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

DepthStencilDescription DepthStencilDescription::StencilWriteDescription()
{
	DepthStencilDescription desc;
	desc.StencilEnable = true; //Enable stencil operations
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //Disable writing to depth buffer
	desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; //Back faces should never pass stencil test
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; //Front faces should always pass stencil test
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; //when pixel passes depth and stencil test write to stencil buffer
	return desc;
}

DepthStencilDescription DepthStencilDescription::StencilTestDescription()
{
	DepthStencilDescription desc;
	desc.StencilEnable = true; //Enable stencil operation
	desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; //Back faces should never pass stencil test
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; //Front faces pass stencil test when drawn on pixels with specific stencil buffer value
	return desc;
}

ShaderResourceViewDescription::ShaderResourceViewDescription()
{
	ZeroMemory(this, sizeof(ShaderResourceViewDescription));
}

ShaderResourceViewDescription ShaderResourceViewDescription::Texture2DViewDescription()
{
	ShaderResourceViewDescription result;
	result.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	result.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	result.Texture2D.MipLevels = -1;
	result.Texture2D.MostDetailedMip = 0;
	return result;
}

SubresourceData::SubresourceData()
{
	ZeroMemory(this, sizeof(SubresourceData));
}