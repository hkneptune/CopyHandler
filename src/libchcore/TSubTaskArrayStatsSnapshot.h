// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
#ifndef __TSUBTASKARRAYSTATSSNAPSHOT_H__
#define __TSUBTASKARRAYSTATSSNAPSHOT_H__

#include "libchcore.h"
#include "TSubTaskStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TSubTaskArrayStatsSnapshot
{
public:
	TSubTaskArrayStatsSnapshot();

	void Clear();

	void AddSubTaskSnapshot(const TSubTaskStatsSnapshotPtr& rSnapshot);
	TSubTaskStatsSnapshotPtr GetSubTaskSnapshotAt(size_t stIndex) const;
	size_t GetSubTaskSnapshotCount() const;
	TSubTaskStatsSnapshotPtr GetCurrentSubTaskSnapshot() const;

	size_t GetCurrentSubtaskIndex() const { return m_stCurrentSubtaskIndex; }
	void SetCurrentSubtaskIndex(size_t val) { m_stCurrentSubtaskIndex = val; }

private:
	size_t m_stCurrentSubtaskIndex;
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<TSubTaskStatsSnapshotPtr> m_vSubTaskSnapshots;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
