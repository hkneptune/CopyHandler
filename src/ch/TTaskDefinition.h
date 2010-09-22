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
#include "TTaskConfiguration.h"

///////////////////////////////////////////////////////////////////////////
// TTaskDefinition

class TTaskDefinition
{
public:
	TTaskDefinition();
	~TTaskDefinition();

	// initialize object with data (get/set, from cfg file?, from string(cmd line options))
	void AddSourcePath(const CString& strPath);
	CString GetSourcePathNameAt(size_t stIndex) const;
	CClipboardEntryPtr GetSourcePathAt(size_t stIndex) const;
	size_t GetSourcePathCount() const;
	void ClearSourcePaths();
	const CClipboardArray& GetSourcePaths() const;

	void SetDestinationPath(const CString& strPath);
	CString GetDestinationPath() const;
	const CDestPath& GetDestPath() const;

	void SetOperationType(EOperationType eOperation);
	EOperationType GetOperationType() const;
	const TOperationPlan& GetOperationPlan() const;

	TTaskConfiguration& GetConfiguration();
	const TTaskConfiguration& GetConfiguration() const;

	CString GetTaskUniqueID() const;

	// serialize (from/to xml)
	template<class Archive>
	void Load(Archive& ar, bool bData, const unsigned int uiVersion)
	{
		if(bData)
		{
			ar & m_strTaskUniqueID;
			m_arrSourcePaths.Load(ar, uiVersion, bData);
			ar & m_tDestinationPath;
			ar & m_tOperationPlan;
			ar & m_tOperationConfiguration;
		}
		else
		{
			m_arrSourcePaths.Load(ar, uiVersion, bData);
		}

		m_bNeedsSaving = false;
	}

	template<class Archive>
	void Save(Archive& ar, bool bData, const unsigned int uiVersion) const
	{
		if(bData)
		{
			ar & m_strTaskUniqueID;
			m_arrSourcePaths.Store(ar, uiVersion, bData);
			ar & m_tDestinationPath;
			ar & m_tOperationPlan;
			ar & m_tOperationConfiguration;
		}
		else
			m_arrSourcePaths.Store(ar, uiVersion, bData);

		m_bNeedsSaving = false;
	}

private:
	std::wstring m_strTaskUniqueID;				///< Unique ID of the task that will process this request (generated automatically)

	// basic information
	CClipboardArray m_arrSourcePaths;		///< Contains source paths to be processed
	CDestPath m_tDestinationPath;			///< Contains destination path for the data to be processed to

	TOperationPlan m_tOperationPlan;			///< Describes the operation along with sub-operations to be performed on the task input data

	// Global task settings
	TTaskConfiguration	m_tOperationConfiguration;

	// Other info (volatile, not to be saved to xml)
	mutable bool m_bNeedsSaving;				///< Some parameters has been modified and this object needs to be serialized again
};

#endif // __TTASKDEFINITION_H__
