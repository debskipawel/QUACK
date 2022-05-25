#pragma once
#include "effect.h"
#include "model.h"
#include "cbVariableManager.h"
#include "dxDevice.h"
#include "exceptions.h"
#include <type_traits>

typedef struct _D3D11_SHADER_DESC D3D11_SHADER_DESC;
struct ID3D11ShaderReflection;

namespace mini
{
	class InputLayoutManager;
	namespace gk2
	{
		class RenderPass
		{
			class ICBVariablesEffect
			{
			public:
				virtual ~ICBVariablesEffect() = default;
				virtual void Update(const dx_ptr<ID3D11DeviceContext>& context, const CBVariableManager& manager) = 0;
			};

			template<typename ConstantBufferEffectT>
			class CBVariablesEffect : public ConstantBufferEffectT, public ICBVariablesEffect
			{
			public:
				using MyBase = ConstantBufferEffectT;
				using MyResourceSetT = ResourceSet<ID3D11Buffer>;

				static_assert(std::is_base_of<EffectComponent, ConstantBufferEffectT>::value &&
					std::is_base_of<MyResourceSetT, ConstantBufferEffectT>::value,
					"Base class of CBVariableEffect needst to be a shader constant buffer effect type");



				CBVariablesEffect(const DxDevice& device, std::vector<CBufferDesc>&& buffers)
					: m_bufferDescriptions(std::move(buffers))
				{
					for (unsigned int i = 0; i < m_bufferDescriptions.size(); ++i)
					{
						CBufferDesc& bufferDesc = m_bufferDescriptions[i];
						MyBase::SetResource(i,
							device.CreateBuffer(
								utils::BufferDescription::ConstantBufferDescription(
									static_cast<UINT>(bufferDesc.size))));
					}
				}

				void Update(const dx_ptr<ID3D11DeviceContext>& context, const CBVariableManager& manager) override
				{
					for (unsigned int i = 0; i < m_bufferDescriptions.size(); ++i)
					{
						D3D11_MAPPED_SUBRESOURCE resource;
						ID3D11Buffer* cbuffer = MyBase::m_buffers[i].get();
						auto hr = context->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
						if (FAILED(hr))
							THROW_DX(hr);
						manager.FillCBuffer(resource.pData, m_bufferDescriptions[i]);
						context->Unmap(cbuffer, 0);
					}
				}

			private:

				std::vector<CBufferDesc> m_bufferDescriptions;
			};

		public:
			static constexpr D3D11_INPUT_ELEMENT_DESC DefaultLayout[3] ={
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};

			RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
				const std::wstring& vsShader, const std::wstring& psShader);
			RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
				const std::wstring& vsShader, const std::wstring& gsShader, const std::wstring& psShader);

			RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
				const RenderTargetsEffect& renderTarget, const std::wstring& vsShader, const std::wstring& psShader)
				: RenderPass(device, variables, layouts, renderTarget, false, vsShader, psShader) { }
			RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
				const RenderTargetsEffect& renderTarget, const std::wstring& vsShader, const std::wstring& gsShader,
				const std::wstring& psShader)
				: RenderPass(device, variables, layouts, renderTarget, false, vsShader, gsShader, psShader) { }
			RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
				const RenderTargetsEffect& renderTarget, bool clearRenderTarget,
				const std::wstring& vsShader, const std::wstring& psShader);
			RenderPass(const DxDevice& device, const CBVariableManager& variables, InputLayoutManager* layouts,
				const RenderTargetsEffect& renderTarget, bool clearRenderTarget,
				const std::wstring& vsShader, const std::wstring& gsShader, const std::wstring& psShader);

			//stores the pointer to the model. Make sure it will exist throughout RenderPass lifetime.
			void AddModel(const Model* m);

			void AddEffect(std::unique_ptr<EffectComponent>&& effect);

			template<class T>
			std::enable_if_t<std::is_base_of_v<EffectComponent, std::remove_cv_t<std::remove_reference_t<T>>>>
			AddEffect(T&& effect)
			{
				AddEffect(std::make_unique<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(effect)));
			}

			template<class T, class ... TArgs>
			std::enable_if_t<std::is_base_of_v<EffectComponent, T>>
			EmplaceEffect(TArgs&& ... args)
			{
				AddEffect(std::make_unique<T>(std::forward<TArgs>(args)...));
			}

			void Execute(const dx_ptr<ID3D11DeviceContext>& context, CBVariableManager& manager);

		private:
			void _initShaders(const DxDevice& device, const CBVariableManager& variables,
				const std::wstring& vsShader, const std::wstring& psShader);

			void _initShaders(const DxDevice& device, const CBVariableManager& variables,
				const std::wstring& vsShader, const std::wstring& gsShader, const std::wstring& psShader);

			void _addRenderTarget(const RenderTargetsEffect& renderTarget, bool clearRenderTarget);

			static dx_ptr<ID3D11ShaderReflection> _reflectShader(const std::vector<BYTE>& shaderCode, D3D11_SHADER_DESC& shaderDesc);

			std::vector<CBufferDesc> _getConstantBufferDescriptions(const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc);

			void _updateCBuffers(const dx_ptr<ID3D11DeviceContext>& context, const CBVariableManager& manager);

			template<typename ConstantBufferEffectT>
			void _addShaderConstantBuffers(const DxDevice& device, const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc)
			{
				std::vector<CBufferDesc> buffers = _getConstantBufferDescriptions(shaderRefl, shaderDesc);
				if (!buffers.empty())
				{
					auto uptr = std::make_unique<CBVariablesEffect<ConstantBufferEffectT>>(device, std::move(buffers));
					m_cbuffers.push_back(uptr.get());
					m_effect.m_components.push_back(std::move(uptr));
				}
			}

			static std::vector<std::string> _getNames(const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc, D3D_SHADER_INPUT_TYPE type);

			template<typename SamplersEffectT>
			void _addShaderSamplers(const CBVariableManager& variables, const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc)
			{
				std::vector<std::string> samplerNames = _getNames(shaderRefl, shaderDesc, D3D_SIT_SAMPLER);
				if (!samplerNames.empty())
				{
					auto uptr = std::make_unique<SamplersEffectT>();
					uptr->m_buffers.reserve(samplerNames.size());
					for (size_t i = 0; i < samplerNames.size(); ++i)
						if (!samplerNames[i].empty())
							uptr->SetResource(static_cast<UINT>(i), variables.GetSampler(samplerNames[i]));
					m_effect.m_components.push_back(std::move(uptr));
				}
			}

			template<typename ShaderResourcesEffectT>
			void _addShaderTextures(const CBVariableManager& variables, const dx_ptr<ID3D11ShaderReflection>& shaderRefl, const D3D11_SHADER_DESC& shaderDesc)
			{
				std::vector<std::string> textureNames = _getNames(shaderRefl, shaderDesc, D3D_SIT_TEXTURE);
				if (!textureNames.empty())
				{
					auto uptr = std::make_unique<ShaderResourcesEffectT>();
					uptr->m_buffers.reserve(textureNames.size());
					for (size_t i = 0; i < textureNames.size(); ++i)
						if (!textureNames[i].empty())
							uptr->SetResource(static_cast<UINT>(i), variables.GetTexture(textureNames[i]));
					m_effect.m_components.push_back(std::move(uptr));
				}
			}

			DynamicEffect m_effect;
			InputLayoutManager* m_layouts;
			std::vector<const Model*> m_models;
			std::vector<ICBVariablesEffect*> m_cbuffers;
			size_t m_vsSignatureID;
		};
	}
}
