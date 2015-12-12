// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TPATHCONTAINER_H__
#define __TPATHCONTAINER_H__

#include "libchcore.h"
#include "TPath.h"
#include "TConfig.h"

namespace chcore
{
	class LIBCHCORE_API TPathContainer
	{
	public:
		TPathContainer();
		TPathContainer(const TPathContainer& rSrcContainer);
		~TPathContainer();

		TPathContainer& operator=(const TPathContainer& rSrcContainer);

		void Add(const TSmartPath& spPath);
		void Append(const TPathContainer& vPaths);

		const TSmartPath& GetAt(size_t stIndex) const;
		TSmartPath& GetAt(size_t stIndex);

		void SetAt(size_t stIndex, const TSmartPath& spPath);

		void DeleteAt(size_t stIndex);
		void Clear();

		size_t GetCount() const;
		bool IsEmpty() const;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszPropName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszPropName);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TSmartPath> m_vPaths;
#pragma warning(pop)
	};
}

CONFIG_MEMBER_SERIALIZATION(TPathContainer)

#endif
