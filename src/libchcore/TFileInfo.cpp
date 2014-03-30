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
	m_pBasePaths(NULL),
	m_pathFile(),
	m_stSrcIndex(std::numeric_limits<size_t>::max()),
	m_dwAttributes(0),
	m_uhFileSize(0),
	m_uiFlags(0)
{
	m_ftCreation.dwHighDateTime = m_ftCreation.dwLowDateTime = 0;
	m_ftLastAccess.dwHighDateTime = m_ftLastAccess.dwLowDateTime = 0;
	m_ftLastWrite.dwHighDateTime = m_ftLastWrite.dwLowDateTime = 0;
}

TFileInfo::TFileInfo(const TFileInfo& finf) :
	m_pathFile(finf.m_pathFile),
	m_stSrcIndex(finf.m_stSrcIndex),
	m_dwAttributes(finf.m_dwAttributes),
	m_uhFileSize(finf.m_uhFileSize),
	m_ftCreation(finf.m_ftCreation),
	m_ftLastAccess(finf.m_ftLastAccess),
	m_ftLastWrite(finf.m_ftLastWrite),
	m_uiFlags(finf.m_uiFlags),
	m_pBasePaths(finf.m_pBasePaths)
{
}

TFileInfo::~TFileInfo()
{
}

void TFileInfo::Init(const TSmartPath& rpathFile, size_t stSrcIndex, const TModPathContainer* pBasePaths,
					 DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation, FILETIME ftLastAccess, FILETIME ftLastWrite,
					 uint_t uiFlags)
{
	m_pathFile = rpathFile;
	m_stSrcIndex = stSrcIndex;
	m_pBasePaths = pBasePaths;
	m_dwAttributes = dwAttributes;
	m_uhFileSize = uhFileSize;
	m_ftCreation = ftCreation;
	m_ftLastAccess = ftLastAccess;
	m_ftLastWrite = ftLastWrite;
	m_uiFlags = uiFlags;

	if(m_pBasePaths && m_stSrcIndex != std::numeric_limits<size_t>::max())
		m_pathFile.MakeRelativePath(m_pBasePaths->GetAt(m_stSrcIndex));	// cut path from clipboard
}

void TFileInfo::Init(const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation, FILETIME ftLastAccess, FILETIME ftLastWrite,
					 uint_t uiFlags)
{
	m_pathFile = rpathFile;
	m_stSrcIndex = std::numeric_limits<size_t>::max();
	m_pBasePaths = NULL;
	m_dwAttributes = dwAttributes;
	m_uhFileSize = uhFileSize;
	m_ftCreation = ftCreation;
	m_ftLastAccess = ftLastAccess;
	m_ftLastWrite = ftLastWrite;
	m_uiFlags = uiFlags;
}

void TFileInfo::SetParentObject(size_t stIndex, const TModPathContainer* pBasePaths)
{
	// cannot set parent object if there is already one specified
	if(m_pBasePaths && m_stSrcIndex != std::numeric_limits<size_t>::max())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_stSrcIndex = stIndex;
	m_pBasePaths = pBasePaths;

	if(m_pBasePaths && m_stSrcIndex != std::numeric_limits<size_t>::max())
		m_pathFile.MakeRelativePath(m_pBasePaths->GetAt(m_stSrcIndex));
}

bool TFileInfo::operator==(const TFileInfo& rInfo)
{
	return (rInfo.m_dwAttributes == m_dwAttributes && rInfo.m_ftCreation.dwHighDateTime == m_ftCreation.dwHighDateTime && rInfo.m_ftCreation.dwLowDateTime == m_ftCreation.dwLowDateTime
		&& rInfo.m_ftLastWrite.dwHighDateTime == m_ftLastWrite.dwHighDateTime && rInfo.m_ftLastWrite.dwLowDateTime == m_ftLastWrite.dwLowDateTime && rInfo.m_uhFileSize == m_uhFileSize);
}

TSmartPath TFileInfo::GetFullFilePath() const
{
	if(m_stSrcIndex != std::numeric_limits<size_t>::max())
	{
		BOOST_ASSERT(m_pBasePaths);
		if(!m_pBasePaths)
			THROW_CORE_EXCEPTION(eErr_InvalidPointer);

		TSmartPath pathCombined = m_pBasePaths->GetAt(m_stSrcIndex);
		pathCombined += m_pathFile;
		return pathCombined;
	}
	else
		return m_pathFile;
}

void TFileInfo::Serialize(TReadBinarySerializer& rSerializer)
{
	using Serializers::Serialize;

	Serialize(rSerializer, m_pathFile);
	Serialize(rSerializer, m_stSrcIndex);
	Serialize(rSerializer, m_dwAttributes);
	Serialize(rSerializer, m_uhFileSize);
	Serialize(rSerializer, m_ftCreation.dwHighDateTime);
	Serialize(rSerializer, m_ftCreation.dwLowDateTime);
	Serialize(rSerializer, m_ftLastAccess.dwHighDateTime);
	Serialize(rSerializer, m_ftLastAccess.dwLowDateTime);
	Serialize(rSerializer, m_ftLastWrite.dwHighDateTime);
	Serialize(rSerializer, m_ftLastWrite.dwLowDateTime);
	Serialize(rSerializer, m_uiFlags);
}

void TFileInfo::Serialize(TWriteBinarySerializer& rSerializer) const
{
	using Serializers::Serialize;

	Serialize(rSerializer, m_pathFile);
	Serialize(rSerializer, m_stSrcIndex);
	Serialize(rSerializer, m_dwAttributes);
	Serialize(rSerializer, m_uhFileSize);
	Serialize(rSerializer, m_ftCreation.dwHighDateTime);
	Serialize(rSerializer, m_ftCreation.dwLowDateTime);
	Serialize(rSerializer, m_ftLastAccess.dwHighDateTime);
	Serialize(rSerializer, m_ftLastAccess.dwLowDateTime);
	Serialize(rSerializer, m_ftLastWrite.dwHighDateTime);
	Serialize(rSerializer, m_ftLastWrite.dwLowDateTime);
	Serialize(rSerializer, m_uiFlags);
}

END_CHCORE_NAMESPACE
