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
#ifndef __TSQLITESERIALIZERCONTAINER_H__
#define __TSQLITESERIALIZERCONTAINER_H__

#include "libchcore.h"
#include "ISerializerRow.h"
#include "ISerializerContainer.h"
#include <map>
#include <boost/optional.hpp>
#include "TSQLiteColumnDefinition.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TSQLiteSerializerContainer : public ISerializerContainer
{
public:
	TSQLiteSerializerContainer();
	TSQLiteSerializerContainer(size_t stParentID);
	virtual ~TSQLiteSerializerContainer();

	ISerializerRowPtr GetNewRow();
	ISerializerRowPtr GetRow(size_t stRowID);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::map<size_t, ISerializerRowPtr> m_mapRows;
	boost::optional<size_t> m_stParentID;
	TSQLiteColumnDefinitionPtr m_spColumns;
#pragma warning(pop)
};

typedef boost::shared_ptr<TSQLiteSerializerContainer> TSQLiteSerializerContainerPtr;

END_CHCORE_NAMESPACE

#endif
