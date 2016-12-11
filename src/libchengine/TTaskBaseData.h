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

#include <bitset>
#include "ETaskCurrentState.h"
#include "../libstring/TString.h"
#include "../libchcore/TPath.h"
#include "libchengine.h"
#include "../libserializer/ISerializerContainer.h"
#include "../libserializer/TSharedModificationTracker.h"

namespace chengine
{
	class LIBCHENGINE_API TTaskBaseData
	{
	public:
		TTaskBaseData();
		~TTaskBaseData();

		string::TString GetTaskName() const;
		void SetTaskName(const string::TString& strTaskName);

		ETaskCurrentState GetCurrentState() const;
		void SetCurrentState(ETaskCurrentState eCurrentState);

		chcore::TSmartPath GetDestinationPath() const;
		void SetDestinationPath(const chcore::TSmartPath& pathDst);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerContainerPtr& spContainer);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

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

		serializer::TSharedModificationTracker<string::TString, ModBitSet, eMod_TaskName> m_strTaskName;
		serializer::TSharedModificationTracker<volatile ETaskCurrentState, ModBitSet, eMod_CurrentState> m_eCurrentState;     // current state of processing this task represents
		serializer::TSharedModificationTracker<chcore::TSmartPath, ModBitSet, eMod_DstPath> m_pathDestinationPath;
#pragma warning(pop)
	};
}

#endif
