// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
#ifndef __TTASKLOCALSTATS_H__
#define __TTASKLOCALSTATS_H__

#include "TSubTaskStatsInfo.h"
#include "TTaskStatsSnapshot.h"
#include "TSharedModificationTracker.h"
#include "IRunningTimeControl.h"

namespace chcore
{
	class TTaskLocalStatsInfo;
	class TTaskStatsSnapshot;

	class TTaskLocalStatsInfo : public IRunningTimeControl
	{
	public:
		TTaskLocalStatsInfo();
		TTaskLocalStatsInfo(const TTaskLocalStatsInfo&) = delete;
		~TTaskLocalStatsInfo();

		TTaskLocalStatsInfo& operator=(const TTaskLocalStatsInfo&) = delete;

		void Clear();
		void GetSnapshot(TTaskStatsSnapshotPtr& spSnapshot) const;

		bool IsRunning() const;

		void Store(const ISerializerContainerPtr& spContainer) const;
		void Load(const ISerializerContainerPtr& spContainer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

	protected:
		// running/not running state
		void MarkAsRunning() override;
		void MarkAsNotRunning() override;

		// time tracking
		void EnableTimeTracking() override;
		void DisableTimeTracking() override;

#pragma warning(push)
#pragma warning(disable: 4251)
		void UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const;
#pragma warning(pop)

	private:
		enum EModifications
		{
			eMod_Added,
			eMod_Timer,

			eMod_Last
		};

		typedef std::bitset<eMod_Last> Bitset;
		mutable Bitset m_setModifications;

		volatile bool m_bTaskIsRunning;

		mutable TSharedModificationTracker<TSimpleTimer, Bitset, eMod_Timer> m_tTimer;

#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)

		friend class TScopedRunningTimeTracker;
	};
}

#endif
