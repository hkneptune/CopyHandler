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
/// @file  TStringSet.h
/// @date  2011/06/05
/// @brief Contains definition of string set.
// ============================================================================
#ifndef __TSTRINGSET_H__
#define __TSTRINGSET_H__

#include "TString.h"
#include "libchcore.h"

namespace chcore
{
	class LIBCHCORE_API TStringSetIterator
	{
	protected:
		TStringSetIterator(std::set<TString>::iterator iterSet);

	public:
		TStringSetIterator();
		~TStringSetIterator();

		TStringSetIterator operator++(int);
		TStringSetIterator& operator++();

		bool operator==(const TStringSetIterator& rSrc) const;
		bool operator!=(const TStringSetIterator& rSrc) const;

		const TString& operator*() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::set<TString>::iterator m_iterSet;
#pragma warning(pop)

		friend class TStringSet;
	};

	class LIBCHCORE_API TStringSetConstIterator
	{
	protected:
		TStringSetConstIterator(std::set<TString>::const_iterator iterSet);

	public:
		TStringSetConstIterator();
		~TStringSetConstIterator();

		TStringSetConstIterator operator++(int);
		TStringSetConstIterator& operator++();

		bool operator==(const TStringSetConstIterator& rSrc) const;
		bool operator!=(const TStringSetConstIterator& rSrc) const;

		const TString& operator*() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::set<TString>::const_iterator m_iterSet;
#pragma warning(pop)

		friend class TStringSet;
	};

	class LIBCHCORE_API TStringSet
	{
	public:
		typedef TStringSetIterator iterator;
		typedef TStringSetConstIterator const_iterator;

	public:
		TStringSet();
		~TStringSet();

		void Insert(const TString& str);
		void Insert(const TStringSet& setStrings);
		void Remove(const TString& str);
		void Clear();

		bool HasValue(const TString& str) const;
		size_t GetCount() const;
		bool IsEmpty() const;

		TStringSetIterator Begin();
		TStringSetIterator End();
		TStringSetConstIterator Begin() const;
		TStringSetConstIterator End() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::set<TString> m_setItems;
#pragma warning(pop)
	};
}

#endif
