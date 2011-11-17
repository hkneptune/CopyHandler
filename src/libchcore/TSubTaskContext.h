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

namespace icpf
{
	class log_file;
}

BEGIN_CHCORE_NAMESPACE

class IFeedbackHandler;
class TTaskDefinition;
class TWorkerThreadController;
class TBasePathDataContainer;
class TTaskConfigTracker;
class TLocalFilesystem;
class TTaskLocalStats;
class TTaskBasicProgressInfo;
class TFileInfoArray;

///////////////////////////////////////////////////////////////////////////
// TSubTaskContext

class LIBCHCORE_API TSubTaskContext
{
public:
	TSubTaskContext(TTaskDefinition& rTaskDefinition, TBasePathDataContainer& rBasePathDataContainer, TFileInfoArray& rFilesCache, TTaskLocalStats& rTaskLocalStats,
		TTaskConfigTracker& rCfgTracker, icpf::log_file& rLog,
		IFeedbackHandler* piFeedbackHandler, TWorkerThreadController& rThreadController, TLocalFilesystem& rfsLocal);
	~TSubTaskContext();

	TTaskDefinition& GetTaskDefinition() { return m_rTaskDefinition; }
	const TTaskDefinition& GetTaskDefinition() const { return m_rTaskDefinition; }

	TBasePathDataContainer& GetBasePathDataContainer() { return m_rBasePathDataContainer; }
	const TBasePathDataContainer& GetBasePathDataContainer() const { return m_rBasePathDataContainer; }

	TFileInfoArray& GetFilesCache() { return m_rFilesCache; }
	const TFileInfoArray& GetFilesCache() const { return m_rFilesCache; }

	TTaskLocalStats& GetTaskLocalStats() { return m_rTaskLocalStats; }
	const TTaskLocalStats& GetTaskLocalStats() const { return m_rTaskLocalStats; }

	TTaskConfigTracker& GetCfgTracker() { return m_rCfgTracker; }
	const TTaskConfigTracker& GetCfgTracker() const { return m_rCfgTracker; }

	icpf::log_file& GetLog() { return m_rLog; }
	const icpf::log_file& GetLog() const { return m_rLog; }

	IFeedbackHandler* GetFeedbackHandler() { return m_piFeedbackHandler; }
	const IFeedbackHandler* GetFeedbackHandler() const { return m_piFeedbackHandler; }

	TWorkerThreadController& GetThreadController() { return m_rThreadController; }
	const TWorkerThreadController& GetThreadController() const { return m_rThreadController; }

	TLocalFilesystem& GetLocalFilesystem() { return m_rfsLocal; }
	const TLocalFilesystem& GetLocalFilesystem() const { return m_rfsLocal; }

private:
	TSubTaskContext(const TSubTaskContext& rSrc);
	TSubTaskContext& operator=(const TSubTaskContext& rSrc);

private:
	TTaskDefinition& m_rTaskDefinition;

	// information about input paths
	TBasePathDataContainer& m_rBasePathDataContainer;

	// data on which to operate
	TFileInfoArray& m_rFilesCache;

	// local stats for task
	TTaskLocalStats& m_rTaskLocalStats;

	// configuration changes tracking
	TTaskConfigTracker& m_rCfgTracker;

	// local filesystem access functions
	TLocalFilesystem& m_rfsLocal;

	// additional data
	icpf::log_file& m_rLog;

	// feedback handling
	IFeedbackHandler* m_piFeedbackHandler;

	// thread control
	TWorkerThreadController& m_rThreadController;
};

END_CHCORE_NAMESPACE

#endif // __TSUBTASKCONTEXT_H__
