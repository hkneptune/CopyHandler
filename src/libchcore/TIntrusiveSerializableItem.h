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
#ifndef __TINTRUSIVESERIALIZABLEITEM_H__
#define __TINTRUSIVESERIALIZABLEITEM_H__

#include "libchcore.h"
#include <limits>

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TIntrusiveSerializableItem
{
public:
	enum EModificationFlags
	{
		eMod_None = 0,
		eMod_Added = 1,
		eMod_Modified = 2,	// a base for derived classes to implement own modified states
	};

public:
	TIntrusiveSerializableItem();
	TIntrusiveSerializableItem(size_t stObjectID, int iModifications = eMod_None);
	virtual ~TIntrusiveSerializableItem();

	void SetModification(int iFlags, int iMask = std::numeric_limits<int>::max());
	int GetModifications() const;
	void ResetModifications();

	bool IsAdded() const;
	bool IsModified() const;	// has modifications? added state is also considered a modification

	void SetObjectID(size_t stObjectID);
	size_t GetObjectID() const;

protected:
	size_t m_stObjectID;
	int m_iModifications;
};

typedef boost::shared_ptr<TIntrusiveSerializableItem> TIntrusiveSerializableItemPtr;

END_CHCORE_NAMESPACE

#endif
