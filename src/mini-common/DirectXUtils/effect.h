#pragma once

#include "dxptr.h"
#include <D3D11.h>
#include <vector>
#include "dxArray.h"
#include "constantBuffer.h"
#include <iterator>
#include "dxstructures.h"

namespace mini
{
	class EffectComponent
	{
	public:
		virtual ~EffectComponent() = default;
		EffectComponent() = default;
		EffectComponent(const EffectComponent& other) = delete;
		EffectComponent(EffectComponent&& other) = default;

		EffectComponent& operator=(const EffectComponent& other) = delete;
		EffectComponent& operator=(EffectComponent&& other) = default;

		virtual void Begin(const dx_ptr<ID3D11DeviceContext>& context) const = 0;
	};

	class BasicEffect : public EffectComponent
	{
	public:
		explicit BasicEffect(dx_ptr<ID3D11VertexShader>&& vs = nullptr, dx_ptr<ID3D11PixelShader>&& ps = nullptr)
			: m_vs(move(vs)), m_ps(move(ps)) { }

		BasicEffect(BasicEffect&& other) noexcept
			: m_vs(move(other.m_vs)), m_ps(move(other.m_ps)) { }

		BasicEffect& operator=(BasicEffect&& other) noexcept
		{
			SetVertexShader(move(other.m_vs));
			SetPixelShader(move(other.m_ps));
			return *this;
		}

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			context->VSSetShader(m_vs.get(), nullptr, 0);
			context->PSSetShader(m_ps.get(), nullptr, 0);
		}

		void SetVertexShader(dx_ptr<ID3D11VertexShader>&& vs) { m_vs = move(vs); }
		void SetPixelShader(dx_ptr<ID3D11PixelShader>&& ps) { m_ps = move(ps); }

		dx_ptr<ID3D11VertexShader> m_vs;
		dx_ptr<ID3D11PixelShader> m_ps;
	};

	class TessellationEffectComponent : public EffectComponent
	{
	public:
		explicit TessellationEffectComponent(dx_ptr<ID3D11HullShader>&& hs = nullptr, dx_ptr<ID3D11DomainShader>&& ds = nullptr)
			: m_hs(move(hs)), m_ds(move(ds)) { }

		TessellationEffectComponent(TessellationEffectComponent&& other) noexcept
			: m_hs(move(other.m_hs)), m_ds(move(other.m_ds)) { }

		TessellationEffectComponent& operator=(TessellationEffectComponent&& other) noexcept
		{
			SetHullShader(move(other.m_hs));
			SetDomainShader(move(other.m_ds));
			return *this;
		}

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			context->HSSetShader(m_hs.get(), nullptr, 0);
			context->DSSetShader(m_ds.get(), nullptr, 0);
		}

		void SetHullShader(dx_ptr<ID3D11HullShader>&& hs) { m_hs = move(hs); }
		void SetDomainShader(dx_ptr<ID3D11DomainShader>&& ds) { m_ds = move(ds); }

		dx_ptr<ID3D11HullShader> m_hs;
		dx_ptr<ID3D11DomainShader> m_ds;
	};

	class GeometryShaderComponent : public EffectComponent
	{
	public:
		explicit GeometryShaderComponent(dx_ptr<ID3D11GeometryShader>&& gs = nullptr)
			: m_gs(move(gs)) { }

		GeometryShaderComponent(GeometryShaderComponent&& other) noexcept
			: m_gs(move(other.m_gs)) { }

		GeometryShaderComponent& operator=(GeometryShaderComponent&& other) noexcept
		{
			SetGeometryShader(move(other.m_gs));
			return *this;
		}

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			context->GSSetShader(m_gs.get(), nullptr, 0);
		}

		void SetGeometryShader(dx_ptr<ID3D11GeometryShader>&& gs) { m_gs = move(gs); }

		dx_ptr<ID3D11GeometryShader> m_gs;
	};

	template<typename T>
	class ResourceSet
	{
	public:
		using pointer = T*;
		using value_type = T;
		using unique_ptr_type = dx_ptr<value_type>;

		ResourceSet()
			: m_buffers(0)
		{}

		template<typename... Args>
		explicit ResourceSet(dx_ptr<Args>&&... args)
			: m_buffers(sizeof...(Args))
		{
			m_buffers.assign({ args.release()... });
		}

		template<typename... Args>
		explicit ResourceSet(const dx_ptr<Args>&... args)
			: m_buffers(0)
		{
			m_buffers.reserve(sizeof...(Args));
			pointer ptr[] = { args.get()... };
			for (pointer p : ptr)
				m_buffers.push_back(p);
			_incrementRefs();
		}

		ResourceSet(std::initializer_list<pointer> ilist)
			: m_buffers(ilist)
		{
			_incrementRefs();
		}

		ResourceSet(const ResourceSet<T>& other)
		{
			m_buffers.reserve(other.m_buffers.size());
			for(auto buffer : other.m_buffers)
			{
				m_buffers.push_back(buffer);
				buffer->AddRef();
			}
		}

		ResourceSet(ResourceSet<T>&& other) noexcept
			: m_buffers(std::move(other.m_buffers))
		{ }

		ResourceSet& operator=(ResourceSet<T>&& other) noexcept
		{
			m_buffers = std::move(other.m_buffers);
			return *this;
		}

		ResourceSet& operator=(const ResourceSet<T>& other)
		{
			m_buffers.clear();
			m_buffers.reserve(other.m_buffers.size());
			for (auto buffer : other.m_buffers)
			{
				m_buffers.push_back(buffer);
				buffer->AddRef();
			}
			return *this;
		}

		void SetResources(std::initializer_list<pointer> ilist)
		{
			m_buffers = ilist;
			_incrementRefs();
		}

		void SetResource(unsigned slot, pointer buffer)
		{
			_addPtr(slot, buffer);
			buffer->AddRef();
		}

		void SetResource(unsigned slot, unique_ptr_type&& buffer)
		{
			_addPtr(slot, buffer.release());
		}

		void SetResource(unsigned slot, const unique_ptr_type& buffer)
		{
			SetResource(slot, clone(buffer));
		}

		dx_ptr_vector<value_type> m_buffers;

	private:
		void _incrementRefs()
		{
			for (auto ref : m_buffers)
				if (ref)
					ref->AddRef();
		}

		void _addPtr(unsigned slot, pointer buffer)
		{
			if (m_buffers.size() <= slot)
				m_buffers.resize(static_cast<size_t>(slot) + 1);
			m_buffers[slot] = buffer;
		}
	};

#define RESOURCE_SET_CLASS_NAME(SHADER_TYPE, RESOURCE_TYPE) SHADER_TYPE ## RESOURCE_TYPE ## s

#define RESOURCE_SET_BASE_TYPE(RESOURCE_TYPE) RESOURCE_TYPE ## Set

#define SET_RESOURCE_F_NAME(SHADER_TYPE, RESOURCE_TYPE) Set ## SHADER_TYPE ## RESOURCE_TYPE

#define SET_RESOURCE_SET_F_NAME(SHADER_TYPE, RESOURCE_TYPE) Set ## SHADER_TYPE ## RESOURCE_TYPE ## s

#define CONTEXT_SET_F_NAME(SHADER_TYPE, RESOURCE_TYPE) SHADER_TYPE ## Set ## RESOURCE_TYPE ## s

#define GET_RESOURCES_VECTOR_F_NAME(SHADER_TYPE, RESOURCE_TYPE) get ## SHADER_TYPE ## RESOURCE_TYPE ## Vector

#define DEFINE_RESOURCE_SET_CLASS(SHADER_TYPE, RESOURCE_TYPE) \
	class RESOURCE_SET_CLASS_NAME(SHADER_TYPE, RESOURCE_TYPE) : public EffectComponent, public RESOURCE_SET_BASE_TYPE(RESOURCE_TYPE) {\
	public:\
		using base_set_type = RESOURCE_SET_BASE_TYPE(RESOURCE_TYPE);\
		using pointer = base_set_type::pointer;\
		using value_type = base_set_type::value_type;\
		using unique_ptr_type = base_set_type::unique_ptr_type; \
		using base_set_type ::ResourceSet;\
		void SET_RESOURCE_F_NAME(SHADER_TYPE, RESOURCE_TYPE) (unsigned slot, pointer buffer) {\
			SetResource(slot, buffer);\
		}\
		void SET_RESOURCE_F_NAME(SHADER_TYPE, RESOURCE_TYPE) (unsigned slot, unique_ptr_type&& buffer) {\
			SetResource(slot, std::move(buffer));\
		}\
		void SET_RESOURCE_F_NAME(SHADER_TYPE, RESOURCE_TYPE) (unsigned slot, const unique_ptr_type& buffer) {\
			SetResource(slot, buffer);\
		}\
		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override {\
			context-> CONTEXT_SET_F_NAME(SHADER_TYPE, RESOURCE_TYPE) (0, static_cast<unsigned>(m_buffers.size()), m_buffers.data());\
		}\
		dx_ptr_vector<value_type>& GET_RESOURCES_VECTOR_F_NAME(SHADER_TYPE, RESOURCE_TYPE) ()\
		{ return m_buffers; }\
		const dx_ptr_vector<value_type>& GET_RESOURCES_VECTOR_F_NAME(SHADER_TYPE, RESOURCE_TYPE) () const\
		{ return m_buffers; }\
	}

#define DEFINE_SHADER_RESOURCE_SET_CLASSES(RESOURCE_TYPE, PTR_TYPE) \
	using RESOURCE_SET_BASE_TYPE(RESOURCE_TYPE) = ResourceSet<PTR_TYPE>;\
	DEFINE_RESOURCE_SET_CLASS(VS, RESOURCE_TYPE);\
	DEFINE_RESOURCE_SET_CLASS(HS, RESOURCE_TYPE);\
	DEFINE_RESOURCE_SET_CLASS(DS, RESOURCE_TYPE);\
	DEFINE_RESOURCE_SET_CLASS(GS, RESOURCE_TYPE);\
	DEFINE_RESOURCE_SET_CLASS(PS, RESOURCE_TYPE);

	DEFINE_SHADER_RESOURCE_SET_CLASSES(ConstantBuffer, ID3D11Buffer)

	DEFINE_SHADER_RESOURCE_SET_CLASSES(ShaderResource, ID3D11ShaderResourceView)

	DEFINE_SHADER_RESOURCE_SET_CLASSES(Sampler, ID3D11SamplerState)

	using RenderTargetSet = ResourceSet<ID3D11RenderTargetView>;

	class RenderTargetsEffect : public EffectComponent, public RenderTargetSet
	{
	public:

		RenderTargetsEffect() = default;

		RenderTargetsEffect(RenderTargetsEffect&& other) = default;

		RenderTargetsEffect(const RenderTargetsEffect& other)
			: RenderTargetSet(other), m_viewport(other.m_viewport), m_depthBuffer(clone(other.m_depthBuffer)), m_clearOnBegin(other.m_clearOnBegin)
		{ }

		template<typename... ARGS>
		RenderTargetsEffect(const utils::ViewportDescription& viewport, dx_ptr<ID3D11DepthStencilView>&& depthBuffer, dx_ptr<ARGS>&&... args)
			: ResourceSet(args...), m_viewport(viewport), m_depthBuffer(std::move(depthBuffer)), m_clearOnBegin(false)
		{ }

		template<typename... ARGS>
		explicit RenderTargetsEffect(utils::ViewportDescription viewport, dx_ptr<ARGS>&& ...args)
			: ResourceSet(args...), m_viewport(viewport), m_depthBuffer(nullptr), m_clearOnBegin(false)
		{ }

		RenderTargetsEffect& operator=(RenderTargetsEffect&& other) = default;

		RenderTargetsEffect& operator=(const RenderTargetsEffect& other)
		{
			static_cast<RenderTargetSet&>(*this) = static_cast<const RenderTargetSet&>(other);
			m_viewport = other.m_viewport;
			m_depthBuffer = clone(other.m_depthBuffer);
			m_clearOnBegin = other.m_clearOnBegin;
			return *this;
		}

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			if (m_clearOnBegin)
				ClearRenderTargets(context);
			context->RSSetViewports(1, &m_viewport);
			context->OMSetRenderTargets(static_cast<UINT>(m_buffers.size()), m_buffers.data(), m_depthBuffer.get());
		}

		void SetRenderTarget(unsigned slot, dx_ptr<ID3D11RenderTargetView>&& renderTarget)
		{
			SetResource(slot, std::move(renderTarget));
		}

		void SetRenderTargets(std::initializer_list<ID3D11RenderTargetView*> ilist)
		{
			SetResources(ilist);
		}

		void SetDepthStencilBuffer(dx_ptr<ID3D11DepthStencilView>&& depthBuffer)
		{
			m_depthBuffer = std::move(depthBuffer);
		}

		void SetViewport(const utils::ViewportDescription& viewport)
		{
			m_viewport = viewport;
		}

		void SetClearOnBegin(bool clearOnBegin) { m_clearOnBegin = clearOnBegin; }

		void ClearDepthStencil(const dx_ptr<ID3D11DeviceContext>& context,
			UINT clearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) const
		{
			if (m_depthBuffer)
				context->ClearDepthStencilView(m_depthBuffer.get(), clearFlags, 1.0f, 0);
		}

		void ClearRenderTargets(const dx_ptr<ID3D11DeviceContext>& context, const float (&color)[4] = { 0.0f, 0.0f, 0.0f, 0.0f }) const
		{
			for (auto rtv : m_buffers)
				context->ClearRenderTargetView(rtv, color);
			ClearDepthStencil(context);
		}

		ID3D11RenderTargetView* getRenderTarget(unsigned slot) const { return m_buffers[slot]; }
		ID3D11DepthStencilView* getDepthStencilBuffer() const { return m_depthBuffer.get(); }
		const utils::ViewportDescription& getViewport() const { return m_viewport; }

	private:
		utils::ViewportDescription m_viewport;
		dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
		bool m_clearOnBegin = false;
	};

	class InputLayoutEffect : public EffectComponent
	{
	public:
		InputLayoutEffect() = default;
		InputLayoutEffect(InputLayoutEffect&& other) = default;

		explicit InputLayoutEffect(dx_ptr<ID3D11InputLayout>&& layout)
			: m_layout(std::move(layout)) { }

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			context->IASetInputLayout(m_layout.get());
		}

		void SetInputLayout(dx_ptr<ID3D11InputLayout>&& layout) { m_layout = std::move(layout); }

	private:
		dx_ptr<ID3D11InputLayout> m_layout;
	};

	class RasterizerEffect : public EffectComponent
	{
	public:
		RasterizerEffect() = default;
		RasterizerEffect(RasterizerEffect&& other) = default;

		explicit RasterizerEffect(dx_ptr<ID3D11RasterizerState>&& state)
			: m_state(std::move(state)) { }

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			context->RSSetState(m_state.get());
		}

		void SetState(dx_ptr<ID3D11RasterizerState>&& state) { m_state = std::move(state); }

	private:
		dx_ptr<ID3D11RasterizerState> m_state;
	};

	class DynamicEffect : public EffectComponent
	{
	public:
		DynamicEffect() = default;

		template<typename COMPONENT, typename... COMPONENTS_T>
		explicit DynamicEffect(std::unique_ptr<COMPONENT>& component, std::unique_ptr<COMPONENTS_T>&&... components)
		{
			using namespace std;
			unique_ptr<EffectComponent> ptrs[] = { move(component), move(components) ... };
			m_components.assign(make_move_iterator(begin(ptrs)), make_move_iterator(end(ptrs)));
		}

		DynamicEffect(DynamicEffect&& other) noexcept
			: m_components(move(other.m_components)) { }

		DynamicEffect& operator=(DynamicEffect&& other) noexcept
		{
			m_components = move(other.m_components);
			other.m_components.clear();
			return *this;
		}

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override;

		//this field is public since any content of the vector is a valid one
		//and replicating an iterface for modifying the contents of the vector
		//in the class redundant
		std::vector<std::unique_ptr<EffectComponent>> m_components;
	};

	template<typename... COMPONENTS_T>
	class StaticEffect : public COMPONENTS_T ...
	{
	public:
		StaticEffect() = default;

		explicit StaticEffect(COMPONENTS_T&&... components)
		{
			_Assign<COMPONENTS_T...>(std::move(components)...);
		}

		void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
		{
			_Begin<COMPONENTS_T...>(context);
		}

	private:

		template<typename T>
		void _Assign(T&& component)
		{
			static_cast<T&>(*this) = std::move(component);
		}

		template<typename T1, typename T2, typename... ARGS>
		void _Assign(T1&& component1, T2&& component2, ARGS&&... otherComponents)
		{
			static_cast<T1&>(*this) = std::move(component1);
			_Assign<T2, ARGS...>(std::move(component2), std::move(otherComponents)...);
		}

		template<typename T>
		void _Begin(const dx_ptr<ID3D11DeviceContext>& context) const
		{
			T::Begin(context);
		}

		template<typename T, typename T2, typename... ARGS>
		void _Begin(const dx_ptr<ID3D11DeviceContext>& context) const
		{
			T::Begin(context);
			_Begin<T2, ARGS...>(context);
		}
	};
	
	/*class Effect
	{
	public:
		virtual ~Effect() { }

		Effect() : Effect(nullptr, nullptr, nullptr, nullptr, nullptr) { }

		Effect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps)
			: Effect(move(vs), nullptr, nullptr, nullptr, move(ps)) { }

		Effect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11GeometryShader>&& gs, dx_ptr<ID3D11PixelShader>&& ps)
			: Effect(move(vs), nullptr, nullptr, move(gs), move(ps)) { }

		Effect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11HullShader>&& hs,
			dx_ptr<ID3D11DomainShader>&& ds, dx_ptr<ID3D11PixelShader>&& ps)
			: Effect(move(vs), move(hs), move(ds), nullptr, move(ps)) { }

		Effect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11HullShader>&& hs, dx_ptr<ID3D11DomainShader>&& ds,
			dx_ptr<ID3D11GeometryShader>&& gs, dx_ptr<ID3D11PixelShader>&& ps);

		Effect(Effect&& other) noexcept
			: Effect(move(other.m_vs), move(other.m_hs), move(other.m_ds), move(other.m_gs), move(other.m_ps)) { }

		Effect& operator=(Effect&& other) noexcept
		{ return SetShaders(move(other.m_vs), move(other.m_hs), move(other.m_ds), move(other.m_gs), move(other.m_ps)); }

		Effect& SetPixelShader(dx_ptr<ID3D11PixelShader>&& ps) { m_ps = move(ps); return *this; }
		Effect& SetHullShader(dx_ptr<ID3D11HullShader>&& hs) { m_hs = move(hs); return *this; }
		Effect& SetDomainShader(dx_ptr<ID3D11DomainShader>&& ds) { m_ds = move(ds); return *this; }
		Effect& SetGeometryShader(dx_ptr<ID3D11GeometryShader>&& gs) { m_gs = move(gs); return *this; }
		Effect& SetVertexShader(dx_ptr<ID3D11VertexShader>&& vs) { m_vs = move(vs); return *this; }

		Effect& SetShaders(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps)
		{ return SetShaders(move(vs), nullptr, nullptr, nullptr, move(ps)); }
		Effect& SetShaders(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11GeometryShader>&& gs, dx_ptr<ID3D11PixelShader>&& ps)
		{ return SetShaders(move(vs), nullptr, nullptr, move(gs), move(ps)); }
		Effect& SetShaders(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11HullShader>&& hs,
			dx_ptr<ID3D11DomainShader>&& ds, dx_ptr<ID3D11PixelShader>&& ps)
		{ return SetShaders(move(vs), move(hs), move(ds), nullptr, move(ps)); }
		Effect& SetShaders(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11HullShader>&& hs, dx_ptr<ID3D11DomainShader>&& ds,
			dx_ptr<ID3D11GeometryShader>&& gs, dx_ptr<ID3D11PixelShader>&& ps);

		virtual void Begin(const dx_ptr<ID3D11DeviceContext>& context) const;

		Effect Copy() const { return{ clone(m_vs), clone(m_hs), clone(m_ds), clone(m_gs), clone(m_ps) }; }
		
	private:
		dx_ptr<ID3D11VertexShader> m_vs;
		dx_ptr<ID3D11HullShader> m_hs;
		dx_ptr<ID3D11DomainShader> m_ds;
		dx_ptr<ID3D11GeometryShader> m_gs;
		dx_ptr<ID3D11PixelShader> m_ps;
	};*/
}
