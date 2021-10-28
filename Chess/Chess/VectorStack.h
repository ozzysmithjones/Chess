#pragma once
#include <iterator>
#include <vector>
#include <stack>
//#include <stdexcept>
#include <algorithm>

//#define MAX_DEPTH 3
//#define AVERAGE_MOVES_PER_TURN 45 //the highest is 218, the average is 35-40

template<typename T>
class VectorStack
{
public:

	VectorStack(size_t capacity);
	size_t Size();
	void PushBack(const T& item);
	void Erase(const T& item);
	void Pop();
	void Push();
	T& operator[](unsigned int index);

	template<typename Comp>
	void Sort(Comp comp)
	{
		std::sort(m_data.begin() + m_startStack.top(), m_data.end(), comp);
	}

	struct Iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;  // or also value_type*
		using reference = T&;  // or also value_type&

		Iterator(pointer ptr) : m_ptr(ptr) {}

		reference operator*() const { return *m_ptr; }
		pointer operator->() { return m_ptr; }

		// Prefix increment
		Iterator& operator++() { m_ptr++; return *this; }
		Iterator operator--() { m_ptr--; return *this; }

		// Postfix increment
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
		Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

	private:

		pointer m_ptr;
	};

	Iterator begin() { return Iterator(&(*m_data.begin()) + m_startStack.top()) }
	Iterator end() { return Iterator(&(*m_data.end()); }

private:

	std::stack<size_t> m_startStack;
	std::vector<T> m_data;
};

template<typename T>
inline VectorStack<T>::VectorStack(size_t capacity)
{
	m_startStack.push(0);
	m_data.reserve(capacity);
}

template<typename T>
inline size_t VectorStack<T>::Size()
{
	return (m_data.size() - m_startStack.top());
}

template<typename T>
inline void VectorStack<T>::PushBack(const T& item)
{
	m_data.push_back(item);
}

template<typename T>
inline void VectorStack<T>::Erase(const T& item)
{
	m_data.erase(std::remove(m_data.begin() + m_startStack.top(),m_data.end(),item);
}

template<typename T>
inline void VectorStack<T>::Pop()
{
	if (m_startStack.size() != 1u)
	{
		m_data.resize(m_startStack.top());
		m_startStack.pop();
	}
}

template<typename T>
inline void VectorStack<T>::Push()
{
	m_startStack.push(m_data.size());
}

template<typename T>
inline T& VectorStack<T>::operator[](unsigned int index)
{
	return m_data[m_startStack.top() + index];
}


