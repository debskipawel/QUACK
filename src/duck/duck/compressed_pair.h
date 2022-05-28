#pragma once
#include <type_traits>

namespace mini
{
	template<typename T1, typename T2, bool = std::is_empty<T1>::value && !std::is_final<T1>::value>
	class compressed_pair
	{
	public:
		using first_type = T1;
		using second_type = T2;
		using first_const_reference = typename std::add_lvalue_reference<typename std::add_const<first_type>::type>::type;
		using second_const_reference = typename std::add_lvalue_reference<typename std::add_const<second_type>::type>::type;
		using first_reference = typename std::add_lvalue_reference<first_type>::type;
		using second_reference = typename std::add_lvalue_reference<second_type>::type;
		using first_rvalue_reference = std::add_rvalue_reference<first_type>;
		using second_rvalue_reference = std::add_rvalue_reference<second_type>;

		template<typename std::enable_if<std::is_default_constructible<T1>::value &&
			std::is_default_constructible<T2>::value, int>::type = 0>
			compressed_pair() { }

		template<typename OtherT1, typename OtherT2,
			typename std::enable_if<std::is_constructible<T1, OtherT1>::value &&
			std::is_constructible<T2, OtherT2>::value, int>::type = 0>
			compressed_pair(OtherT1 first, OtherT2 second)
			: m_first(std::forward<OtherT1>(first)), m_second(std::forward<OtherT2>(second))
		{ }

		template<typename OtherT1, typename OtherT2,
			typename std::enable_if<std::is_constructible<T1, OtherT1>::value &&
			std::is_constructible<T2, OtherT2>::value, int>::type = 0>
			compressed_pair(const compressed_pair<OtherT1, OtherT2>& other)
			: m_first(std::forward<OtherT1>(other.first)), m_second(std::forward<OtherT2>(other.second))
		{ }

		compressed_pair(compressed_pair&& other) noexcept
			: m_first(std::move(other.m_first)), m_second(std::move(other.m_second))
		{ }

		template<typename OtherT1, typename OtherT2,
			typename std::enable_if<std::is_assignable<T1, OtherT1>::value &&
			std::is_assignable<T2, OtherT2>::value, int>::type = 0>
			compressed_pair& operator=(const compressed_pair<OtherT1, OtherT2>& other)
		{
			first() = std::forward<OtherT1>(other.m_first);
			second() = std::forward<OtherT2>(other.m_second);
			return *this;
		}

		compressed_pair& operator=(compressed_pair&& other) noexcept
		{
			first() = std::move(other.m_first);
			second() = std::move(other.m_second);
			return *this;
		}

		void swap(compressed_pair& other) noexcept
		{
			using std::swap;
			swap(first(), other.first());
			swap(second(), other.second());
		}

		first_reference first() { return m_first; }
		first_const_reference first() const { return m_first; }

		second_reference second() { return m_second; }
		first_const_reference second() const { return m_second; }

	private:
		T1 m_first;
		T2 m_second;
	};

	template<typename T1, typename T2>
	class compressed_pair<T1, T2, true> : private T1
	{
	public:
		using first_type = T1;
		using second_type = T2;
		using first_const_reference = typename std::add_lvalue_reference<typename std::add_const<first_type>::type>::type;
		using second_const_reference = typename std::add_lvalue_reference<typename std::add_const<second_type>::type>::type;
		using first_reference = typename std::add_lvalue_reference<first_type>::type;
		using second_reference = typename std::add_lvalue_reference<second_type>::type;
		using first_rvalue_reference = std::add_rvalue_reference<first_type>;
		using second_rvalue_reference = std::add_rvalue_reference<second_type>;

		template<typename std::enable_if<std::is_default_constructible<T1>::value &&
			std::is_default_constructible<T2>::value, int>::type = 0>
			compressed_pair() { }

		template<typename OtherT1, typename OtherT2,
			typename std::enable_if<std::is_constructible<T1, OtherT1>::value &&
			std::is_constructible<T2, OtherT2>::value, int>::type = 0>
			compressed_pair(OtherT1 first, OtherT2 second)
			: first_type(std::forward<OtherT1>(first)), m_second(std::forward<OtherT2>(second))
		{ }

		template<typename OtherT1, typename OtherT2,
			typename std::enable_if<std::is_constructible<T1, OtherT1>::value &&
			std::is_constructible<T2, OtherT2>::value, int>::type = 0>
			compressed_pair(const compressed_pair<OtherT1, OtherT2>& other)
			: first_type(std::forward<OtherT1>(other.first)), m_second(std::forward<OtherT2>(other.second))
		{ }

		compressed_pair(compressed_pair&& other) noexcept
			: first_type(std::move(other.first())), m_second(std::move(other.m_second))
		{ }

		template<typename OtherT1, typename OtherT2,
			typename std::enable_if<std::is_assignable<T1, OtherT1>::value &&
			std::is_assignable<T2, OtherT2>::value, int>::type = 0>
			compressed_pair& operator=(const compressed_pair<OtherT1, OtherT2>& other)
		{
			first() = std::forward<OtherT1>(other.first());
			second() = std::forward<OtherT2>(other.m_second);
			return *this;
		}

		compressed_pair& operator=(compressed_pair&& other) noexcept
		{
			first() = std::move(other.first());
			second() = std::move(other.m_second);
			return *this;
		}

		void swap(compressed_pair& other) noexcept
		{
			using std::swap;
			swap(first(), other.first());
			swap(second(), other.second());
		}

		first_reference first() { return *this; }
		first_const_reference first() const { return *this; }

		second_reference second() { return m_second; }
		second_const_reference second() const { return m_second; }
	private:
		T2 m_second;
	};

	template<typename T1, typename T2>
	inline void swap(compressed_pair<T1, T2>& x, compressed_pair<T1, T2>& y) noexcept
	{
		x.swap(y);
	}
}