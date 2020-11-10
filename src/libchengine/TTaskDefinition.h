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
#include "TFileFiltersArray.h"
#include "../libchcore/TPathContainer.h"
#include "FeedbackAlreadyExistsRuleList.h"
#include "FeedbackRules.h"

namespace chengine
{
	///////////////////////////////////////////////////////////////////////////
	// TTaskDefinition

	class LIBCHENGINE_API TTaskDefinition
	{
	public:
		TTaskDefinition();
		~TTaskDefinition();

		// Task unique ID
		string::TString GetTaskName() const;

		// Source paths
		void AddSourcePath(const chcore::TSmartPath& tPath);
		chcore::TSmartPath GetSourcePathAt(size_t stIndex) const;
		size_t GetSourcePathCount() const;
		void SetSourcePaths(const chcore::TPathContainer& rvPaths);
		const chcore::TPathContainer& GetSourcePaths() const;

		void ClearSourcePaths();

		// filters
		const TFileFiltersArray& GetFilters() const;
		TFileFiltersArray& GetFilters();
		void SetFilters(const TFileFiltersArray& rFilters);

		// feedback rules
		const FeedbackRules& GetFeedbackRules() const;
		FeedbackRules& GetFeedbackRules();
		void SetFeedbackRules(const FeedbackRules& rFeedbackRules);

		// Destination path
		void SetDestinationPath(const chcore::TSmartPath& pathDestination);
		chcore::TSmartPath GetDestinationPath() const;

		// Operation type
		void SetOperationType(EOperationType eOperation);
		EOperationType GetOperationType() const;
		const TOperationPlan& GetOperationPlan() const;

		// Task configuration
		void SetConfiguration(const TConfig& rConfig);
		TConfig& GetConfiguration();
		const TConfig& GetConfiguration() const;

		// Serialization
		void Load(const chcore::TSmartPath& strPath);
		void Load(const TConfig& rDataSrc, bool bAllowEmptyDstPath);
		void LoadFromString(const string::TString& strInput, bool bAllowEmptyDstPath = false);

		void Store(const chcore::TSmartPath& strPath) const;
		void Store(TConfig& rConfig) const;
		void StoreInString(string::TString& strInput);

	private:
		string::TString m_strTaskName;				///< Unique ID of the task that will process this request (generated automatically)

		// basic information
		chcore::TPathContainer m_vSourcePaths;
		chcore::TSmartPath m_pathDestinationPath;
		TFileFiltersArray m_afFilters;
		FeedbackRules m_feedbackRules;

		TOperationPlan m_tOperationPlan;			///< Describes the operation along with sub-operations to be performed on the task input data

		// Task version
		unsigned long long m_ullTaskVersion;

		// Global task settings
		TConfig m_tConfiguration;

		// Other info (volatile, not to be saved to xml)
		mutable bool m_bModified;				///< Some parameters has been modified and this object needs to be serialized again
	};
}

#endif // __TTASKDEFINITION_H__
