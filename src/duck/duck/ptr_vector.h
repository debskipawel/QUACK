#pragma once

#include "compressed_pair.h"
#include <stdexcept>
#include <memory>
#include <vector>
#include <algorithm>
#include <utility>

namespace mini
{
	/**
	* Smart vector of pointers that owns and manages objects through the pointers contained within it
	* and disposes of them when necessary.
	*
	* Acts similarly to to a vector of unique_ptr-s. The main difference is that it stores addresses
	* in a contiguous array of regular pointers, which helps to interop with API's that require
	* arrays of pointers as input. A vector of unique_ptr-s could be made to work in this context
	* but only if the size of unique_ptr and a regular pointer are the same. This, however, cannot
	* be guaranteed, and is impossible if the Deleter is not an empty class. In this collection the
	* problem is avoided by storing only one deleter for all contained objects.
	*
	* @tparam T      Type of object managed by the vector
	* @tparam _Dx	 Deleter used to dispose object. It should provide a function call operator accepting
	*				 T* which frees all resources of the object referenced by the pointer. Defaults to
	*				 std::default_delete<T>.
	* @tparam _Alloc Allocator used to aquire/release memory for the array of pointers
	*				 (but NOT the underlying objects). Defaults to std::allocator<T*>
	*/
	template<class T, class _Dx = std::default_delete<T>, class _Alloc = std::allocator<T*>>
	class ptr_vector
	{
	public:
		class ptr_ref;
		class ptr_iterator;

		using value_type = T*;
		using allocator_type = _Alloc;
		using vector_type = std::vector<value_type, allocator_type>;
		using size_type = typename vector_type::size_type;
		using difference_type = typename vector_type::difference_type;
		using reference = ptr_ref;
		using const_reference = typename vector_type::const_reference;
		using pointer = typename vector_type::pointer;
		using const_pointer = typename vector_type::const_pointer;
		using iterator = ptr_iterator;
		using const_iterator = typename vector_type::const_iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = typename vector_type::const_reverse_iterator;
		using deleter_type = _Dx;
		using unique_ptr_type = std::unique_ptr<T, deleter_type>;

	private:
		using vector_iterator = typename vector_type::iterator;
		using deleter_const_reference = typename std::conditional<std::is_rvalue_reference<deleter_type>::value,
				typename std::add_rvalue_reference<typename std::add_const<typename std::remove_reference<deleter_type>::type>::type>::type,
				typename std::conditional<std::is_reference<deleter_type>::value,
					typename std::add_lvalue_reference<typename std::add_const<typename std::remove_reference<deleter_type>>::type>::type,
					const deleter_type&
				>::type
			>::type;
		using deleter_reference = typename std::conditional<std::is_reference<deleter_type>::value, deleter_type, const deleter_type&>::type;

	public:

		class ptr_ref
		{
		public:

			ptr_ref(ptr_ref&& other) noexcept
				: m_myPair(std::move(other.m_myPair))
			{ }

			ptr_ref& operator=(value_type ptr)
			{
				ptr_vector::_reset(get_deleter(), _element(), ptr);
				return *this;
			}

			ptr_ref& operator=(ptr_ref&& other) noexcept
			{
				swap(other);
				return *this;
			}

			ptr_ref& operator=(unique_ptr_type&& uptr) noexcept
			{
				swap(uptr);
				return *this;
			}

			ptr_ref& operator=(const ptr_ref& other) = delete;

			value_type get() const { return *_element(); }

			operator value_type() const { return get(); }

			T& operator*() const { return *get(); }

			value_type operator->() const { return get(); }

			void swap(ptr_ref& other) noexcept
			{
				using std::swap;
				swap(*_element(), *other._element());
			}

			void swap(unique_ptr_type& uptr) noexcept
			{
				value_type other = uptr.release();
				uptr.reset(*_element());
				*_element() = other;
			}

		private:
			friend class ptr_vector;
			friend class ptr_iterator;

			const vector_iterator& _element() const { return m_myPair.second(); }

			vector_iterator& _element() { return m_myPair.second(); }

			deleter_reference get_deleter() { return m_myPair.first(); }

			deleter_const_reference get_deleter() const { return m_myPair.first(); }

			ptr_ref(ptr_vector& container, vector_iterator element)
				: m_myPair(container.get_deleter(), element)
			{ }

			ptr_ref(deleter_reference dt, vector_iterator element)
				: m_myPair(dt, element)
			{ }

			compressed_pair<deleter_type, vector_iterator> m_myPair;
		};

		class ptr_iterator
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = value_type;
			using difference_type = difference_type;
			using pointer = pointer;
			using reference = ptr_ref;

			ptr_iterator() = default;
			ptr_iterator(ptr_iterator&& other) = default;
			ptr_iterator(const ptr_iterator& other) = default;

			ptr_iterator& operator=(ptr_iterator&& other) = default;
			ptr_iterator& operator=(const ptr_iterator& other) = default;

			void swap(ptr_iterator& other) noexcept { swap(m_myPair, other.m_myPair); }

			ptr_ref operator*() const { return ptr_ref{ get_deleter(), _element() }; }

			pointer operator->() const { return std::pointer_traits<pointer>::pointer_to(*_element()); }

			ptr_iterator& operator++()
			{
				advance(1);
				return *this;
			}

			ptr_iterator operator++(int)
			{
				ptr_iterator copy = *this;
				advance(1);
				return copy;
			}

			ptr_iterator& operator--()
			{
				advance(-1);
				return *this;
			}

			ptr_iterator operator--(int)
			{
				ptr_iterator copy = *this;
				advance(-1);
				return copy;
			}

			ptr_iterator& operator+=(difference_type n)
			{
				advance(n);
				return *this;
			}

			ptr_iterator operator+(difference_type n) const
			{
				return ptr_iterator{ get_deleter(), _element() + n };
			}

			friend ptr_iterator operator+(difference_type n, const ptr_iterator& x)
			{
				return x + n;
			}

			ptr_iterator& operator-=(difference_type n)
			{
				advance(-n);
				return *this;
			}

			ptr_iterator operator-(difference_type n) const
			{
				return ptr_iterator{ get_deleter(), _element() - n };
			}

			difference_type operator-(const ptr_iterator& other) const
			{
				return _element() - other._element();
			}

			ptr_ref operator[](difference_type n)
			{
				return ptr_ref{ get_deleter(), _element() + n };
			}

			bool operator<(const ptr_iterator& other) const
			{
				return _element() < other._element();
			}

			bool operator>=(const ptr_iterator& other) const
			{
				return other < *this;
			}

			bool operator>(const ptr_iterator& other) const
			{
				return _element() > other._element();
			}

			bool operator<=(const ptr_iterator& other) const
			{
				return other > *this;
			}

			bool operator==(const ptr_iterator& other) const
			{
				return equals(other);
			}

			bool operator!=(const ptr_iterator& other) const
			{
				return !equals(other);
			}

			void advance(difference_type n) { _element() += n; }

			bool equals(const ptr_iterator& other) const { return _element() == other._element(); }

			operator const_iterator() const { return _element(); }

		private:
			friend class ptr_vector;

			const vector_iterator& _element() const { return m_myPair.second(); }

			vector_iterator& _element() { return m_myPair.second(); }

			deleter_reference get_deleter() { return m_myPair.first(); }

			deleter_const_reference get_deleter() const { return m_myPair.first(); }

			ptr_iterator(ptr_vector& container, vector_iterator element)
				: m_myPair(container.get_deleter(), element)
			{ }

			ptr_iterator(deleter_reference dt, vector_iterator element)
				: m_myPair(dt, element)
			{ }

			compressed_pair<deleter_type, vector_iterator> m_myPair;
		};

		ptr_vector()
			: m_myPair(deleter_type{}, vector_type(0))
		{}

		//SFINAE seems borked

		explicit ptr_vector(const allocator_type& alloc,
			typename std::enable_if<std::is_default_constructible<deleter_type>::value, int>::type = 0)
			: m_myPair(deleter_type{}, vector_type{ alloc })
		{ }

		explicit ptr_vector(deleter_reference dt)
			: m_myPair(dt, vector_type{})
		{ }

		ptr_vector(const allocator_type& alloc, deleter_reference dt)
			: m_myPair(dt, vector_type{ alloc })
		{ }

		ptr_vector(size_type count, const allocator_type& alloc = allocator_type{},
			typename std::enable_if<std::is_default_constructible<deleter_type>::value, int>::type = 0)
			: m_myPair(deleter_type{}, vector_type{ count, nullptr, alloc })
		{ }

		ptr_vector(size_type count, deleter_reference dt)
			: m_myPair(dt, vector_type{ count, nullptr })
		{ }

		ptr_vector(size_type count, const allocator_type& alloc, deleter_reference dt)
			: m_myPair(dt, vector_type{ count, nullptr, alloc })
		{ }

		template<typename InputIt, typename std::enable_if<std::is_default_constructible<deleter_type>::value && std::is_constructible<value_type, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		ptr_vector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type{})
			: m_myPair(deleter_type{}, vector_type{ first, last, alloc })
		{ }

		template<typename InputIt, typename std::enable_if<std::is_constructible<value_type, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		ptr_vector(InputIt first, InputIt last, deleter_reference dt)
			: m_myPair(dt, vector_type{ first, last })
		{ }

		template<typename InputIt, typename std::enable_if<std::is_constructible<value_type, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		ptr_vector(InputIt first, InputIt last, const allocator_type& alloc, deleter_reference dt)
			: m_myPair(dt, vector_type{ first, last, alloc })
		{ }

		template<typename InputIt, typename std::enable_if<std::is_default_constructible<deleter_type>::value && std::is_same<unique_ptr_type&&, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		ptr_vector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type{})
			: m_myPair(deleter_type{}, vector_type( std::distance(first, last), nullptr, alloc ))
		{
			_takeOver(_myVector().begin(), first, last);
		}

		template<typename InputIt, typename std::enable_if<std::is_same<unique_ptr_type&&, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		ptr_vector(InputIt first, InputIt last, deleter_reference dt)
			: m_myPair(dt, vector_type{ std::distance(first, last), nullptr})
		{
			_takeOver(_myVector().begin(), first, last);
		}

		template<typename InputIt, typename std::enable_if<std::is_same<unique_ptr_type&&, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		ptr_vector(InputIt first, InputIt last, const allocator_type& alloc, deleter_reference dt)
			: m_myPair(dt, vector_type{ std::distance(first, last), nullptr, alloc })
		{
			_takeOver(_myVector().begin(), first, last);
		}

		//assumes ownership of all pointers
		ptr_vector(std::initializer_list<value_type> init, const allocator_type& alloc = allocator_type{},
			typename std::enable_if<std::is_default_constructible<deleter_type>::value, int>::type = 0)
			: m_myPair(deleter_type{}, vector_type{ init, alloc })
		{ }

		//assumes ownership of all pointers
		ptr_vector(std::initializer_list<value_type> init, deleter_reference dt)
			: m_myPair(dt, vector_type{ init })
		{ }

		//assumes ownership of all pointers
		ptr_vector(std::initializer_list<value_type> init, const allocator_type& alloc, deleter_reference dt)
			: m_myPair(dt, vector_type{ init, alloc })
		{ }

		ptr_vector(const ptr_vector& other) = delete;

		ptr_vector(ptr_vector&& other) = default;

		ptr_vector(ptr_vector&& other, const allocator_type& alloc)
			: m_myPair(std::move(other.get_deleter()), vector_type(std::move(other._myVector()), alloc))
		{ }

		~ptr_vector()
		{
			_delete();
		}

		ptr_vector& operator=(ptr_vector&& other) noexcept
		{
			_myVector() = std::move(other._myVector());
			return *this;
		}
		ptr_vector& operator=(const ptr_vector& other) = delete;

		// assumes ownership of all pointers
		ptr_vector& operator=(std::initializer_list<value_type> ilist)
		{
			assign(ilist);
			return *this;
		}

		// assumes ownership of all pointers
		template<typename InputIt, typename std::enable_if<std::is_assignable<value_type, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		void assign(InputIt first, InputIt last)
		{
			_delete();
			_myVector().assign(first, last);
		}

		// assumes ownership of all pointers
		template<typename InputIt, typename std::enable_if<std::is_same<unique_ptr_type&&, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		void assign(InputIt first, InputIt last)
		{
			_delete();
			_myVector().resize(std::distance(first, last), nullptr);
			_takeOver(_myVector().begin(), first, last);
		}

		// assumes ownership of all pointers
		void assign(std::initializer_list<value_type> ilist)
		{
			_delete();
			_myVector() = ilist;
		}

		allocator_type get_allocator() const { return _myVector().get_allocator(); }

		reference at(size_type pos)
		{
			if (size() <= pos)
				throw std::out_of_range("invalid ptr_vector<T> subscript");
			return reference{ *this, _myVector().begin() + pos };
		}

		const_reference at(size_type pos) const { return _myVector().at(pos); }

		reference operator[](size_type pos) { return reference{ *this, _myVector().begin() + pos }; }

		const_reference operator[](size_type pos) const { return _myVector()[pos]; }

		reference front() { return reference{ *this, _myVector().begin() }; }

		const_reference front() const { return _myVector().front(); }

		reference back() { return reference{ *this, --_myVector().end() }; }

		const_reference back() const { return _myVector().back(); }

		pointer data() { return _myVector().data(); }

		const_pointer data() const { return _myVector().data(); }

		iterator begin() { return iterator{ *this, _myVector().begin() }; }

		const_iterator begin() const { return _myVector().begin(); }

		const_iterator cbegin() const { return begin(); }

		iterator end() { return iterator{ *this, _myVector().end() }; }

		const_iterator end() const { return _myVector().end(); }

		const_iterator cend() const { return end(); }

		reverse_iterator rbegin() { return reverse_iterator(end()); }

		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

		const_reverse_iterator crbegin() const { return rbegin(); }

		reverse_iterator rend() { return reverse_iterator(begin()); }

		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

		const_reverse_iterator crend() const { return rend(); }

		bool empty() const { return _myVector().empty(); }

		size_type size() const { return _myVector().size(); }

		size_type max_size() const { return _myVector().max_size(); }

		void reserve(size_type new_cap) { _myVector().reserve(new_cap); }

		size_type capacity() const { return _myVector().capacity(); }

		void shrink_to_fit() { return _myVector().shrink_to_fit(); }

		void clear()
		{
			_delete();
			_myVector().clear();
		}

		//assumes ownership of the pointer
		iterator insert(const_iterator pos, const_reference value)
		{
			return iterator{ *this, _myVector().insert(pos, value) };
		}

		//assumes ownership of the pointer
		iterator insert(const_iterator pos, value_type&& value)
		{
			return iterator{ *this, _myVector().insert(pos, std::move(value)) };
		}

		//assumes ownership of all pointers
		template<typename InputIt, typename std::enable_if<std::is_assignable<value_type, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			return iterator{ *this, _myVector().insert(pos, first, last) };
		}

		//assumes ownership of all pointers
		template<typename InputIt, typename std::enable_if<std::is_same<unique_ptr_type&&, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			auto result = iterator{ *this, _myVector().insert(pos, std::distance(first, last), nullptr) };
			_takeOver(pos, first, last);
			return result;
		}

		//assumes ownership of all pointers
		iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
		{
			return iterator{ *this, _myVector().insert(pos, ilist) };
		}

		//assumes ownership of the pointer
		template<typename... Args>
		iterator emplace(const_iterator pos, Args... args)
		{
			return iterator{ *this, _myVector().emplace(pos, std::forward<Args>(args)...) };
		}

		iterator erase(const_iterator pos)
		{
			_delete(pos, _myVector().end());
			return iterator{ *this, _myVector().erase(pos) };
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			_delete(first, last);
			return iterator{ *this, _myVector().erase(first, last) };
		}

		//assumes ownership of the pointer
		void push_back(const_reference val)
		{
			_myVector().push_back(val);
		}

		//assumes ownership of the pointer
		void push_back(value_type&& val)
		{
			_myVector().push_back(val);
		}

		void push_back(unique_ptr_type&& val)
		{
			_myVector().push_back(val.get());
			val.release();
		}

		template<typename... Args>
		reference emplace_back(Args... args)
		{
			_myVector().emplace_back(std::forward<Args>(args)...);
			return back();
		}

		void pop_back()
		{
			_delete(_myVector().back());
			_myVector().pop_back();
		}

		void resize(size_type count)
		{
			if (size() > count)
				_delete(_myVector().begin() + count, _myVector().end());
			_myVector().resize(count, nullptr);
		}

		void swap(ptr_vector& other) noexcept
		{
			using std::swap;
			swap(m_myPair, other.m_myPair);
		}

		deleter_reference get_deleter() { return m_myPair.first(); }

		deleter_const_reference get_deleter() const { return m_myPair.first(); }

		template<typename OtherT, typename OtherDx, typename OtherAlloc>
		friend bool operator==(const ptr_vector& x, const ptr_vector&y);

	private:
		friend class ptr_ref;

		template <typename InputIt, typename std::enable_if<std::is_default_constructible<_Dx>::value && std::is_same<std::unique_ptr<T, _Dx>&&, decltype(*std::declval<InputIt>())>::value, int>::type = 0>
		void _takeOver(vector_iterator pos, InputIt first, InputIt last)
		{
			std::transform(first, last, pos, [](unique_ptr_type&& ptr) { return ptr.release(); });
		}

		static value_type _release(vector_iterator it)
		{
			value_type ptr = *it;
			*it = nullptr;
			return ptr;
		}

		static void _reset(deleter_const_reference dt, vector_iterator it, value_type ptr = nullptr)
		{
			if (ptr == *it)
				return;
			_delete(dt, *it);
			*it = ptr;
		}

		void _reset(vector_iterator it, value_type ptr = nullptr)
		{
			_reset(get_deleter(), it, ptr);
		}

		static void _delete(deleter_const_reference dt, const_reference ptr)
		{
			if (ptr) dt(ptr);
		}

		void _delete(const_reference ptr) const { get_deleter()(ptr); }

		void _delete(const_iterator it) const { _delete(*it); }

		void _delete(const_iterator first, const_iterator last) const
		{
			std::for_each(first, last, [this](value_type ptr) {_delete(ptr); });
		}

		void _delete() const
		{
			_delete(_myVector().begin(), _myVector().end());
		}

		const vector_type& _myVector() const { return m_myPair.second(); }

		vector_type& _myVector() { return m_myPair.second(); }

		compressed_pair<deleter_type, vector_type> m_myPair;
	};

	template<class T, class _Dx, class _Alloc>
	void swap(typename ptr_vector<T, _Dx, _Alloc>::ptr_ref& x, typename ptr_vector<T, _Dx, _Alloc>::ptr_ref& y) noexcept
	{
		x.swap(y);
	}

	template<class T, class _Dx, class _Alloc>
	void swap(typename ptr_vector<T, _Dx, _Alloc>::iterator& x, typename ptr_vector<T, _Dx, _Alloc>::iterator& y) noexcept
	{
		x.swap(y);
	}

	template<class T, class _Dx, class _Alloc>
	void swap(ptr_vector<T, _Dx, _Alloc>& x, ptr_vector<T, _Dx, _Alloc>& y) noexcept
	{
		x.swap(y);
	}

	template<class T, class _Dx, class _Alloc>
	bool operator==(const ptr_vector<T, _Dx, _Alloc>& x, const ptr_vector<T, _Dx, _Alloc>& y) noexcept
	{
		return x._myVector() == y._myVector();
	}

	template<class T, class _Dx, class _Alloc>
	bool operator!=(const ptr_vector<T, _Dx, _Alloc>& x, const ptr_vector<T, _Dx, _Alloc>& y) noexcept
	{
		return !(x == y);
	}	
}
