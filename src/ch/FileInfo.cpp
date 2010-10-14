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
#include "FileInfo.h"
#include "FileFilter.h"
#include "DataBuffer.h"
#include "Device IO.h"
#include "imagehlp.h"
#include "ch.h"
#include "../libicpf/exception.h"
#include <limits>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileInfo::CFileInfo() :
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

CFileInfo::CFileInfo(const CFileInfo& finf) :
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

CFileInfo::~CFileInfo()
{
}

void CFileInfo::Create(const WIN32_FIND_DATA* pwfd, const chcore::TSmartPath& pathFile, size_t stSrcIndex)
{
	BOOST_ASSERT(stSrcIndex == std::numeric_limits<size_t>::max() || m_pBasePaths);
	if(stSrcIndex != std::numeric_limits<size_t>::max() && !m_pBasePaths)
		THROW(_t("Internal error: pointer not initialized."), 0, 0, 0);

	// copy data from W32_F_D
	m_pathFile = pathFile + chcore::PathFromString(pwfd->cFileName);

	// if proper index has been passed - reduce the path
	if(m_pBasePaths && stSrcIndex >= 0)
		m_pathFile.MakeRelativePath(m_pBasePaths->GetAt(stSrcIndex));	// cut path from clipboard

	m_stSrcIndex = stSrcIndex;
	m_dwAttributes = pwfd->dwFileAttributes;
	m_uhFileSize = (((ULONGLONG) pwfd->nFileSizeHigh) << 32) + pwfd->nFileSizeLow;
	m_ftCreation = pwfd->ftCreationTime;
	m_ftLastAccess = pwfd->ftLastAccessTime;
	m_ftLastWrite = pwfd->ftLastWriteTime;
	m_uiFlags = 0;
}

bool CFileInfo::Create(const chcore::TSmartPath& pathFile, size_t stSrcIndex)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(pathFile.ToString(), &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);

		// add data to members
		chcore::TSmartPath pathNew(pathFile);
		pathNew.DeleteLastComponent();
		Create(&wfd, pathNew, stSrcIndex);
		
		return true;
	}
	else
	{
		m_pathFile.Clear();
		m_stSrcIndex = std::numeric_limits<size_t>::max();
		m_dwAttributes = (DWORD)-1;
		m_uhFileSize = (unsigned __int64)-1;
		m_ftCreation.dwHighDateTime = m_ftCreation.dwLowDateTime = 0;
		m_ftLastAccess.dwHighDateTime = m_ftCreation.dwLowDateTime = 0;
		m_ftLastWrite.dwHighDateTime = m_ftCreation.dwLowDateTime = 0;
		m_uiFlags = 0;
		return false;
	}
}

chcore::TSmartPath CFileInfo::GetFileDrive() const
{
	BOOST_ASSERT(m_pBasePaths);
	if(!m_pBasePaths)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pBasePaths->GetAt(m_stSrcIndex) + m_pathFile : m_pathFile;

	TCHAR szDrive[_MAX_DRIVE];
	_tsplitpath(pathCombined.ToString(), szDrive, NULL, NULL, NULL);
	return chcore::PathFromString(szDrive);
}

chcore::TSmartPath CFileInfo::GetFileDir() const
{ 
	BOOST_ASSERT(m_pBasePaths);
	if(!m_pBasePaths)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pBasePaths->GetAt(m_stSrcIndex) + m_pathFile : m_pathFile;

	TCHAR szDir[_MAX_DIR];
	_tsplitpath(pathCombined.ToString(), NULL, szDir, NULL, NULL);
	return chcore::PathFromString(szDir);
}

chcore::TSmartPath CFileInfo::GetFileTitle() const
{
	BOOST_ASSERT(m_pBasePaths);
	if(!m_pBasePaths)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pBasePaths->GetAt(m_stSrcIndex) + m_pathFile : m_pathFile;
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(pathCombined.ToString(), NULL, NULL, szName, NULL);
	return chcore::PathFromString(szName);
}

chcore::TSmartPath CFileInfo::GetFileExt() const
{
	ASSERT(m_pBasePaths);
	BOOST_ASSERT(m_pBasePaths);
	if(!m_pBasePaths)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pBasePaths->GetAt(m_stSrcIndex) + m_pathFile : m_pathFile;
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(pathCombined.ToString(), NULL, NULL, NULL, szExt);
	return chcore::PathFromString(szExt);
}

chcore::TSmartPath CFileInfo::GetFileRoot() const
{
	ASSERT(m_pBasePaths);
	BOOST_ASSERT(m_pBasePaths);
	if(!m_pBasePaths)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pBasePaths->GetAt(m_stSrcIndex) + m_pathFile : m_pathFile;

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(pathCombined.ToString(), szDrive, szDir, NULL, NULL);
	return chcore::PathFromString(szDrive) + chcore::PathFromString(szDir);
}

chcore::TSmartPath CFileInfo::GetFileName() const
{
	BOOST_ASSERT(m_pBasePaths);
	if(!m_pBasePaths)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined;
	if(m_pBasePaths && m_stSrcIndex != std::numeric_limits<size_t>::max())
		pathCombined = m_pBasePaths->GetAt(m_stSrcIndex) + m_pathFile;
	else
	{
		ASSERT(m_stSrcIndex == std::numeric_limits<size_t>::max());
		pathCombined = m_pathFile;
	}

	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(pathCombined.ToString(), NULL, NULL, szName, szExt);
	return chcore::PathFromString(szName) + chcore::PathFromString(szExt);
}

bool CFileInfo::operator==(const CFileInfo& rInfo)
{
	return (rInfo.m_dwAttributes == m_dwAttributes && rInfo.m_ftCreation.dwHighDateTime == m_ftCreation.dwHighDateTime && rInfo.m_ftCreation.dwLowDateTime == m_ftCreation.dwLowDateTime
		&& rInfo.m_ftLastWrite.dwHighDateTime == m_ftLastWrite.dwHighDateTime && rInfo.m_ftLastWrite.dwLowDateTime == m_ftLastWrite.dwLowDateTime && rInfo.m_uhFileSize == m_uhFileSize);
}

chcore::TSmartPath CFileInfo::GetFullFilePath() const
{
	if(m_stSrcIndex != std::numeric_limits<size_t>::max())
	{
		BOOST_ASSERT(m_pBasePaths);
		if(!m_pBasePaths)
			THROW(_T("Invalid pointer"), 0, 0, 0);

		chcore::TSmartPath pathCombined = m_pBasePaths->GetAt(m_stSrcIndex);
		pathCombined += m_pathFile;
		return pathCombined;
	}
	else
		return m_pathFile;
}

///////////////////////////////////////////////////////////////////////
// Array
CFileInfoArray::CFileInfoArray(const chcore::TPathContainer& rBasePaths) :
	m_rBasePaths(rBasePaths)
{
}

CFileInfoArray::~CFileInfoArray()
{
}

void CFileInfoArray::AddFileInfo(const CFileInfoPtr& spFileInfo)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vFiles.push_back(spFileInfo);
}

size_t CFileInfoArray::GetSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vFiles.size();
}

CFileInfoPtr CFileInfoArray::GetAt(size_t stIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	
	if(stIndex >= m_vFiles.size())
		THROW(_T("Out of bounds"), 0, 0, 0);
	
	return m_vFiles.at(stIndex);
}

CFileInfo CFileInfoArray::GetCopyAt(size_t stIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	
	if(stIndex >= m_vFiles.size())
		THROW(_T("Out of bounds"), 0, 0, 0);
	const CFileInfoPtr& spInfo = m_vFiles.at(stIndex);
	if(!spInfo)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	return *spInfo;
}

void CFileInfoArray::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vFiles.clear();
}

unsigned long long CFileInfoArray::CalculateTotalSize()
{
	unsigned long long ullSize = 0;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CFileInfoPtr& spFileInfo, m_vFiles)
	{
		ullSize += spFileInfo->GetLength64();
	}

	return ullSize;
}

unsigned long long CFileInfoArray::CalculatePartialSize(size_t stCount)
{
	unsigned long long ullSize = 0;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	if(stCount > m_vFiles.size())
		THROW(_T("Invalid argument"), 0, 0, 0);

	for(std::vector<CFileInfoPtr>::iterator iter = m_vFiles.begin(); iter != m_vFiles.begin() + stCount; ++iter)
	{
		ullSize += (*iter)->GetLength64();
	}

	return ullSize;
}
