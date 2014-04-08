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
/// @file  TSubTaskContext.h
/// @date  2010/09/19
/// @brief Contains declaration of subtask context class.
// ============================================================================
#ifndef __TSUBTASKCONTEXT_H__
#define __TSUBTASKCONTEXT_H__

#include "libchcore.h"
#include "TPath.h"
#include "EOperationTypes.h"
#include "IFeedbackHandler.h"

namespace icpf
{
	class log_file;
}

BEGIN_CHCORE_NAMESPACE

class TWorkerThreadController;
class TModPathContainer;
class TBasePathDataContainer;
class TTaskConfigTracker;
class TLocalFilesystem;
class TTaskLocalStatsInfo;
class TTaskBasicProgressInfo;
class TFileInfoArray;
class TConfig;

///////////////////////////////////////////////////////////////////////////
// TSubTaskContext

class LIBCHCORE_API TSubTaskContext
{
public:
	TSubTaskContext(TConfig& rConfig, TModPathContainer& rBasePaths,
		TBasePathDataContainer& rBasePathDataContainer, TFileInfoArray& rFilesCache,
		TTaskConfigTracker& rCfgTracker, icpf::log_file& rLog,
		const IFeedbackHandlerPtr& spFeedbackHandler, TWorkerThreadController& rThreadController, TLocalFilesystem& rfsLocal);
	~TSubTaskContext();

	TConfig& GetConfig();
	const TConfig& GetConfig() const;

	chcore::EOperationType GetOperationType() const;
	void SetOperationType(chcore::EOperationType eOperationType);

	TBasePathDataContainer& GetBasePathDataContainer();
	const TBasePathDataContainer& GetBasePathDataContainer() const;

	TModPathContainer& GetBasePaths();
	const TModPathContainer& GetBasePaths() const;

	TFileInfoArray& GetFilesCache();
	const TFileInfoArray& GetFilesCache() const;

	TSmartPath GetDestinationPath() const;
	void SetDestinationPath(const TSmartPath& pathDestination);

	TTaskConfigTracker& GetCfgTracker();
	const TTaskConfigTracker& GetCfgTracker() const;

	icpf::log_file& GetLog();
	const icpf::log_file& GetLog() const;

	IFeedbackHandlerPtr GetFeedbackHandler();

	TWorkerThreadController& GetThreadController();
	const TWorkerThreadController& GetThreadController() const;

	TLocalFilesystem& GetLocalFilesystem();
	const TLocalFilesystem& GetLocalFilesystem() const;

private:
	TSubTaskContext(const TSubTaskContext& rSrc);
	TSubTaskContext& operator=(const TSubTaskContext& rSrc);

private:
	TConfig& m_rConfig;

	EOperationType m_eOperationType;

	// information about input paths
	TModPathContainer& m_rBasePaths;
	TBasePathDataContainer& m_rBasePathDataContainer;

	// data on which to operate
	TFileInfoArray& m_rFilesCache;

	TSmartPath m_pathDestination;

	// configuration changes tracking
	TTaskConfigTracker& m_rCfgTracker;

	// local filesystem access functions
	TLocalFilesystem& m_rfsLocal;

	// additional data
	icpf::log_file& m_rLog;

	// feedback handling
#pragma warning(push)
#pragma warning(disable: 4251)
	IFeedbackHandlerPtr m_spFeedbackHandler;
#pragma warning(pop)

	// thread control
	TWorkerThreadController& m_rThreadController;
};

END_CHCORE_NAMESPACE

#endif // __TSUBTASKCONTEXT_H__
