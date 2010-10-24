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
/// @file  TTaskDefinition.cpp
/// @date  2010/09/18
/// @brief Contains implementation of class representing task input data.
// ============================================================================
#include "stdafx.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>
#include "TTaskDefinition.h"

TTaskDefinition::TTaskDefinition() :
	m_bModified(false),
	m_strTaskUniqueID()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	m_strTaskUniqueID = boost::lexical_cast<std::wstring>(u).c_str();
}

TTaskDefinition::TTaskDefinition(const TTaskDefinition& rSrc) :
	m_strTaskUniqueID(rSrc.m_strTaskUniqueID),
	m_vSourcePaths(rSrc.m_vSourcePaths),
	m_pathDestinationPath(rSrc.m_pathDestinationPath),
	m_tOperationPlan(rSrc.m_tOperationPlan),
	m_tConfiguration(rSrc.m_tConfiguration),
	m_bModified(rSrc.m_bModified)
{
}

TTaskDefinition::~TTaskDefinition()
{
}

TTaskDefinition& TTaskDefinition::operator=(const TTaskDefinition& rSrc)
{
	if(this != &rSrc)
	{
		m_strTaskUniqueID = rSrc.m_strTaskUniqueID;
		m_vSourcePaths = rSrc.m_vSourcePaths;
		m_pathDestinationPath = rSrc.m_pathDestinationPath;
		m_tOperationPlan = rSrc.m_tOperationPlan;
		m_tConfiguration = rSrc.m_tConfiguration;
		m_bModified = rSrc.m_bModified;
	}

	return *this;
}

// Task unique id
CString TTaskDefinition::GetTaskUniqueID() const
{
	return m_strTaskUniqueID;
}

// Source paths
// initialize object with data (get/set, from cfg file?, from string(cmd line options))
void TTaskDefinition::AddSourcePath(const chcore::TSmartPath& tPath)
{
	m_vSourcePaths.Add(tPath);
	m_bModified = true;
}

chcore::TSmartPath TTaskDefinition::GetSourcePathAt(size_t stIndex) const
{
	return m_vSourcePaths.GetAt(stIndex);
}

size_t TTaskDefinition::GetSourcePathCount() const
{
	return m_vSourcePaths.GetCount();
}

void TTaskDefinition::ClearSourcePaths()
{
	m_vSourcePaths.Clear();
	m_bModified = true;
}

const chcore::TPathContainer& TTaskDefinition::GetSourcePaths() const
{
	return m_vSourcePaths;
}

// Destination path
void TTaskDefinition::SetDestinationPath(const chcore::TSmartPath& pathDestination)
{
	m_pathDestinationPath = pathDestination;
	m_pathDestinationPath.AppendSeparatorIfDoesNotExist();
	m_bModified = true;
}

chcore::TSmartPath TTaskDefinition::GetDestinationPath() const
{
	return m_pathDestinationPath;
}

// Operation type
void TTaskDefinition::SetOperationType(EOperationType eOperation)
{
	m_tOperationPlan.SetOperationType(eOperation);
	m_bModified = true;
}

EOperationType TTaskDefinition::GetOperationType() const
{
	return m_tOperationPlan.GetOperationType();
}

const TOperationPlan& TTaskDefinition::GetOperationPlan() const
{
	return m_tOperationPlan;
}

// Task configuration
void TTaskDefinition::SetConfig(const chcore::TConfig& rConfig)
{
	m_tConfiguration = rConfig;
	m_bModified = true;
}

chcore::TConfig& TTaskDefinition::GetConfiguration()
{
	return m_tConfiguration;
}

const chcore::TConfig& TTaskDefinition::GetConfiguration() const
{
	return m_tConfiguration;
}

// Serialization
void TTaskDefinition::Load(const CString& strPath)
{
	// read everything
	chcore::TConfig tTaskInfo;
	tTaskInfo.Read(strPath);

	// clear everything
	m_strTaskUniqueID.Empty();
	m_vSourcePaths.Clear();
	m_pathDestinationPath.Clear();

	m_tConfiguration.Clear();

	m_bModified = false;

	// get information from config file
	// task unique id - use if provided, generate otherwise
	if(!GetConfigValue(tTaskInfo, _T("TaskDefinition.UniqueID"), m_strTaskUniqueID) || m_strTaskUniqueID.IsEmpty())
	{
		boost::uuids::random_generator gen;
		boost::uuids::uuid u = gen();
		m_strTaskUniqueID = boost::lexical_cast<std::wstring>(u).c_str();

		m_bModified = true;
	}

	// basic information
	if(!GetConfigValue(tTaskInfo, _T("TaskDefinition.SourcePaths"), m_vSourcePaths) || m_vSourcePaths.IsEmpty())
		THROW(_T("Missing source paths"), 0, 0, 0);

	if(!GetConfigValue(tTaskInfo, _T("TaskDefinition.DestinationPath"), m_pathDestinationPath) || m_pathDestinationPath.IsEmpty())
		THROW(_T("Missing destination path"), 0, 0, 0);

	m_pathDestinationPath.AppendSeparatorIfDoesNotExist();

	int iOperation = eOperation_None;
	if(!tTaskInfo.GetValue(_T("TaskDefinition.OperationType"), iOperation))
		THROW(_T("Missing operation type"), 0, 0, 0);

	m_tOperationPlan.SetOperationType((EOperationType)iOperation);

	tTaskInfo.ExtractSubConfig(_T("TaskDefinition.TaskSettings"), m_tConfiguration);
}

void TTaskDefinition::Store(const CString& strPath, bool bOnlyIfModified)
{
	if(!bOnlyIfModified || m_bModified || m_tConfiguration.IsModified())
	{
		// read everything
		chcore::TConfig tTaskInfo;
		tTaskInfo.SetFilePath(strPath);

		// get information from config file
		// task unique id - use if provided, generate otherwise
		SetConfigValue(tTaskInfo, _T("TaskDefinition.UniqueID"), m_strTaskUniqueID);

		// basic information
		SetConfigValue(tTaskInfo, _T("TaskDefinition.SourcePaths.Path"), m_vSourcePaths);
		SetConfigValue(tTaskInfo, _T("TaskDefinition.DestinationPath"), m_pathDestinationPath);

		int iOperation = m_tOperationPlan.GetOperationType();
		SetConfigValue(tTaskInfo, _T("TaskDefinition.OperationType"), iOperation);

		tTaskInfo.PutSubConfig(_T("TaskDefinition.TaskSettings"), m_tConfiguration);

		tTaskInfo.Write();

		m_bModified = false;
	}
}
