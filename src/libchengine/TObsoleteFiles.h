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

#include <map>
#include "../libchcore/TPath.h"
#include "libchengine.h"
#include "../libserializer/ISerializerContainer.h"
#include "../libserializer/TRemovedObjects.h"

namespace chengine
{
	struct ObsoleteFileInfo
	{
		ObsoleteFileInfo();
		ObsoleteFileInfo(const chcore::TSmartPath& path, bool bAdded);

		chcore::TSmartPath m_path;
		mutable bool m_bAdded;
	};

	class LIBCHENGINE_API TObsoleteFiles
	{
	public:
		TObsoleteFiles();
		TObsoleteFiles(const TObsoleteFiles&) = delete;
		virtual ~TObsoleteFiles();

		TObsoleteFiles& operator=(const TObsoleteFiles&) = delete;

		void DeleteObsoleteFile(const chcore::TSmartPath& pathToDelete);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerContainerPtr& spContainer);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::map<serializer::object_id_t, ObsoleteFileInfo> MapPaths;
		MapPaths m_mapPaths;
#pragma warning(pop)

		mutable serializer::TRemovedObjects m_setRemovedObjects;
		serializer::object_id_t m_oidLast;
	};
}

#endif
