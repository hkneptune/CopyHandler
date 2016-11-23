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
#ifndef __TOBSOLETEFILES_H__
#define __TOBSOLETEFILES_H__

#include "libchcore.h"
#include "TPath.h"
#include <map>
#include "ISerializerRowData.h"
#include "SerializerDataTypes.h"
#include "TRemovedObjects.h"

namespace chcore
{
	struct ObsoleteFileInfo
	{
		ObsoleteFileInfo();
		ObsoleteFileInfo(const TSmartPath& path, bool bAdded);

		TSmartPath m_path;
		mutable bool m_bAdded;
	};

	class LIBCHCORE_API TObsoleteFiles
	{
	public:
		TObsoleteFiles();
		virtual ~TObsoleteFiles();

		void DeleteObsoleteFile(const TSmartPath& pathToDelete);

		void Store(const ISerializerContainerPtr& spContainer) const;
		void Load(const ISerializerContainerPtr& spContainer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::map<object_id_t, ObsoleteFileInfo> MapPaths;
		MapPaths m_mapPaths;
#pragma warning(pop)

		mutable TRemovedObjects m_setRemovedObjects;
		object_id_t m_oidLast;
	};
}

#endif
