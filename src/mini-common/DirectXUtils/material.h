#pragma once

#include <DirectXMath.h>
#include <D3D11.h>
#include "dxptr.h"

namespace mini
{
	class Material
	{
	public:
		struct MaterialData
		{
			DirectX::XMFLOAT4 diffuseColor; //[r, g, b, a]
			DirectX::XMFLOAT4 specularColor; //[r, g, b, m]
		};

		Material();
		Material(Material&& right);
		Material& operator =(Material&& right);
		Material(const Material& right) = delete;
		Material& operator =(const Material& right) = delete;

		const MaterialData& getMaterialData() const { return m_data; }
		void setMaterialData(const MaterialData& data) { m_data = data; }

		const dx_ptr<ID3D11ShaderResourceView>& getDiffuseTexture() const { return m_diffuseTexture; }
		void setDiffuseTexture(dx_ptr<ID3D11ShaderResourceView>&& tex) { m_diffuseTexture = std::move(tex); }
		const dx_ptr<ID3D11ShaderResourceView>& getSpecularTexture() const { return m_specularTexture; }
		void setSpecularTexture(dx_ptr<ID3D11ShaderResourceView>&& tex) { m_specularTexture = std::move(tex); }

	private:

		void Release();

		MaterialData m_data;
		dx_ptr<ID3D11ShaderResourceView> m_diffuseTexture;
		dx_ptr<ID3D11ShaderResourceView> m_specularTexture;
	};
}
