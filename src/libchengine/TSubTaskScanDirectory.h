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
/// @file  TSubTaskScanDirectory.h
/// @date  2010/09/18
/// @brief Contains declarations of classes responsible for directory scan sub-operation.
// ============================================================================
#ifndef __TSUBTASKSCANDIRECTORY_H__
#define __TSUBTASKSCANDIRECTORY_H__

#include "TSubTaskBase.h"
#include "TBasePathData.h"
#include "../liblogger/TLogger.h"

namespace chengine
{
	class TFileFiltersArray;

	///////////////////////////////////////////////////////////////////////////
	// TSubTaskScanDirectories

	class LIBCHENGINE_API TSubTaskScanDirectories : public TSubTaskBase
	{
	public:
		explicit TSubTaskScanDirectories(TSubTaskContext& rContext);
		virtual ~TSubTaskScanDirectories();

		void Reset() override;

		void InitBeforeExec() override;
		ESubOperationResult Exec() override;
		ESubOperationType GetSubOperationType() const override { return eSubOperation_Scanning; }

		void Store(const serializer::ISerializerPtr& spSerializer) const override;
		void Load(const serializer::ISerializerPtr& spSerializer) override;

		void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const override;

	private:
		size_t ScanDirectory(chcore::TSmartPath pathDirName, const TBasePathDataPtr& spBasePathData,
			bool bIncludeDirs, const TFileFiltersArray& afFilters, bool bExcludeEmptyDirs);
		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSubTaskStatsInfo m_tSubTaskStats;
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)
	};
}

#endif
