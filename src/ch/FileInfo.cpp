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
/*************************************************************************
	FileInfo.cpp: implementation of the CFileInfo class.
	(c) Codeguru & friends
	Coded by Antonio Tejada Lacaci. 1999, modified by Ixen Gerthannes
	atejada@espanet.com
*************************************************************************/

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

void GetDriveData(const chcore::TSmartPath& spPath, int* piDrvNum, UINT* puiDrvType)
{
	TCHAR drv[_MAX_DRIVE+1];

	_tsplitpath(spPath.ToString(), drv, NULL, NULL, NULL);
	if(lstrlen(drv) != 0)
	{
		// add '\\'
		lstrcat(drv, _T("\\"));
		_tcsupr(drv);

		// disk number
		if(piDrvNum)
			*piDrvNum=drv[0]-_T('A');

		// disk type
		if(puiDrvType)
		{
			*puiDrvType=GetDriveType(drv);
			if(*puiDrvType == DRIVE_NO_ROOT_DIR)
				*puiDrvType=DRIVE_UNKNOWN;
		}
	}
	else
	{
		// there's no disk in a path
		if(piDrvNum)
			*piDrvNum=-1;

		if(puiDrvType)
		{
			// check for unc path
			if(_tcsncmp(spPath.ToString(), _T("\\\\"), 2) == 0)
				*puiDrvType=DRIVE_REMOTE;
			else
				*puiDrvType=DRIVE_UNKNOWN;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CClipboardEntry

CClipboardEntry::CClipboardEntry() :
	m_bMove(true),
	m_iDriveNumber(-2),
	m_iBufferIndex(-1)
{
}

CClipboardEntry::CClipboardEntry(const CClipboardEntry& rEntry) :
	m_path(rEntry.m_path),
	m_bMove(rEntry.m_bMove),
	m_iDriveNumber(rEntry.m_iDriveNumber),
	m_pathDst(rEntry.m_pathDst)
{
}

void CClipboardEntry::SetPath(const chcore::TSmartPath& tPath)
{
	m_path = tPath;			// guaranteed without ending '\\'
	m_path.CutIfExists(_T("\\"), false);
}

chcore::TSmartPath CClipboardEntry::GetFileName() const
{
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(m_path.ToString(), NULL, NULL, szName, szExt);
	return chcore::PathFromString(CString(szName) + szExt);
}

int CClipboardEntry::GetDriveNumber()
{
	if(m_iDriveNumber == -2)
		GetDriveData(m_path, &m_iDriveNumber, NULL);

	return m_iDriveNumber;
}

int CClipboardEntry::GetBufferIndex(const chcore::TSmartPath& dpDestPath)
{
	if(m_iBufferIndex == -1)
	{
		int iDriveNumber = 0;
		UINT uiDriveType = 0;
		int iDstDriveNumber = 0;
		UINT uiDstDriveType = 0;
		GetDriveData(m_path, &iDriveNumber, &uiDriveType);
		GetDriveData(dpDestPath, &iDstDriveNumber, &uiDstDriveType);

		// what kind of buffer
		if(uiDriveType == DRIVE_REMOTE || uiDstDriveType == DRIVE_REMOTE)
			m_iBufferIndex = BI_LAN;
		else if(uiDriveType == DRIVE_CDROM || uiDstDriveType == DRIVE_CDROM)
			m_iBufferIndex = BI_CD;
		else if(uiDriveType == DRIVE_FIXED && uiDstDriveType == DRIVE_FIXED)
		{
			// two hdd's - is this the same physical disk ?
			if(iDriveNumber == iDstDriveNumber || IsSamePhysicalDisk(iDriveNumber, iDstDriveNumber))
				m_iBufferIndex = BI_ONEDISK;
			else
				m_iBufferIndex = BI_TWODISKS;
		}
		else
			m_iBufferIndex = BI_DEFAULT;
	}

	return m_iBufferIndex;
}

void CClipboardEntry::SetDestinationPath(const chcore::TSmartPath& tPath)
{
	m_pathDst = tPath;
}

chcore::TSmartPath CClipboardEntry::GetDestinationPath() const
{
	return m_pathDst;
}

//////////////////////////////////////////////////////////////////////////////
// CClipboardArray

CClipboardArray::CClipboardArray()
{
}

CClipboardArray::~CClipboardArray()
{
	RemoveAll();
}

CClipboardArray::CClipboardArray(const CClipboardArray& rSrc) :
	m_vEntries(rSrc.m_vEntries)
{
}

CClipboardArray& CClipboardArray::operator=(const CClipboardArray& rSrc)
{
	if(this != &rSrc)
	{
		m_vEntries = rSrc.m_vEntries;
	}

	return *this;
}

CClipboardEntryPtr CClipboardArray::GetAt(size_t stPos) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	
	if(stPos >= m_vEntries.size())
		THROW(_T("Out of range"), 0, 0, 0);

	return m_vEntries.at(stPos);
}

void CClipboardArray::SetAt(size_t stIndex, const CClipboardEntryPtr& spEntry)
{
	if(!spEntry)
		THROW(_T("Invalid argument"), 0, 0, 0);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	
	if(stIndex >= m_vEntries.size())
		THROW(_T("Out of range"), 0, 0, 0);
	
	m_vEntries[stIndex] = spEntry;
}

void CClipboardArray::Add(const CClipboardEntryPtr& spEntry)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vEntries.push_back(spEntry);
}

void CClipboardArray::RemoveAt(size_t nIndex, size_t nCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vEntries.erase(m_vEntries.begin() + nIndex, m_vEntries.begin() + nIndex + nCount);
}

void CClipboardArray::RemoveAll()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vEntries.clear();
}

size_t CClipboardArray::GetSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vEntries.size();
}

int CClipboardArray::ReplacePathsPrefix(CString strOld, CString strNew)
{
	// small chars to make comparing case insensitive
	strOld.MakeLower();

	CString strText;
	int iOffset = 0;
	int iCount = 0;
	
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	BOOST_FOREACH(CClipboardEntryPtr& spEntry, m_vEntries)
	{
		strText = spEntry->GetPath().ToString();
		strText.MakeLower();
		iOffset = strText.Find(strOld, 0);
		if(iOffset != -1)
		{
			// found
			strText = spEntry->GetPath().ToString();
			strText = strText.Left(iOffset) + strNew + strText.Mid(iOffset + strOld.GetLength());
			spEntry->SetPath(chcore::PathFromString(strText));

			++iCount;
		}
	}

	return iCount;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileInfo::CFileInfo() :
	m_pClipboard(NULL),
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
	m_pClipboard(finf.m_pClipboard)
{
}

CFileInfo::~CFileInfo()
{
}

bool CFileInfo::Exist(chcore::TSmartPath pathToCheck)
{
	WIN32_FIND_DATA fd;
	
	// search by exact name
	HANDLE hFind = FindFirstFile(pathToCheck.ToString(), &fd);
	if(hFind != INVALID_HANDLE_VALUE)
		return true;

	// another try (add '\\' if needed and '*' for marking that we look for ie. c:\*
	// instead of c:\, which would never be found prev. way)
	pathToCheck.AppendIfNotExists(_T("\\"), false);
	pathToCheck.AppendIfNotExists(_T("*"), false);

	hFind = FindFirstFile(pathToCheck.ToString(), &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
		return true;
	}
	else
		return false;
}

void CFileInfo::Create(const WIN32_FIND_DATA* pwfd, const chcore::TSmartPath& pathFile, size_t stSrcIndex)
{
	BOOST_ASSERT(stSrcIndex == std::numeric_limits<size_t>::max() || m_pClipboard);
	if(stSrcIndex != std::numeric_limits<size_t>::max() && !m_pClipboard)
		THROW(_t("Internal error: pointer not initialized."), 0, 0, 0);

	// copy data from W32_F_D
	m_pathFile = pathFile + chcore::PathFromString(pwfd->cFileName);

	// if proper index has been passed - reduce the path
	if(m_pClipboard && stSrcIndex >= 0)
		m_pathFile.MakeRelativePath(m_pClipboard->GetAt(stSrcIndex)->GetPath());	// cut path from clipboard

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
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath() + m_pathFile : m_pathFile;

	TCHAR szDrive[_MAX_DRIVE];
	_tsplitpath(pathCombined.ToString(), szDrive, NULL, NULL, NULL);
	return chcore::PathFromString(szDrive);
}

int CFileInfo::GetDriveNumber()
{
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	if(m_stSrcIndex != std::numeric_limits<size_t>::max())
	{
		// read data stored in CClipboardEntry
		return m_pClipboard->GetAt(m_stSrcIndex)->GetDriveNumber();
	}
	else
	{
		// manually
		int iNum = 0;
		GetDriveData(m_pathFile, &iNum, NULL);
		return iNum;
	}
}

chcore::TSmartPath CFileInfo::GetFileDir() const
{ 
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath() + m_pathFile : m_pathFile;

	TCHAR szDir[_MAX_DIR];
	_tsplitpath(pathCombined.ToString(), NULL, szDir, NULL, NULL);
	return chcore::PathFromString(szDir);
}

chcore::TSmartPath CFileInfo::GetFileTitle() const
{
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath()+m_pathFile : m_pathFile;
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(pathCombined.ToString(), NULL, NULL, szName, NULL);
	return chcore::PathFromString(szName);
}

chcore::TSmartPath CFileInfo::GetFileExt() const
{
	ASSERT(m_pClipboard);
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath()+m_pathFile : m_pathFile;
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(pathCombined.ToString(), NULL, NULL, NULL, szExt);
	return chcore::PathFromString(szExt);
}

chcore::TSmartPath CFileInfo::GetFileRoot() const
{
	ASSERT(m_pClipboard);
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined = (m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath() + m_pathFile : m_pathFile;

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(pathCombined.ToString(), szDrive, szDir, NULL, NULL);
	return chcore::PathFromString(szDrive) + chcore::PathFromString(szDir);
}

chcore::TSmartPath CFileInfo::GetFileName() const
{
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathCombined;
	if(m_pClipboard && m_stSrcIndex != std::numeric_limits<size_t>::max())
		pathCombined = m_pClipboard->GetAt(m_stSrcIndex)->GetPath() + m_pathFile;
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
		BOOST_ASSERT(m_pClipboard);
		if(!m_pClipboard)
			THROW(_T("Invalid pointer"), 0, 0, 0);

		chcore::TSmartPath pathCombined = m_pClipboard->GetAt(m_stSrcIndex)->GetPath();
		pathCombined += m_pathFile;
		return pathCombined;
	}
	else
		return m_pathFile;
}

int CFileInfo::GetBufferIndex(const chcore::TSmartPath& dpDestPath) const
{
	if(m_stSrcIndex != std::numeric_limits<size_t>::max())
		return m_pClipboard->GetAt(m_stSrcIndex)->GetBufferIndex(dpDestPath);
	else
		return BI_DEFAULT;
}

///////////////////////////////////////////////////////////////////////
// Array
CFileInfoArray::CFileInfoArray(const CClipboardArray& rClipboardArray) :
	m_rClipboard(rClipboardArray)
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

int CFileInfoArray::GetBufferIndexAt(size_t stIndex, const chcore::TSmartPath& dpDestPath) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	if(stIndex >= m_vFiles.size())
		return 0;
	else
	{
		const CFileInfoPtr& spFileInfo = m_vFiles[stIndex];
		if(!spFileInfo)
			THROW(_T("Invalid pointer"), 0, 0, 0);

		return spFileInfo->GetBufferIndex(dpDestPath);
	}
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
