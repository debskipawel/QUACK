#include "effect.h"

using namespace mini;

void DynamicEffect::Begin(const dx_ptr<ID3D11DeviceContext>& context) const
{
	for (auto& compPtr : m_components)
		compPtr->Begin(context);
}

//Effect::Effect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11HullShader>&& hs, dx_ptr<ID3D11DomainShader>&& ds, dx_ptr<ID3D11GeometryShader>&& gs, dx_ptr<ID3D11PixelShader>&& ps)
//	: m_vs(move(vs)), m_hs(move(hs)), m_ds(move(ds)), m_gs(move(gs)), m_ps(move(ps))
//{ }
//
//Effect& Effect::SetShaders(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11HullShader>&& hs, dx_ptr<ID3D11DomainShader>&& ds, dx_ptr<ID3D11GeometryShader>&& gs, dx_ptr<ID3D11PixelShader>&& ps)
//{
//	SetVertexShader(move(vs));
//	SetHullShader(move(hs));
//	SetDomainShader(move(ds));
//	SetGeometryShader(move(gs));
//	SetPixelShader(move(ps));
//	return *this;
//}
//
//void Effect::Begin(const dx_ptr<ID3D11DeviceContext>& context) const
//{
//	context->VSSetShader(m_vs.get(), nullptr, 0);
//	context->HSSetShader(m_hs.get(), nullptr, 0);
//	context->DSSetShader(m_ds.get(), nullptr, 0);
//	context->GSSetShader(m_gs.get(), nullptr, 0);
//	context->PSSetShader(m_ps.get(), nullptr, 0);
//}
