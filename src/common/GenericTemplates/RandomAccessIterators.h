// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __RANDOMACCESSITERATORS_H__
#define __RANDOMACCESSITERATORS_H__

template<class T>
class RandomAccessIteratorWrapper : public std::iterator<std::random_access_iterator_tag, T>
{
protected:
	explicit RandomAccessIteratorWrapper(typename std::vector<T>::iterator iterArray);

public:
	RandomAccessIteratorWrapper();
	~RandomAccessIteratorWrapper();

	RandomAccessIteratorWrapper& operator+=(const int& rhs);
	RandomAccessIteratorWrapper& operator-=(const int& rhs);
	T& operator*();
	T* operator->();
	T& operator[](const int& rhs) const;

	RandomAccessIteratorWrapper& operator++();
	RandomAccessIteratorWrapper& operator--();
	RandomAccessIteratorWrapper operator++(int);
	RandomAccessIteratorWrapper operator--(int);
	RandomAccessIteratorWrapper operator+(const int& rhs) const;
	RandomAccessIteratorWrapper operator-(const int& rhs) const;

	bool operator==(const RandomAccessIteratorWrapper& rhs) const;
	bool operator!=(const RandomAccessIteratorWrapper& rhs) const;
	bool operator>(const RandomAccessIteratorWrapper& rhs) const;
	bool operator<(const RandomAccessIteratorWrapper& rhs) const;
	bool operator>=(const RandomAccessIteratorWrapper& rhs) const;
	bool operator<=(const RandomAccessIteratorWrapper& rhs) const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	typename std::vector<T>::iterator m_iterArray;
#pragma warning(pop)

	template<class T>
	friend class RandomAccessConstIteratorWrapper;
	template<class T>
	friend class RandomAccessContainerWrapper;
};

template<class T>
class RandomAccessConstIteratorWrapper : public std::iterator<std::random_access_iterator_tag, T>
{
protected:
	explicit RandomAccessConstIteratorWrapper(typename std::vector<T>::const_iterator iterArray);

public:
	RandomAccessConstIteratorWrapper();
	RandomAccessConstIteratorWrapper(const RandomAccessIteratorWrapper<T>& rIterator);
	~RandomAccessConstIteratorWrapper();

	RandomAccessConstIteratorWrapper& operator=(const RandomAccessIteratorWrapper<T>& rIterator);

	RandomAccessConstIteratorWrapper& operator+=(const int& rhs);
	RandomAccessConstIteratorWrapper& operator-=(const int& rhs);
	const T& operator*();
	const T* operator->();
	const T& operator[](const int& rhs) const;

	RandomAccessConstIteratorWrapper& operator++();
	RandomAccessConstIteratorWrapper& operator--();
	RandomAccessConstIteratorWrapper operator++(int);
	RandomAccessConstIteratorWrapper operator--(int);
	RandomAccessConstIteratorWrapper operator+(const int& rhs) const;
	RandomAccessConstIteratorWrapper operator-(const int& rhs) const;

	bool operator==(const RandomAccessConstIteratorWrapper& rhs) const;
	bool operator!=(const RandomAccessConstIteratorWrapper& rhs) const;
	bool operator>(const RandomAccessConstIteratorWrapper& rhs) const;
	bool operator<(const RandomAccessConstIteratorWrapper& rhs) const;
	bool operator>=(const RandomAccessConstIteratorWrapper& rhs) const;
	bool operator<=(const RandomAccessConstIteratorWrapper& rhs) const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	typename std::vector<T>::const_iterator m_iterArray;
#pragma warning(pop)

	template<class T>
	friend class RandomAccessContainerWrapper;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// class RandomAccessIteratorWrapper

template<class T>
RandomAccessIteratorWrapper<T>::RandomAccessIteratorWrapper(typename std::vector<T>::iterator iterArray) :
	m_iterArray(iterArray)
{
}

template<class T>
RandomAccessIteratorWrapper<T>::RandomAccessIteratorWrapper()
{
}

template<class T>
RandomAccessIteratorWrapper<T>::~RandomAccessIteratorWrapper()
{
}

template<class T>
RandomAccessIteratorWrapper<T>& RandomAccessIteratorWrapper<T>::operator+=(const int& rhs)
{
	m_iterArray += rhs; return *this;
}

template<class T>
bool RandomAccessIteratorWrapper<T>::operator<=(const RandomAccessIteratorWrapper& rhs) const
{
	return m_iterArray <= rhs.m_iterArray;
}

template<class T>
bool RandomAccessIteratorWrapper<T>::operator<(const RandomAccessIteratorWrapper& rhs) const
{
	return m_iterArray < rhs.m_iterArray;
}

template<class T>
bool RandomAccessIteratorWrapper<T>::operator!=(const RandomAccessIteratorWrapper& rhs) const
{
	return m_iterArray != rhs.m_iterArray;
}

template<class T>
bool RandomAccessIteratorWrapper<T>::operator==(const RandomAccessIteratorWrapper& rhs) const
{
	return m_iterArray == rhs.m_iterArray;
}

template<class T>
RandomAccessIteratorWrapper<T> RandomAccessIteratorWrapper<T>::operator--(int)
{
	RandomAccessIteratorWrapper tmp(*this); --m_iterArray; return tmp;
}

template<class T>
RandomAccessIteratorWrapper<T>& RandomAccessIteratorWrapper<T>::operator--()
{
	--m_iterArray; return *this;
}

template<class T>
RandomAccessIteratorWrapper<T>& RandomAccessIteratorWrapper<T>::operator-=(const int& rhs)
{
	m_iterArray -= rhs; return *this;
}

template<class T>
T& RandomAccessIteratorWrapper<T>::operator*()
{
	return *m_iterArray;
}

template<class T>
T* RandomAccessIteratorWrapper<T>::operator->()
{
	return &(*m_iterArray);
}

template<class T>
T& RandomAccessIteratorWrapper<T>::operator[](const int& rhs) const
{
	return m_iterArray[rhs];
}

template<class T>
RandomAccessIteratorWrapper<T> RandomAccessIteratorWrapper<T>::operator++(int)
{
	RandomAccessIteratorWrapper tmp(*this); ++m_iterArray; return tmp;
}

template<class T>
RandomAccessIteratorWrapper<T>& RandomAccessIteratorWrapper<T>::operator++()
{
	++m_iterArray; return *this;
}

template<class T>
RandomAccessIteratorWrapper<T> RandomAccessIteratorWrapper<T>::operator+(const int& rhs) const
{
	return RandomAccessIteratorWrapper(m_iterArray + rhs);
}

template<class T>
RandomAccessIteratorWrapper<T> RandomAccessIteratorWrapper<T>::operator-(const int& rhs) const
{
	return RandomAccessIteratorWrapper(m_iterArray - rhs);
}

template<class T>
bool RandomAccessIteratorWrapper<T>::operator>=(const RandomAccessIteratorWrapper& rhs) const
{
	return m_iterArray >= rhs.m_iterArray;
}

template<class T>
bool RandomAccessIteratorWrapper<T>::operator>(const RandomAccessIteratorWrapper& rhs) const
{
	return m_iterArray > rhs.m_iterArray;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// class RandomAccessConstIteratorWrapper

template<class T>
RandomAccessConstIteratorWrapper<T>::RandomAccessConstIteratorWrapper(typename std::vector<T>::const_iterator iterArray) :
	m_iterArray(iterArray)
{
}

template<class T>
RandomAccessConstIteratorWrapper<T>::RandomAccessConstIteratorWrapper()
{
}

template<class T>
RandomAccessConstIteratorWrapper<T>::RandomAccessConstIteratorWrapper(const RandomAccessIteratorWrapper<T>& rIterator) :
	m_iterArray(rIterator.m_iterArray)
{
}

template<class T>
RandomAccessConstIteratorWrapper<T>::~RandomAccessConstIteratorWrapper()
{
}

template<class T>
RandomAccessConstIteratorWrapper<T>& RandomAccessConstIteratorWrapper<T>::operator=(const RandomAccessIteratorWrapper<T>& rIterator)
{
	m_iterArray = rIterator.m_iterArray;
	return *this;
}

template<class T>
RandomAccessConstIteratorWrapper<T>& RandomAccessConstIteratorWrapper<T>::operator+=(const int& rhs)
{
	m_iterArray += rhs; return *this;
}

template<class T>
bool RandomAccessConstIteratorWrapper<T>::operator<=(const RandomAccessConstIteratorWrapper& rhs) const
{
	return m_iterArray <= rhs.m_iterArray;
}

template<class T>
bool RandomAccessConstIteratorWrapper<T>::operator<(const RandomAccessConstIteratorWrapper& rhs) const
{
	return m_iterArray < rhs.m_iterArray;
}

template<class T>
bool RandomAccessConstIteratorWrapper<T>::operator!=(const RandomAccessConstIteratorWrapper& rhs) const
{
	return m_iterArray != rhs.m_iterArray;
}

template<class T>
bool RandomAccessConstIteratorWrapper<T>::operator==(const RandomAccessConstIteratorWrapper& rhs) const
{
	return m_iterArray == rhs.m_iterArray;
}

template<class T>
RandomAccessConstIteratorWrapper<T> RandomAccessConstIteratorWrapper<T>::operator--(int)
{
	RandomAccessConstIteratorWrapper tmp(*this); --m_iterArray; return tmp;
}

template<class T>
RandomAccessConstIteratorWrapper<T>& RandomAccessConstIteratorWrapper<T>::operator--()
{
	--m_iterArray; return *this;
}

template<class T>
RandomAccessConstIteratorWrapper<T>& RandomAccessConstIteratorWrapper<T>::operator-=(const int& rhs)
{
	m_iterArray -= rhs; return *this;
}

template<class T>
const T& RandomAccessConstIteratorWrapper<T>::operator*()
{
	return *m_iterArray;
}

template<class T>
const T* RandomAccessConstIteratorWrapper<T>::operator->()
{
	return &(*m_iterArray);
}

template<class T>
const T& RandomAccessConstIteratorWrapper<T>::operator[](const int& rhs) const
{
	return m_iterArray[rhs];
}

template<class T>
RandomAccessConstIteratorWrapper<T> RandomAccessConstIteratorWrapper<T>::operator++(int)
{
	RandomAccessConstIteratorWrapper tmp(*this); ++m_iterArray; return tmp;
}

template<class T>
RandomAccessConstIteratorWrapper<T>& RandomAccessConstIteratorWrapper<T>::operator++()
{
	++m_iterArray; return *this;
}

template<class T>
RandomAccessConstIteratorWrapper<T> RandomAccessConstIteratorWrapper<T>::operator+(const int& rhs) const
{
	return RandomAccessConstIteratorWrapper(m_iterArray + rhs);
}

template<class T>
RandomAccessConstIteratorWrapper<T> RandomAccessConstIteratorWrapper<T>::operator-(const int& rhs) const
{
	return RandomAccessConstIteratorWrapper(m_iterArray - rhs);
}

template<class T>
bool RandomAccessConstIteratorWrapper<T>::operator>=(const RandomAccessConstIteratorWrapper& rhs) const
{
	return m_iterArray >= rhs.m_iterArray;
}

template<class T>
bool RandomAccessConstIteratorWrapper<T>::operator>(const RandomAccessConstIteratorWrapper& rhs) const
{
	return m_iterArray > rhs.m_iterArray;
}

#endif
