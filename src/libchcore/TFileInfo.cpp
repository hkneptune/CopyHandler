/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
// File was originally based on FileInfo.cpp by Antonio Tejada Lacaci.
// Almost everything has changed since then.
#include "stdafx.h"
#include "TFileInfo.h"
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TModPathContainer.h"

BEGIN_CHCORE_NAMESPACE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TFileInfo::TFileInfo() :
	m_pathFile(),
	m_dwAttributes(0),
	m_uhFileSize(0),
	m_uiFlags(0)
{
	m_ftCreation.dwHighDateTime = m_ftCreation.dwLowDateTime = 0;
	m_ftLastAccess.dwHighDateTime = m_ftLastAccess.dwLowDateTime = 0;
	m_ftLastWrite.dwHighDateTime = m_ftLastWrite.dwLowDateTime = 0;
}

TFileInfo::TFileInfo(const TFileInfo& rSrc) :
	m_pathFile(rSrc.m_pathFile),
	m_spBasePathData(rSrc.m_spBasePathData),
	m_dwAttributes(rSrc.m_dwAttributes),
	m_uhFileSize(rSrc.m_uhFileSize),
	m_ftCreation(rSrc.m_ftCreation),
	m_ftLastAccess(rSrc.m_ftLastAccess),
	m_ftLastWrite(rSrc.m_ftLastWrite),
	m_uiFlags(rSrc.m_uiFlags)
{
}

TFileInfo::~TFileInfo()
{
}

void TFileInfo::Init(const TBasePathDataPtr& spBasePathData, const TSmartPath& rpathFile,
					DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation, FILETIME ftLastAccess, FILETIME ftLastWrite,
					uint_t uiFlags)
{
	m_pathFile = rpathFile;
	m_spBasePathData = spBasePathData;
	m_dwAttributes = dwAttributes;
	m_uhFileSize = uhFileSize;
	m_ftCreation = ftCreation;
	m_ftLastAccess = ftLastAccess;
	m_ftLastWrite = ftLastWrite;
	m_uiFlags = uiFlags;

	if(m_spBasePathData)
		m_pathFile.MakeRelativePath(m_spBasePathData->GetSrcPath());
}

void TFileInfo::Init(const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation, FILETIME ftLastAccess, FILETIME ftLastWrite,
					 uint_t uiFlags)
{
	m_pathFile = rpathFile;
	m_spBasePathData.reset();
	m_dwAttributes = dwAttributes;
	m_uhFileSize = uhFileSize;
	m_ftCreation = ftCreation;
	m_ftLastAccess = ftLastAccess;
	m_ftLastWrite = ftLastWrite;
	m_uiFlags = uiFlags;
}

void TFileInfo::SetParentObject(const TBasePathDataPtr& spBasePathData)
{
	// cannot set parent object if there is already one specified
	if(m_spBasePathData)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_spBasePathData = spBasePathData;
	if(m_spBasePathData)
		m_pathFile.MakeRelativePath(m_spBasePathData->GetSrcPath());
}

bool TFileInfo::operator==(const TFileInfo& rInfo)
{
	return (rInfo.m_dwAttributes == m_dwAttributes && rInfo.m_ftCreation.dwHighDateTime == m_ftCreation.dwHighDateTime && rInfo.m_ftCreation.dwLowDateTime == m_ftCreation.dwLowDateTime
		&& rInfo.m_ftLastWrite.dwHighDateTime == m_ftLastWrite.dwHighDateTime && rInfo.m_ftLastWrite.dwLowDateTime == m_ftLastWrite.dwLowDateTime && rInfo.m_uhFileSize == m_uhFileSize);
}

TSmartPath TFileInfo::GetFullFilePath() const
{
	if(m_spBasePathData)
	{
		TSmartPath pathCombined = m_spBasePathData->GetSrcPath();
		pathCombined += m_pathFile;
		return pathCombined;
	}
	else
		return m_pathFile;
}

size_t TFileInfo::GetSrcObjectID() const
{
	if(m_spBasePathData)
		return m_spBasePathData->GetObjectID();
	return std::numeric_limits<size_t>::max();
}

TBasePathDataPtr TFileInfo::GetBasePathData() const
{
	return m_spBasePathData;
}

END_CHCORE_NAMESPACE
