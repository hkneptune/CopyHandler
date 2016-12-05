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
/// @file  TStringArray.h
/// @date  2011/06/05
/// @brief Contains string array definition.
// ============================================================================
#ifndef __TSTRINGARRAY_H__
#define __TSTRINGARRAY_H__

#include "TString.h"
#include "libchcore.h"
#include <vector>

namespace chcore
{
	class LIBCHCORE_API TStringArrayIterator
	{
	protected:
		explicit TStringArrayIterator(std::vector<TString>::iterator iterArray);

	public:
		TStringArrayIterator();
		~TStringArrayIterator();

		TStringArrayIterator operator++(int);
		TStringArrayIterator& operator++();

		bool operator==(const TStringArrayIterator& rSrc) const;
		bool operator!=(const TStringArrayIterator& rSrc) const;

		TString& operator*();
		const TString& operator*() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TString>::iterator m_iterArray;
#pragma warning(pop)

		friend class TStringArray;
	};

	class LIBCHCORE_API TStringArrayConstIterator
	{
	protected:
		explicit TStringArrayConstIterator(std::vector<TString>::const_iterator iterArray);

	public:
		TStringArrayConstIterator();
		~TStringArrayConstIterator();

		TStringArrayConstIterator operator++(int);
		TStringArrayConstIterator& operator++();

		bool operator==(const TStringArrayConstIterator& rSrc) const;
		bool operator!=(const TStringArrayConstIterator& rSrc) const;

		const TString& operator*();
		const TString& operator*() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TString>::const_iterator m_iterArray;
#pragma warning(pop)

		friend class TStringArray;
	};

	class LIBCHCORE_API TStringArray
	{
	public:
		typedef TStringArrayIterator iterator;
		typedef TStringArrayConstIterator const_iterator;

	public:
		bool operator==(const TStringArray& rSrc) const;
		bool operator!=(const TStringArray& rSrc) const;

		void Add(const TString& str);
		void InsertAt(size_t stIndex, const TString& str);
		void SetAt(size_t stIndex, const TString& str);
		void RemoveAt(size_t stIndex);
		void Clear();

		const TString& GetAt(size_t stIndex) const;
		size_t GetCount() const;

		TStringArrayIterator Begin();
		TStringArrayIterator End();
		TStringArrayConstIterator Begin() const;
		TStringArrayConstIterator End() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TString> m_vItems;
#pragma warning(pop)
	};
}

#endif
