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
/// @file  TTaskDefinition.h
/// @date  2010/09/18
/// @brief Contains declaration of class representing task input data.
// ============================================================================
#ifndef __TTASKDEFINITION_H__
#define __TTASKDEFINITION_H__

#include "TTaskOperationPlan.h"
#include "TConfig.h"
#include "FileInfo.h"

///////////////////////////////////////////////////////////////////////////
// TTaskDefinition

class TTaskDefinition
{
public:
	TTaskDefinition();
	TTaskDefinition(const TTaskDefinition& rSrc);
	~TTaskDefinition();

	TTaskDefinition& operator=(const TTaskDefinition& rSrc);

	// Task unique ID
	CString GetTaskUniqueID() const;

	// Source paths
	void AddSourcePath(const chcore::TSmartPath& tPath);
	chcore::TSmartPath GetSourcePathAt(size_t stIndex) const;
	size_t GetSourcePathCount() const;
	const chcore::TPathContainer& GetSourcePaths() const;

	void ClearSourcePaths();

	// Destination path
	void SetDestinationPath(const chcore::TSmartPath& pathDestination);
	chcore::TSmartPath GetDestinationPath() const;

	// Operation type
	void SetOperationType(EOperationType eOperation);
	EOperationType GetOperationType() const;
	const TOperationPlan& GetOperationPlan() const;

	// Task configuration
	void SetConfig(const TConfig& rConfig);
	TConfig& GetConfiguration();
	const TConfig& GetConfiguration() const;

	// Serialization
	void Load(const CString& strPath);
	void Store(const CString& strPath, bool bOnlyIfModified = false);

private:
	CString m_strTaskUniqueID;				///< Unique ID of the task that will process this request (generated automatically)

	// basic information
	chcore::TPathContainer m_vSourcePaths;
	chcore::TSmartPath m_pathDestinationPath;

	TOperationPlan m_tOperationPlan;			///< Describes the operation along with sub-operations to be performed on the task input data

	// Global task settings
	TConfig	m_tConfiguration;

	// Other info (volatile, not to be saved to xml)
	mutable bool m_bModified;				///< Some parameters has been modified and this object needs to be serialized again
};

#endif // __TTASKDEFINITION_H__
