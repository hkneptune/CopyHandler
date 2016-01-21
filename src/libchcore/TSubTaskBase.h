// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTaskBase.h
/// @date  2010/09/18
/// @brief Contains declarations of common elements of sub-operations.
// ============================================================================
#ifndef __TSUBTASKBASE_H__
#define __TSUBTASKBASE_H__

#include "libchcore.h"
#include "TPath.h"
#include "ESubTaskTypes.h"
#include "TSubTaskStatsInfo.h"
#include "ISerializer.h"
#include "IFeedbackHandler.h"

namespace chcore
{
	class TSubTaskContext;
	class TFileInfo;
	typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;

	///////////////////////////////////////////////////////////////////////////
	// TSubTaskBase

	class LIBCHCORE_API TSubTaskBase
	{
	public:
		enum ESubOperationResult
		{
			eSubResult_Continue,
			eSubResult_KillRequest,
			eSubResult_Error,
			eSubResult_CancelRequest,
			eSubResult_PauseRequest,
			eSubResult_Retry,
		};

	public:
		explicit TSubTaskBase(TSubTaskContext& rContext);
		virtual ~TSubTaskBase();

		virtual void Reset() = 0;

		virtual void InitBeforeExec() = 0;
		virtual ESubOperationResult Exec(const IFeedbackHandlerPtr& spFeedbackHandler) = 0;
		virtual ESubOperationType GetSubOperationType() const = 0;

		// serialization
		virtual void Store(const ISerializerPtr& spSerializer) const = 0;
		virtual void Load(const ISerializerPtr& spSerializer) = 0;

		// stats
		virtual void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& rStats) const = 0;

	protected:
		// some common operations
		TSubTaskContext& GetContext() { return m_rContext; }
		const TSubTaskContext& GetContext() const { return m_rContext; }

		TSmartPath CalculateDestinationPath(const TFileInfoPtr& spFileInfo, TSmartPath pathDst, int iFlags);
		TSmartPath FindFreeSubstituteName(TSmartPath pathSrcPath, TSmartPath pathDstPath) const;

	private:
		TSubTaskBase(const TSubTaskBase&);
		TSubTaskBase& operator=(const TSubTaskBase&);

	private:
		TSubTaskContext& m_rContext;
	};

	typedef boost::shared_ptr<TSubTaskBase> TSubTaskBasePtr;
}

#endif
