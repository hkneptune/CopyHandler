/***************************************************************************
 *   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   (version 2) as published by the Free Software Foundation;             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __TFILEFILTERSARRAY_H__
#define __TFILEFILTERSARRAY_H__

#include "libchcore.h"
#include "TFileFilter.h"

BEGIN_CHCORE_NAMESPACE

class TConfig;
class TFileInfo;
typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;
class TReadBinarySerializer;
class TWriteBinarySerializer;

class LIBCHCORE_API TFileFiltersArray
{
public:
	TFileFiltersArray() {}
	~TFileFiltersArray() {}

	TFileFiltersArray& operator=(const TFileFiltersArray& rSrc);
	bool Match(const TFileInfoPtr& spInfo) const;

	void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	void Serialize(TReadBinarySerializer& rSerializer);
	void Serialize(TWriteBinarySerializer& rSerializer) const;

	bool IsEmpty() const;

	void Add(const TFileFilter& rFilter);
	bool SetAt(size_t stIndex, const TFileFilter& rNewFilter);
	const TFileFilter* GetAt(size_t stIndex) const;
	bool RemoveAt(size_t stIndex);
	size_t GetSize() const;

	void Clear();

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<TFileFilter> m_vFilters;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

CONFIG_MEMBER_SERIALIZATION(TFileFiltersArray)

#endif
