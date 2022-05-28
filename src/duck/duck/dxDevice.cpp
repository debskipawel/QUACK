#include "dxDevice.h"
#include "exceptions.h"
#include <fstream>
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

using namespace mini;
using namespace std;

DxDevice::DxDevice(const Window& window)
{
	SwapChainDescription desc{ window.getHandle(), window.getClientSize() };
	ID3D11Device *d = nullptr;
	ID3D11DeviceContext *dc = nullptr;
	IDXGISwapChain *sc = nullptr;
	unsigned int creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	auto hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, creationFlags, nullptr, 0,
		D3D11_SDK_VERSION, &desc, &sc, &d, nullptr, &dc);
	m_device.reset(d);
	m_context.reset(dc);
	m_swapChain.reset(sc);
	if (FAILED(hr))
		THROW_DX(hr);
}

dx_ptr<ID3D11RenderTargetView> DxDevice::CreateRenderTargetView(const dx_ptr<ID3D11Texture2D>& texture) const
{
	ID3D11RenderTargetView *temp = nullptr;
	auto hr = m_device->CreateRenderTargetView(texture.get(), nullptr, &temp);
	dx_ptr<ID3D11RenderTargetView> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

vector<BYTE> DxDevice::LoadByteCode(const wstring& filename)
{
	ifstream sIn(filename, ios::in | ios::binary);
	if (!sIn)
		THROW(L"Unable to open " + filename);
	sIn.seekg(0, ios::end);
	auto byteCodeLength = sIn.tellg();
	sIn.seekg(0, ios::beg);
	vector<BYTE> byteCode(static_cast<unsigned int>(byteCodeLength));
	if (!sIn.read(reinterpret_cast<char*>(byteCode.data()), byteCodeLength))
		THROW(L"Error reading" + filename);
	sIn.close();
	return byteCode;
}

dx_ptr<ID3D11Texture2D> DxDevice::CreateTexture(const D3D11_TEXTURE2D_DESC& desc) const
{
	ID3D11Texture2D *temp = nullptr;
	auto hr = m_device->CreateTexture2D(&desc, nullptr, &temp);
	dx_ptr<ID3D11Texture2D> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

dx_ptr<ID3D11DepthStencilView> DxDevice::CreateDepthStencilView(const dx_ptr<ID3D11Texture2D>& texture) const
{
	ID3D11DepthStencilView* temp = nullptr;
	auto hr = m_device->CreateDepthStencilView(texture.get(), nullptr, &temp);
	dx_ptr<ID3D11DepthStencilView> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

dx_ptr<ID3D11Buffer> DxDevice::CreateBuffer(const void* data, const D3D11_BUFFER_DESC& desc) const
{
	D3D11_SUBRESOURCE_DATA sdata{};
	sdata.pSysMem = data;

	ID3D11Buffer* temp = nullptr;
	auto hr = m_device->CreateBuffer(&desc, data ? &sdata : nullptr, &temp);
	dx_ptr<ID3D11Buffer> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

dx_ptr<ID3D11VertexShader> DxDevice::CreateVertexShader(vector<BYTE> vsCode) const
{
	ID3D11VertexShader *temp = nullptr;
	auto hr = m_device->CreateVertexShader(reinterpret_cast<const void*>(vsCode.data()),
		vsCode.size(), nullptr, &temp);
	dx_ptr<ID3D11VertexShader> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

dx_ptr<ID3D11GeometryShader> mini::DxDevice::CreateGeometryShader(std::vector<BYTE> gsCode) const
{
	ID3D11GeometryShader* gs = nullptr;
	auto hr = m_device->CreateGeometryShader(gsCode.data(), gsCode.size(), nullptr, &gs);
	dx_ptr<ID3D11GeometryShader> geometryShader(gs);
	if (FAILED(hr))
		THROW_DX(hr);
	return geometryShader;
}

dx_ptr<ID3D11PixelShader> DxDevice::CreatePixelShader(vector<BYTE> psCode) const
{
	ID3D11PixelShader *temp = nullptr;
	auto hr = m_device->CreatePixelShader(reinterpret_cast<const void*>(psCode.data()),
		psCode.size(), nullptr, &temp);
	dx_ptr<ID3D11PixelShader> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

dx_ptr<ID3D11InputLayout>
DxDevice::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elements, unsigned int count, const vector<BYTE>& vsCode) const
{
	ID3D11InputLayout* temp = nullptr;
	auto hr = m_device->CreateInputLayout(elements, count, reinterpret_cast<const void*>(vsCode.data()), vsCode.size(), &temp);
	dx_ptr<ID3D11InputLayout> result{ temp };
	if (FAILED(hr))
		THROW_DX(hr);
	return result;
}

dx_ptr<ID3D11DepthStencilView> DxDevice::CreateDepthStencilView(SIZE size) const
{
	auto desc = Texture2DDescription::DepthStencilDescription(size.cx, size.cy);
	dx_ptr<ID3D11Texture2D> texture = CreateTexture(desc);
	return CreateDepthStencilView(texture);
}

dx_ptr<ID3D11BlendState> mini::DxDevice::CreateBlendState(const BlendDescription& desc) const
{
	ID3D11BlendState* s = nullptr;
	auto hr = m_device->CreateBlendState(&desc, &s);
	dx_ptr<ID3D11BlendState> state(s);
	if (FAILED(hr))
		THROW_DX(hr);
	return state;
}

dx_ptr<ID3D11DepthStencilState> mini::DxDevice::CreateDepthStencilState(const DepthStencilDescription& desc) const
{
	ID3D11DepthStencilState* s = nullptr;
	auto hr = m_device->CreateDepthStencilState(&desc, &s);
	dx_ptr<ID3D11DepthStencilState> state(s);
	if (FAILED(hr))
		THROW_DX(hr);
	return state;
}

dx_ptr<ID3D11RasterizerState> mini::DxDevice::CreateRasterizerState(const RasterizerDescription& desc) const
{
	ID3D11RasterizerState* s = nullptr;
	auto hr = m_device->CreateRasterizerState(&desc, &s);
	dx_ptr<ID3D11RasterizerState> state(s);
	if (FAILED(hr))
		THROW_DX(hr);
	return state;
}

dx_ptr<ID3D11ShaderResourceView> mini::DxDevice::CreateShaderResourceView(const dx_ptr<ID3D11Texture2D>& texture) const
{
	ID3D11ShaderResourceView* srv;
	auto hr = m_device->CreateShaderResourceView(texture.get(), nullptr/*use default shader resource view description*/, &srv);
	dx_ptr<ID3D11ShaderResourceView> resourceView(srv);
	if (FAILED(hr))
		THROW_DX(hr);
	return resourceView;
}

dx_ptr<ID3D11ShaderResourceView> mini::DxDevice::CreateShaderResourceView(const std::wstring& texPath) const
{
	ID3D11ShaderResourceView* rv = nullptr;;
	HRESULT hr = 0;
	const wstring ext{ L".dds" };
	if (texPath.size() > ext.size() && texPath.compare(texPath.size() - ext.size(), ext.size(), ext) == 0)
		hr = DirectX::CreateDDSTextureFromFile(m_device.get(), m_context.get(), texPath.c_str(), nullptr, &rv);
	else
		hr = DirectX::CreateWICTextureFromFile(m_device.get(), m_context.get(), texPath.c_str(), nullptr, &rv);
	dx_ptr<ID3D11ShaderResourceView> resourceView(rv);
	if (FAILED(hr))
		//Make sure CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); is called before first use of this function!
		THROW_DX(hr);
	return resourceView;
}

dx_ptr<ID3D11SamplerState> mini::DxDevice::CreateSamplerState(const SamplerDescription& desc) const
{
	ID3D11SamplerState* s = nullptr;
	auto hr = m_device->CreateSamplerState(&desc, &s);
	dx_ptr<ID3D11SamplerState> sampler(s);
	if (FAILED(hr))
		THROW_DX(hr);
	return sampler;
}
