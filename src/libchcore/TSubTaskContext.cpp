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
/// @file  TSubTaskContext.cpp
/// @date  2010/09/19
/// @brief Contains implementation of classes related to subtask context.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskContext.h"
#include "ErrorCodes.h"
#include "TCoreException.h"

BEGIN_CHCORE_NAMESPACE

TSubTaskContext::TSubTaskContext(TConfig& rConfig, const TBasePathDataContainerPtr& spBasePaths, TFileInfoArray& rFilesCache,
								TTaskConfigTracker& rCfgTracker, icpf::log_file& rLog, const IFeedbackHandlerPtr& spFeedbackHandler,
								TWorkerThreadController& rThreadController, TLocalFilesystem& rfsLocal) :
	m_rConfig(rConfig),
	m_eOperationType(eOperation_None),
	m_spBasePaths(spBasePaths),
	m_rFilesCache(rFilesCache),
	m_pathDestination(),
	m_rCfgTracker(rCfgTracker),
	m_rLog(rLog),
	m_spFeedbackHandler(spFeedbackHandler),
	m_rThreadController(rThreadController),
	m_rfsLocal(rfsLocal)
{
}

TSubTaskContext::~TSubTaskContext()
{
}

TConfig& TSubTaskContext::GetConfig()
{
	return m_rConfig;
}

const TConfig& TSubTaskContext::GetConfig() const
{
	return m_rConfig;
}

chcore::EOperationType TSubTaskContext::GetOperationType() const
{
	return m_eOperationType;
}

void TSubTaskContext::SetOperationType(chcore::EOperationType eOperationType)
{
	m_eOperationType = eOperationType;
}

TBasePathDataContainerPtr TSubTaskContext::GetBasePaths() const
{
	return m_spBasePaths;
}

TFileInfoArray& TSubTaskContext::GetFilesCache()
{
	return m_rFilesCache;
}

const TFileInfoArray& TSubTaskContext::GetFilesCache() const
{
	return m_rFilesCache;
}

chcore::TSmartPath TSubTaskContext::GetDestinationPath() const
{
	return m_pathDestination;
}

void TSubTaskContext::SetDestinationPath(const TSmartPath& pathDestination)
{
	m_pathDestination = pathDestination;
}

TTaskConfigTracker& TSubTaskContext::GetCfgTracker()
{
	return m_rCfgTracker;
}

const TTaskConfigTracker& TSubTaskContext::GetCfgTracker() const
{
	return m_rCfgTracker;
}

icpf::log_file& TSubTaskContext::GetLog()
{
	return m_rLog;
}

const icpf::log_file& TSubTaskContext::GetLog() const
{
	return m_rLog;
}

chcore::IFeedbackHandlerPtr TSubTaskContext::GetFeedbackHandler()
{
	if(!m_spFeedbackHandler)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	return m_spFeedbackHandler;
}

TWorkerThreadController& TSubTaskContext::GetThreadController()
{
	return m_rThreadController;
}

const TWorkerThreadController& TSubTaskContext::GetThreadController() const
{
	return m_rThreadController;
}

TLocalFilesystem& TSubTaskContext::GetLocalFilesystem()
{
	return m_rfsLocal;
}

const TLocalFilesystem& TSubTaskContext::GetLocalFilesystem() const
{
	return m_rfsLocal;
}

END_CHCORE_NAMESPACE
