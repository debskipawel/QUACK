#include "gk2ShaderDemoBase.h"
#include "model.h"
#include "windowsx.h"

using namespace std;
using namespace DirectX;
using namespace mini;
using namespace gk2;


class CopyTextureEffectBase : public EffectComponent
{
public:
	using EffectComponent::EffectComponent;
	using EffectComponent::operator=;

	void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
	{
		context->CopyResource(_getDestination(context).get(), _getSource(context).get());
	}

protected:
	virtual dx_ptr<ID3D11Resource> _getSource(const dx_ptr<ID3D11DeviceContext>& context) const = 0;
	virtual dx_ptr<ID3D11Resource> _getDestination(const dx_ptr<ID3D11DeviceContext>& context) const = 0;
};

class CopyIntoTextureEffectBase : public CopyTextureEffectBase
{
public:
	CopyIntoTextureEffectBase() = default;

	CopyIntoTextureEffectBase(const dx_ptr<ID3D11Texture2D>& destination)
		: m_destination{ destination.get() }
	{
		m_destination->AddRef();
	}

	template<typename TView, typename TVoid = std::enable_if_t<std::is_base_of_v<ID3D11View, TView>>>
	CopyIntoTextureEffectBase(const dx_ptr<TView>& dstView)
	{
		ID3D11Resource* prs = nullptr;
		static_cast<ID3D11View*>(dstView.get())->GetResource(&prs);
		m_destination.reset(prs);
	}

	CopyIntoTextureEffectBase(CopyIntoTextureEffectBase&& other) = default;
	CopyIntoTextureEffectBase& operator=(CopyIntoTextureEffectBase&& other) = default;

	void SetDestination(const dx_ptr<ID3D11Texture2D>& destination) noexcept
	{
		m_destination.reset(destination.get());
		m_destination->AddRef();
	}

	template<typename TView>
	std::enable_if_t<std::is_base_of_v<ID3D11View, TView>> SetDestination(const dx_ptr<TView>& dstView)
	{
		ID3D11Resource* prs = nullptr;
		static_cast<ID3D11View*>(dstView.get())->GetResource(prs);
		m_destination.reset(prs);
	}

protected:
	dx_ptr<ID3D11Resource> _getDestination(const dx_ptr<ID3D11DeviceContext>&) const override
	{
		return clone(m_destination);
	}

private:
	dx_ptr<ID3D11Resource> m_destination;
};

class CopyRenderTargetEffect : public CopyIntoTextureEffectBase
{
public:
	using CopyIntoTextureEffectBase::CopyIntoTextureEffectBase;
	using CopyIntoTextureEffectBase::operator=;

protected:
	dx_ptr<ID3D11Resource> _getSource(const dx_ptr<ID3D11DeviceContext>& context) const override
	{
		ID3D11RenderTargetView* prtv = nullptr;
		context->OMGetRenderTargets(1, &prtv, nullptr);
		dx_ptr<ID3D11RenderTargetView> rtv{ prtv };
		if (!rtv)
			THROW(L"Failed to obtain render target view");

		ID3D11Resource* prs = nullptr;
		rtv->GetResource(&prs);
		return dx_ptr<ID3D11Resource>{ prs };
	}
};

class CopyDephtBufferEffect : public CopyIntoTextureEffectBase
{
public:
	using CopyIntoTextureEffectBase::CopyIntoTextureEffectBase;
	using CopyIntoTextureEffectBase::operator=;

protected:
	dx_ptr<ID3D11Resource> _getSource(const dx_ptr<ID3D11DeviceContext>& context) const override
	{
		ID3D11DepthStencilView* pdsv = nullptr;
		context->OMGetRenderTargets(0, nullptr, &pdsv);
		dx_ptr<ID3D11DepthStencilView> dsv{ pdsv };
		if (!dsv)
			THROW(L"Failed to obtain render target view");

		ID3D11Resource* prs = nullptr;
		dsv->GetResource(&prs);
		return dx_ptr<ID3D11Resource>{ prs };
	}
};

GK2ShaderDemoBase::GK2ShaderDemoBase(HINSTANCE hInst)
	: DxApplication(hInst, 1280, 720, L"Shader Demo"), m_loader(m_device), m_layouts(m_device), m_camera(0.01f, 50.0f, 5),
	  m_frustrum(m_window.getClientSize(), XM_PIDIV4, 0.5f, 85.0f), m_gui(m_device, m_window)
{
}

bool GK2ShaderDemoBase::ProcessMessage(WindowMessage& msg)
{
	static bool hasCapture = false;
	static POINT lastPos{ 0,0 };
	bool guiResult = m_gui.ProcessMessage(msg);
	if (guiResult)
		msg.result = 0;
	ImGuiIO& io = ImGui::GetIO();
	switch(msg.message)
	{
	case WM_MBUTTONDOWN:
		if (!io.WantCaptureMouse)
			return guiResult;
		//fallthrough
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (!hasCapture)
		{
			hasCapture = true;
			SetCapture(m_window.getHandle());
		}
		msg.result = 0;
		return true;
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (hasCapture && (msg.wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON))==0)
		{
			hasCapture = false;
			ReleaseCapture();
		}
		msg.result = 0;
		return true;
	case WM_MOUSEMOVE:
		if (io.WantCaptureMouse)
			return guiResult;
		POINT pos{ GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
		if (msg.wParam & MK_LBUTTON)
			m_camera.Rotate(static_cast<float>(pos.y - lastPos.y) * ROTATION_SPEED,
				static_cast<float>(pos.x - lastPos.x) * ROTATION_SPEED);
		else if (msg.wParam & MK_RBUTTON)
			m_camera.Zoom(static_cast<float>(pos.y - lastPos.y) * ZOOM_SPEED);
		if (msg.wParam & (MK_LBUTTON | MK_RBUTTON))
			m_variables.UpdateView(m_camera, m_frustrum);
		lastPos = pos;
		msg.result = 0;
		return true;
	}
	return WindowApplication::ProcessMessage(msg);
}

int GK2ShaderDemoBase::MainLoop()
{
	m_variables.UpdateViewAndFrustrum(m_camera, m_frustrum);
	return DxApplication::MainLoop();
}

void GK2ShaderDemoBase::Update(const Clock& c)
{
	m_gui.Update(c);
	ImGui::SetNextWindowSize({ 300.0f, 720.0 }, ImGuiSetCond_Always);
	ImGui::SetNextWindowPos({ 980.0f, 0.0f }, ImGuiSetCond_Always);
	ImGui::Begin("Variables", nullptr,
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
	m_variables.UpdateFrame(m_device.context(), c);
	ImGui::End();
}

void GK2ShaderDemoBase::Render()
{
	auto& rt = getDefaultRenderTarget();
	float clearColor[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
	rt.ClearRenderTargets(m_device.context(), clearColor);
	rt.Begin(m_device.context());
	for (auto& p : m_passes)
		p.Execute(m_device.context(), m_variables);
	m_gui.Render(m_device);
}

size_t GK2ShaderDemoBase::addModelFromFile(const std::string& path)
{
	m_models.push_back(make_unique<Model>(m_loader.LoadFromFile(path, m_layouts)));
	return m_models.size() - 1;
}

size_t GK2ShaderDemoBase::addModelFromString(const std::string& model, bool smoothNormals)
{
	m_models.push_back(make_unique<Model>(m_loader.LoadFromString(model, m_layouts, smoothNormals)));
	return m_models.size() - 1;
}

size_t GK2ShaderDemoBase::addPass(const std::wstring& vsShader, const std::wstring& psShader)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, vsShader, psShader);
	return m_passes.size() - 1;
}

size_t GK2ShaderDemoBase::addPass(const std::wstring& vsShader, const std::wstring& gsShader,
	const std::wstring& psShader)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, vsShader, gsShader, psShader);
	return m_passes.size() - 1;
}

size_t GK2ShaderDemoBase::addPass(const std::wstring& vsShader, const std::wstring& psShader,
	const std::string& renderTarget, bool clearRenderTarget)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, m_variables.GetRenderTarget(renderTarget),
		clearRenderTarget, vsShader, psShader);
	return m_passes.size() - 1;
}
size_t GK2ShaderDemoBase::addPass(const std::wstring& vsShader, const std::wstring& psShader,
	const RenderTargetsEffect& renderTarget, bool clearRenderTarget)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, renderTarget, clearRenderTarget, vsShader, psShader);
	return m_passes.size() - 1;
}

void GK2ShaderDemoBase::addRasterizerState(size_t passId, const utils::RasterizerDescription& desc)
{
	m_passes[passId].AddEffect(make_unique<RasterizerEffect>(m_device.CreateRasterizerState(desc)));
}

void GK2ShaderDemoBase::addModelToPass(size_t passId, size_t modelId)
{
	m_passes[passId].AddModel(m_models[modelId].get());
}

void mini::gk2::GK2ShaderDemoBase::copyRenderTarget(size_t passId, std::string dstTexture)
{
	pass(passId).EmplaceEffect<CopyRenderTargetEffect>(m_variables.GetTexture(dstTexture));
}

void mini::gk2::GK2ShaderDemoBase::copyDepthBuffer(size_t passId, std::string dstTexture)
{
	pass(passId).EmplaceEffect<CopyDephtBufferEffect>(m_variables.GetTexture(dstTexture));
}
