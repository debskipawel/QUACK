#pragma once
#include <cassert>
#include <memory>
#include <array>
#include <DirectXMath.h>
#include "imgui.h"
#include <string>

namespace mini
{
	namespace gk2
	{
		class ICBVariable
		{
		public:
			virtual ~ICBVariable() = default;
			virtual void copyTo(void* buffer, size_t bufferSize) const = 0;
		};

		namespace detail
		{
			constexpr size_t ByteAlignment = 4 * 4;

			template<typename T>
			using rem_cvref = std::remove_cv<typename std::remove_reference<T>::type>;

			template<typename T>
			using rem_cvref_t = typename rem_cvref<T>::type;

			template<typename T>
			using elem_t = rem_cvref_t<std::remove_extent_t<T>>;

			template<typename T, bool isArray = std::is_array<T>::value>
			struct is_continuous : std::bool_constant<(sizeof(elem_t<T>)&(ByteAlignment-1)) == 0 && is_continuous<elem_t<T>>::value> {};

			template<typename T>
			struct is_continuous<T, false> : std::true_type {};

			template<typename T, bool Continuous = is_continuous<T>::value>
			class CBVariableBase : public ICBVariable
			{
			public:
				using MyT = CBVariableBase<T, Continuous>;
				using ValueT = T;

				ValueT value;

				void copyTo(void* buffer, size_t bufferSize) const override
				{
					copyTo(buffer, value, bufferSize);
				}

				static size_t copyTo(void* buffer, const T& value, size_t bufferSize)
				{
					assert(bufferSize >= sizeof(T));
					memcpy(buffer, &value, sizeof(T));
					return sizeof(T);
				}

				MyT& operator=(const T& v)
				{
					memcpy(&value, &v, sizeof(T));
					return *this;
				}

				MyT& operator=(T&& v)
				{
					value = std::move(v);
					return *this;
				}
			};

			template<typename T>
			class CBVariableBase<T, false> : public ICBVariable
			{
			public:
				using ExtentT = elem_t<T>;
				using N = std::extent<T>;
				using ElemT = typename CBVariableBase<ExtentT>::ValueT;
				using ValueT = ElemT[N::value];

				ValueT value;

				using MyT = CBVariableBase<T, false>;

				void copyTo(void* buffer, size_t bufferSize) const override
				{
					copyTo(buffer, value, bufferSize);
				}

				static size_t copyTo(void* buffer, const ValueT& value, size_t bufferSize)
				{
					size_t offset = 0;
					for (const ElemT& val : value)
					{
						assert(offset < bufferSize);
						offset += CBVariableBase<ExtentT>::copyTo(reinterpret_cast<unsigned char*>(buffer) + offset, val, bufferSize - offset);
						if (offset & (ByteAlignment-1))
							offset = (offset & ~(ByteAlignment - 1)) + ByteAlignment;
					}
					return offset;
				}

				MyT& operator=(const ExtentT (&v)[N::value])
				{
					std::copy_n(v, N::value, value);
					return *this;
				}

				MyT& operator=(ExtentT (&&v)[N::value])
				{
					std::copy_n(std::make_move_iterator(std::begin(v)), N::value, value.begin());
					//value = std::move(v);
					return *this;
				}
			};

			inline void guiUpdate(const std::string& label, DirectX::XMFLOAT3& value, float min, float max, float step)
			{
				ImGui::DragFloat3(label.c_str(), &value.x, step, min, max);
			}

			inline void guiUpdate(const std::string& label, DirectX::XMFLOAT4& value, float min, float max, float step)
			{
				ImGui::DragFloat4(label.c_str(), &value.x, step, min, max);
			}

			inline void guiUpdate(const std::string& label, float& value, float min, float max, float step)
			{
				ImGui::DragFloat(label.c_str(), &value, step, min, max);
			}

			template<typename T, size_t N>
			inline void guiUpdate(const std::string& label, T (&value)[N], float min, float max, float step)
			{
				for (size_t i = 0; i < N; ++i)
					guiUpdate(label + "[" + std::to_string(i) + "]", value[i], min, max, step);
			}
		}

		template<typename T>
		class CBVariable : public detail::CBVariableBase<detail::rem_cvref_t<T>>
		{
		public:
			using MyBase = detail::CBVariableBase<detail::rem_cvref_t<T>>;
			using MyBase::operator=;
		};

		class IGUIVariable
		{
		public:
			virtual ~IGUIVariable() = default;
			virtual void Update() = 0;
		};

		template<typename T>
		class GUIVariable : public CBVariable<T>, public IGUIVariable
		{
		public:
			using CBVariable<T>::operator=;

			explicit GUIVariable(const std::string name, float minVal = 0, float maxVal = 0, float step = 0.01f)
				: m_name(name), m_min(minVal), m_max(maxVal), m_step(step) { }

			void Update() override
			{
				detail::guiUpdate(m_name, value, m_min, m_max, m_step);
			}

		protected:
			std::string m_name;
			float m_min, m_max, m_step;
		};

		class ColorVariable : public GUIVariable<DirectX::XMFLOAT3>
		{
		public:
			using GUIVariable<DirectX::XMFLOAT3>::operator=;
			explicit ColorVariable(const std::string& name)
				: GUIVariable(name, 0, 1)
			{
			}

			void Update() override
			{
				ImGui::ColorEdit3(m_name.c_str(), &value.x);
			}
		};

		template<size_t NElems>
		class ColorsVariable : public GUIVariable<DirectX::XMFLOAT3[NElems]>
		{
		public:
			using MyBase = GUIVariable<DirectX::XMFLOAT3[NElems]>;
			using MyBase::operator=;

			explicit ColorsVariable(const std::string& name)
				: GUIVariable(name, 0, 1)
			{
			}
			
			void Update() override
			{
				for (size_t i = 0; i < NElems; ++i)
					ImGui::ColorEdit3((m_name + "[" + std::to_string(i) + "]").c_str(), &value[i].x);
			}
		};
	}
}
