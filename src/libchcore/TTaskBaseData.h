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
#ifndef __TTASKBASEDATA_H__
#define __TTASKBASEDATA_H__

#include "ISerializerRowData.h"
#include <bitset>
#include "TSharedModificationTracker.h"
#include "ETaskCurrentState.h"

namespace chcore
{
	class LIBCHCORE_API TTaskBaseData
	{
	public:
		TTaskBaseData();
		~TTaskBaseData();

		TString GetTaskName() const;
		void SetTaskName(const TString& strTaskName);

		ETaskCurrentState GetCurrentState() const;
		void SetCurrentState(ETaskCurrentState eCurrentState);

		TSmartPath GetDestinationPath() const;
		void SetDestinationPath(const TSmartPath& pathDst);

		void Store(const ISerializerContainerPtr& spContainer) const;
		void Load(const ISerializerContainerPtr& spContainer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

	private:
		enum EModifications
		{
			eMod_Added,
			eMod_TaskName,
			eMod_CurrentState,
			eMod_DstPath,

			eMod_Last
		};

#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::bitset<eMod_Last> ModBitSet;
		mutable ModBitSet m_setChanges;

		TSharedModificationTracker<TString, ModBitSet, eMod_TaskName> m_strTaskName;
		TSharedModificationTracker<volatile ETaskCurrentState, ModBitSet, eMod_CurrentState> m_eCurrentState;     // current state of processing this task represents
		TSharedModificationTracker<TSmartPath, ModBitSet, eMod_DstPath> m_pathDestinationPath;
#pragma warning(pop)
	};
}

#endif
