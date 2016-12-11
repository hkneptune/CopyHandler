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
#ifndef __TREMOVEDOBJECTS_H__
#define __TREMOVEDOBJECTS_H__

#include "libserializer.h"
#include "SerializerDataTypes.h"
#include <boost/container/flat_set.hpp>

namespace serializer
{
	class LIBSERIALIZER_API TRemovedObjects
	{
	public:
		TRemovedObjects();
		~TRemovedObjects();

		void Add(object_id_t oidObjectID);
		size_t GetCount() const;
		object_id_t GetAt(size_t stIndex) const;
		void Clear();

		bool IsEmpty() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		boost::container::flat_set<object_id_t> m_setObjects;
#pragma warning(pop)
	};
}

#endif
