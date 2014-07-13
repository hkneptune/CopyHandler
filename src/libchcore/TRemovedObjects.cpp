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
#include "stdafx.h"
#include "TRemovedObjects.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

TRemovedObjects::TRemovedObjects()
{
}

TRemovedObjects::~TRemovedObjects()
{
}

void TRemovedObjects::Add(object_id_t oidObjectID)
{
	m_setObjects.insert(oidObjectID);
}

size_t TRemovedObjects::GetCount() const
{
	return m_setObjects.size();
}

object_id_t TRemovedObjects::GetAt(size_t stIndex) const
{
	if(stIndex >= m_setObjects.size())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	std::set<object_id_t>::const_iterator iter = m_setObjects.begin();
	std::advance(iter, stIndex);
	return *iter;
}

void TRemovedObjects::Clear()
{
	m_setObjects.clear();
}

bool TRemovedObjects::IsEmpty() const
{
	return m_setObjects.empty();
}

END_CHCORE_NAMESPACE
