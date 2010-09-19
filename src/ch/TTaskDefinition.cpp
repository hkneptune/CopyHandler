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
	m_bNeedsSaving(false),
	m_strTaskUniqueID()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	m_strTaskUniqueID = boost::lexical_cast<std::wstring>(u);
}

TTaskDefinition::~TTaskDefinition()
{
}

// initialize object with data (get/set, from cfg file?, from string(cmd line options))
void TTaskDefinition::AddSourcePath(const CString& strPath)
{
	CClipboardEntryPtr spEntry(boost::make_shared<CClipboardEntry>());
	spEntry->SetPath(strPath);

	m_arrSourcePaths.Add(spEntry);
}

CString TTaskDefinition::GetSourcePathAt(size_t stIndex) const
{
	CClipboardEntryPtr spEntry = m_arrSourcePaths.GetAt(stIndex);
	if(spEntry)
		return spEntry->GetPath();
	return CString();
}

size_t TTaskDefinition::GetSourcePathCount() const
{
	return m_arrSourcePaths.GetSize();
}

void TTaskDefinition::ClearSourcePaths()
{
	m_arrSourcePaths.RemoveAll();
}

void TTaskDefinition::SetDestinationPath(const CString& strPath)
{
	m_tDestinationPath.SetPath(strPath);
}

CString TTaskDefinition::GetDestinationPath() const
{
	return m_tDestinationPath.GetPath();
}

void TTaskDefinition::SetOperationType(EOperationType eOperation)
{
	m_tOperationPlan.SetOperationType(eOperation);
}

EOperationType TTaskDefinition::GetOperationType() const
{
	return m_tOperationPlan.GetOperationType();
}

TTaskConfiguration& TTaskDefinition::GetConfiguration()
{
	return m_tOperationConfiguration;
}

const TTaskConfiguration& TTaskDefinition::GetConfiguration() const
{
	return m_tOperationConfiguration;
}
