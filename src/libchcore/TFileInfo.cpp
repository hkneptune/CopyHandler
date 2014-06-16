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
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TRowData.h"
#include "ISerializerRowData.h"
#include "ISerializerContainer.h"

BEGIN_CHCORE_NAMESPACE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TFileInfo::TFileInfo() :
	m_pathFile(m_setModifications),
	m_spBasePathData(m_setModifications),
	m_dwAttributes(m_setModifications, 0),
	m_uhFileSize(m_setModifications, 0),
	m_ftCreation(m_setModifications),
	m_ftLastAccess(m_setModifications),
	m_ftLastWrite(m_setModifications),
	m_uiFlags(m_setModifications, 0),
	m_stObjectID(0)
{
	m_setModifications[eMod_Added] = true;
}

TFileInfo::TFileInfo(const TFileInfo& rSrc) :
	m_pathFile(rSrc.m_pathFile),
	m_spBasePathData(rSrc.m_spBasePathData),
	m_dwAttributes(rSrc.m_dwAttributes),
	m_uhFileSize(rSrc.m_uhFileSize),
	m_ftCreation(rSrc.m_ftCreation),
	m_ftLastAccess(rSrc.m_ftLastAccess),
	m_ftLastWrite(rSrc.m_ftLastWrite),
	m_uiFlags(rSrc.m_uiFlags),
	m_stObjectID(rSrc.m_stObjectID)
{
}

TFileInfo::~TFileInfo()
{
}

TFileInfo& TFileInfo::operator=(const TFileInfo& rSrc)
{
	if(this != & rSrc)
	{
		m_pathFile = rSrc.m_pathFile;
		m_spBasePathData = rSrc.m_spBasePathData;
		m_dwAttributes = rSrc.m_dwAttributes;
		m_uhFileSize = rSrc.m_uhFileSize;
		m_ftCreation = rSrc.m_ftCreation;
		m_ftLastAccess = rSrc.m_ftLastAccess;
		m_ftLastWrite = rSrc.m_ftLastWrite;
		m_uiFlags = rSrc.m_uiFlags;
		m_stObjectID = rSrc.m_stObjectID;
	}

	return *this;
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

	if(m_spBasePathData.Get())
		m_pathFile.Modify().MakeRelativePath(m_spBasePathData.Get()->GetSrcPath());
}

void TFileInfo::Init(const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation, FILETIME ftLastAccess, FILETIME ftLastWrite,
					 uint_t uiFlags)
{
	m_pathFile = rpathFile;
	m_spBasePathData.Modify().reset();
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
	if(m_spBasePathData.Get())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_spBasePathData = spBasePathData;
	if(m_spBasePathData.Get())
		m_pathFile.Modify().MakeRelativePath(m_spBasePathData.Get()->GetSrcPath());
}

bool TFileInfo::operator==(const TFileInfo& rInfo) const
{
	return (rInfo.m_dwAttributes == m_dwAttributes &&
		rInfo.m_ftCreation.Get() == m_ftCreation.Get() &&
		rInfo.m_ftLastWrite.Get() == m_ftLastWrite.Get() &&
		rInfo.m_uhFileSize == m_uhFileSize);
}

TSmartPath TFileInfo::GetFullFilePath() const
{
	if(m_spBasePathData.Get())
	{
		TSmartPath pathCombined = m_spBasePathData.Get()->GetSrcPath();
		pathCombined += m_pathFile;
		return pathCombined;
	}
	else
		return m_pathFile;
}

size_t TFileInfo::GetSrcObjectID() const
{
	if(m_spBasePathData.Get())
		return m_spBasePathData.Get()->GetObjectID();
	return std::numeric_limits<size_t>::max();
}

TBasePathDataPtr TFileInfo::GetBasePathData() const
{
	return m_spBasePathData;
}

void TFileInfo::MarkAsProcessed(bool bProcessed)
{
	if(bProcessed)
		m_uiFlags.Modify() |= eFlag_Processed;
	else
		m_uiFlags.Modify() &= ~eFlag_Processed;
}

bool TFileInfo::IsProcessed() const
{
	return m_uiFlags & eFlag_Processed;
}

ULONGLONG TFileInfo::GetLength64() const
{
	return m_uhFileSize;
}

void TFileInfo::SetLength64(ULONGLONG uhSize)
{
	m_uhFileSize=uhSize;
}

const TSmartPath& TFileInfo::GetFilePath() const
{
	return m_pathFile;
}

void TFileInfo::SetFilePath(const TSmartPath& tPath)
{
	m_pathFile = tPath;
}

const TFileTime& TFileInfo::GetCreationTime() const
{
	return m_ftCreation;
}

const TFileTime& TFileInfo::GetLastAccessTime() const
{
	return m_ftLastAccess;
}

const TFileTime& TFileInfo::GetLastWriteTime() const
{
	return m_ftLastWrite;
}

DWORD TFileInfo::GetAttributes() const
{
	return m_dwAttributes;
}

bool TFileInfo::IsDirectory() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool TFileInfo::IsArchived() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0;
}

bool TFileInfo::IsReadOnly() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_READONLY) != 0;
}

bool TFileInfo::IsCompressed() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0;
}

bool TFileInfo::IsSystem() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
}

bool TFileInfo::IsHidden() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
}

bool TFileInfo::IsTemporary() const
{
	return (m_dwAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0;
}

bool TFileInfo::IsNormal() const
{
	return m_dwAttributes == 0;
}

void TFileInfo::Store(const ISerializerContainerPtr& spContainer) const
{
	ISerializerRowDataPtr spRow;
	if(m_setModifications[eMod_Added])
		spRow = spContainer->AddRow(m_stObjectID);
	else if(m_setModifications.any())
		spRow = spContainer->GetRow(m_stObjectID);
	else
		return;

	if(m_setModifications[eMod_Path])
		*spRow % TRowData(_T("rel_path"), m_pathFile);
	if(m_setModifications[eMod_BasePath])
		*spRow % TRowData(_T("base_path_id"), m_spBasePathData.Get()->GetObjectID());
	if(m_setModifications[eMod_Attributes])
		*spRow % TRowData(_T("attr"), m_dwAttributes);
	if(m_setModifications[eMod_FileSize])
		*spRow % TRowData(_T("size"), m_uhFileSize);
	if(m_setModifications[eMod_TimeCreated])
		*spRow % TRowData(_T("time_created"), m_ftCreation.Get().ToUInt64());
	if(m_setModifications[eMod_TimeLastWrite])
		*spRow % TRowData(_T("time_last_write"), m_ftLastWrite.Get().ToUInt64());
	if(m_setModifications[eMod_TimeLastAccess])
		*spRow % TRowData(_T("time_last_access"), m_ftLastAccess.Get().ToUInt64());
	if(m_setModifications[eMod_Flags])
		*spRow % TRowData(_T("flags"), m_uiFlags);

	m_setModifications.reset();
}

void TFileInfo::InitLoader(IColumnsDefinition& rColumns)
{
	rColumns % _T("id") % _T("rel_path") % _T("base_path_id") % _T("attr") % _T("size") % _T("time_created") % _T("time_last_write") % _T("time_last_access") % _T("flags");
}

void TFileInfo::Load(const ISerializerRowReaderPtr& spRowReader, const TBasePathDataContainerPtr& spSrcContainer)
{
	size_t stBaseObjectID = 0;
	unsigned long long ullTime = 0;
	spRowReader->GetValue(_T("id"), m_stObjectID);
	spRowReader->GetValue(_T("rel_path"), m_pathFile.Modify());
	spRowReader->GetValue(_T("base_path_id"), stBaseObjectID);
	spRowReader->GetValue(_T("attr"), m_dwAttributes.Modify());
	spRowReader->GetValue(_T("size"), m_uhFileSize.Modify());

	spRowReader->GetValue(_T("time_created"), ullTime);
	m_ftCreation.Modify().FromUInt64(ullTime);

	spRowReader->GetValue(_T("time_last_write"), ullTime);
	m_ftLastWrite.Modify().FromUInt64(ullTime);

	spRowReader->GetValue(_T("time_last_access"), ullTime);
	m_ftLastAccess.Modify().FromUInt64(ullTime);

	spRowReader->GetValue(_T("flags"), m_uiFlags.Modify());

	m_spBasePathData = spSrcContainer->FindByID(stBaseObjectID);

	m_setModifications.reset();
}

size_t TFileInfo::GetObjectID() const
{
	return m_stObjectID;
}

void TFileInfo::SetObjectID(size_t stObjectID)
{
	m_stObjectID = stObjectID;
}

END_CHCORE_NAMESPACE
