// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
#ifndef __RANDOMACCESSCONTAINERWRAPPER_H__
#define __RANDOMACCESSCONTAINERWRAPPER_H__

#include "RandomAccessIterators.h"

template<class T>
class RandomAccessContainerWrapper
{
public:
	using iterator = RandomAccessIteratorWrapper<T>;
	using const_iterator = RandomAccessConstIteratorWrapper<T>;

public:
	virtual ~RandomAccessContainerWrapper();

	bool operator==(const RandomAccessContainerWrapper& rSrc) const;
	bool operator!=(const RandomAccessContainerWrapper& rSrc) const;

	void Add(const T& str);
	void InsertAt(size_t stIndex, const T& str);
	void SetAt(size_t stIndex, const T& str);
	void RemoveAt(size_t stIndex);
	void Clear();

	const T& GetAt(size_t stIndex) const;

	bool IsEmpty() const;
	size_t GetCount() const;

	void Append(const RandomAccessContainerWrapper& rSrc);

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

protected:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<T> m_vItems;
#pragma warning(pop)
};

template<class T>
RandomAccessContainerWrapper<T>::~RandomAccessContainerWrapper()
{
}

template<class T>
void RandomAccessContainerWrapper<T>::Add(const T& str)
{
	m_vItems.push_back(str);
}

template<class T>
void RandomAccessContainerWrapper<T>::InsertAt(size_t stIndex, const T& str)
{
	if (stIndex > m_vItems.size())
		throw std::out_of_range("stIndex out of bounds");

	m_vItems.insert(m_vItems.begin() + stIndex, str);
}

template<class T>
void RandomAccessContainerWrapper<T>::SetAt(size_t stIndex, const T& str)
{
	if (stIndex >= m_vItems.size())
		throw std::out_of_range("stIndex out of bounds");

	m_vItems[stIndex] = str;
}

template<class T>
void RandomAccessContainerWrapper<T>::RemoveAt(size_t stIndex)
{
	if (stIndex >= m_vItems.size())
		throw std::out_of_range("stIndex out of bounds");

	m_vItems.erase(m_vItems.begin() + stIndex);
}

template<class T>
void RandomAccessContainerWrapper<T>::Clear()
{
	m_vItems.clear();
}

template<class T>
const T& RandomAccessContainerWrapper<T>::GetAt(size_t stIndex) const
{
	if (stIndex >= m_vItems.size())
		throw std::out_of_range("stIndex out of bounds");

	return m_vItems.at(stIndex);
}

template<class T>
bool RandomAccessContainerWrapper<T>::IsEmpty() const
{
	return m_vItems.empty();
}

template<class T>
size_t RandomAccessContainerWrapper<T>::GetCount() const
{
	return m_vItems.size();
}

template<class T>
void RandomAccessContainerWrapper<T>::Append(const RandomAccessContainerWrapper& rSrc)
{
	m_vItems.insert(m_vItems.end(), rSrc.m_vItems.begin(), rSrc.m_vItems.end());
}

template<class T>
typename RandomAccessContainerWrapper<T>::iterator RandomAccessContainerWrapper<T>::begin()
{
	return iterator(m_vItems.begin());
}

template<class T>
typename RandomAccessContainerWrapper<T>::iterator RandomAccessContainerWrapper<T>::end()
{
	return iterator(m_vItems.end());
}

template<class T>
typename RandomAccessContainerWrapper<T>::const_iterator RandomAccessContainerWrapper<T>::begin() const
{
	return const_iterator(m_vItems.begin());
}

template<class T>
typename RandomAccessContainerWrapper<T>::const_iterator RandomAccessContainerWrapper<T>::end() const
{
	return const_iterator(m_vItems.end());
}

template<class T>
typename RandomAccessContainerWrapper<T>::const_iterator RandomAccessContainerWrapper<T>::cbegin() const
{
	return const_iterator(m_vItems.cbegin());
}

template<class T>
typename RandomAccessContainerWrapper<T>::const_iterator RandomAccessContainerWrapper<T>::cend() const
{
	return const_iterator(m_vItems.cend());
}

template<class T>
bool RandomAccessContainerWrapper<T>::operator==(const RandomAccessContainerWrapper& rSrc) const
{
	if (rSrc.GetCount() != GetCount())
		return false;

	size_t stCount = GetCount();
	while (stCount-- > 0)
	{
		if (m_vItems[stCount] != rSrc.m_vItems[stCount])
			return false;
	}

	return true;
}

template<class T>
bool RandomAccessContainerWrapper<T>::operator!=(const RandomAccessContainerWrapper& rSrc) const
{
	if (rSrc.GetCount() != GetCount())
		return true;

	size_t stCount = GetCount();
	while (stCount-- > 0)
	{
		if (m_vItems[stCount] != rSrc.m_vItems[stCount])
			return true;
	}

	return false;
}

#endif
