#pragma once

#include <d3d11.h>
#include <D3DCompiler.h>
#include <set>
#include <vector>
#include <initializer_list>
#include <algorithm>

bool operator==(const D3D11_SIGNATURE_PARAMETER_DESC& left, const D3D11_SIGNATURE_PARAMETER_DESC& right);
bool operator<(const D3D11_SIGNATURE_PARAMETER_DESC& left, const D3D11_SIGNATURE_PARAMETER_DESC& right);

bool operator==(const D3D11_INPUT_ELEMENT_DESC& left, const D3D11_INPUT_ELEMENT_DESC& right);
bool operator<(const D3D11_INPUT_ELEMENT_DESC& left, const D3D11_INPUT_ELEMENT_DESC& right);

namespace mini
{
	//Basicly I need a sorted contiguous immutable (after creation, ony entire set can be replaced) container
	//and I don't want boost dependencies in the project
	template <class InputElemDesc, class LessThan = std::less<InputElemDesc>,
			  class Equal = std::equal_to<InputElemDesc>>
	class InputElements
	{
	public:
		typedef typename std::vector<InputElemDesc>::size_type size_type;
		typedef typename std::vector<InputElemDesc>::const_iterator iterator_type;

		template<class iter>
		InputElements(iter b, iter e)
			: m_elements(b, e)
		{
			sort();
		}

		template<class iter, class F>
		InputElements(iter b, iter e, F&& f)
			: m_elements(b, e)
		{
			for (auto& el : m_elements)
				f(el);
			sort();
		}

		template<int N>
		InputElements(const InputElemDesc(&elements)[N])
			: InputElements(elements, elements + N)
		{}

		InputElements(const std::vector<InputElemDesc>& elements)
			: InputElements(elements.begin(), elements.end())
		{ }

		InputElements(std::vector<InputElemDesc>&& elements)
			: m_elements(move(elements))
		{
			sort();
		}

		InputElements(std::initializer_list<InputElemDesc> elements)
			: InputElements(elements.begin(), elements.end())
		{ }

		InputElements(InputElements&& other) = default;
		InputElements(const InputElements& other) = default;

		InputElements& operator=(InputElements&& other) = default;
		InputElements& operator=(const InputElements& other) = default;

		template<int N>
		InputElements& operator=(InputElemDesc(&elements)[N])
		{
			assign(elements, elements + N);
			return *this;
		}

		InputElements& operator=(std::initializer_list<InputElemDesc> elements)
		{
			assign(elements.begin(), elements.end());
			return *this;
		}

		InputElements& operator=(std::vector<InputElemDesc>&& elements)
		{
			assign(std::move(elements));
			return *this;
		}

		InputElements& operator=(const std::vector<InputElemDesc>& elements)
		{
			assign(elements.begin(), elements.end());
			return *this;
		}

		template<class iter>
		void assign(iter begin, iter end)
		{
			m_elements.assign(begin, end);
			sort();
		}

		template<int N>
		void asign(InputElemDesc(&elements)[N])
		{
			assign(elements, elements + N);
		}

		void assign(std::initializer_list<InputElemDesc> elements)
		{
			assign(elements.begin(), elements.end());
		}

		void assign(const std::vector<InputElemDesc>& elements)
		{
			assign(elements.begin(), elements.end());
		}

		void assign(std::vector<InputElemDesc>&& elements)
		{
			m_elements = std::move(elements);
			sort();
		}

		friend bool operator<(const InputElements& left, const InputElements& right)
		{
			return std::lexicographical_compare(left.m_elements.begin(), left.m_elements.end(),
				right.m_elements.begin(), right.m_elements.end(), LessThan{});
		}
		friend bool operator ==(const InputElements& left, const InputElements& right)
		{
			return std::equal(left.m_elements.begin(), left.m_elements.end(),
				right.m_elements.begin(), right.m_elements.end(), Equal{});
		}

		
		size_type size() const { return m_elements.size(); }
		bool empty() const { return m_elements.empty(); }

		const InputElemDesc& operator[](size_type idx) const
		{
			return m_elements[idx];
		}

		const InputElemDesc* data() const { return m_elements.data(); }

		iterator_type begin() const { return m_elements.begin(); }
		iterator_type end() const { return m_elements.end(); }

	private:
		std::vector<InputElemDesc> m_elements;
		void sort()
		{
			std::sort(m_elements.begin(), m_elements.end(), LessThan{});
		}
	};


	typedef InputElements<D3D11_INPUT_ELEMENT_DESC> VertexAttributes;

	typedef InputElements<D3D11_SIGNATURE_PARAMETER_DESC> InputSignature;
}