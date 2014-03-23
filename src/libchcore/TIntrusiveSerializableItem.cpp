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
#include "TIntrusiveSerializableItem.h"

BEGIN_CHCORE_NAMESPACE

TIntrusiveSerializableItem::TIntrusiveSerializableItem() :
	m_stObjectID(0),
	m_iModifications(0)
{
}

TIntrusiveSerializableItem::TIntrusiveSerializableItem(size_t stObjectID, int iModifications) :
	m_stObjectID(stObjectID),
	m_iModifications(iModifications)
{
}

TIntrusiveSerializableItem::~TIntrusiveSerializableItem()
{
}

void TIntrusiveSerializableItem::SetModification(int iFlags, int iMask)
{
	m_iModifications &= ~iMask;
	m_iModifications |= (iFlags & iMask);
}

int TIntrusiveSerializableItem::GetModifications() const
{
	return m_iModifications;
}

bool TIntrusiveSerializableItem::IsAdded() const
{
	return m_iModifications & eMod_Added;
}

bool TIntrusiveSerializableItem::IsModified() const
{
	return m_iModifications != 0;
}

void TIntrusiveSerializableItem::SetObjectID(size_t stObjectID)
{
	m_stObjectID = stObjectID;
}

size_t TIntrusiveSerializableItem::GetObjectID() const
{
	return m_stObjectID;
}

void TIntrusiveSerializableItem::ResetModifications()
{
	m_iModifications = eMod_None;
}

END_CHCORE_NAMESPACE
