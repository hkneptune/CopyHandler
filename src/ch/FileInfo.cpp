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
#include "resource.h"
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

void GetDriveData(LPCTSTR lpszPath, int* piDrvNum, UINT* puiDrvType)
{
	TCHAR drv[_MAX_DRIVE+1];

	_tsplitpath(lpszPath, drv, NULL, NULL, NULL);
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
			if(_tcsncmp(lpszPath, _T("\\\\"), 2) == 0)
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
	m_strPath(rEntry.m_strPath),
	m_bMove(rEntry.m_bMove),
	m_iDriveNumber(rEntry.m_iDriveNumber),
	m_strDstPath(rEntry.m_strDstPath)
{
}

void CClipboardEntry::SetPath(const CString& strPath)
{
	m_strPath = strPath;			// guaranteed without ending '\\'
	if(m_strPath.Right(1) == _T('\\'))
		m_strPath = m_strPath.Left(m_strPath.GetLength() - 1);
}

CString CClipboardEntry::GetFileName() const
{
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(m_strPath, NULL, NULL, szName, szExt);
	return CString(szName) + szExt;
}

int CClipboardEntry::GetDriveNumber()
{
	if(m_iDriveNumber == -2)
		GetDriveData(m_strPath, &m_iDriveNumber, NULL);

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
		GetDriveData(m_strPath, &iDriveNumber, &uiDriveType);
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

void CClipboardEntry::SetDestinationPath(const CString& strPath)
{
	m_strDstPath = strPath;
}

CString CClipboardEntry::GetDestinationPath() const
{
	return m_strDstPath;
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
		strText = spEntry->GetPath();
		strText.MakeLower();
		iOffset = strText.Find(strOld, 0);
		if(iOffset != -1)
		{
			// found
			strText = spEntry->GetPath();
			strText = strText.Left(iOffset) + strNew + strText.Mid(iOffset + strOld.GetLength());
			spEntry->SetPath(strText);

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
	m_strFilePath(),
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
	m_strFilePath(finf.m_strFilePath),
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
	HANDLE hFind = FindFirstFile(pathToCheck, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
		return true;

	// another try (add '\\' if needed and '*' for marking that we look for ie. c:\*
	// instead of c:\, which would never be found prev. way)
	pathToCheck.AppendIfNotExists(_T("\\"), false);
	pathToCheck.AppendIfNotExists(_T("*"), false);

	hFind = FindFirstFile(pathToCheck, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
		return true;
	}
	else
		return false;
}

void CFileInfo::Create(const WIN32_FIND_DATA* pwfd, LPCTSTR pszFilePath, size_t stSrcIndex)
{
	BOOST_ASSERT(stSrcIndex == std::numeric_limits<size_t>::max() || m_pClipboard);
	if(stSrcIndex != std::numeric_limits<size_t>::max() && !m_pClipboard)
		THROW(_t("Internal error: pointer not initialized."), 0, 0, 0);

	// copy data from W32_F_D
	m_strFilePath = CString(pszFilePath) + pwfd->cFileName;

	// if proper index has been passed - reduce the path
	if(m_pClipboard && stSrcIndex >= 0)
		m_strFilePath=m_strFilePath.Mid(m_pClipboard->GetAt(stSrcIndex)->GetPath().GetLength());	// cut path from clipboard

	m_stSrcIndex = stSrcIndex;
	m_dwAttributes = pwfd->dwFileAttributes;
	m_uhFileSize = (((ULONGLONG) pwfd->nFileSizeHigh) << 32) + pwfd->nFileSizeLow;
	m_ftCreation = pwfd->ftCreationTime;
	m_ftLastAccess = pwfd->ftLastAccessTime;
	m_ftLastWrite = pwfd->ftLastWriteTime;
	m_uiFlags = 0;
}

bool CFileInfo::Create(CString strFilePath, size_t stSrcIndex)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	int nBarPos;
	
	hFind = FindFirstFile(strFilePath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);

		// add data to members
		nBarPos = strFilePath.ReverseFind(TCHAR('\\'));
		Create(&wfd, strFilePath.Left(nBarPos+1), stSrcIndex);
		
		return true;
	}
	else
	{
		m_strFilePath=GetResManager().LoadString(IDS_NOTFOUND_STRING);
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

CString CFileInfo::GetFileDrive() const
{
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	CString strPath=(m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath() + m_strFilePath : m_strFilePath;
	TCHAR szDrive[_MAX_DRIVE];
	_tsplitpath(strPath, szDrive, NULL, NULL, NULL);
	return CString(szDrive);
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
		GetDriveData(m_strFilePath, &iNum, NULL);
		return iNum;
	}
}

CString CFileInfo::GetFileDir() const
{ 
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	CString strPath=(m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(strPath, NULL, szDir, NULL, NULL);
	return CString(szDir);
}

CString CFileInfo::GetFileTitle() const
{
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	CString strPath=(m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(strPath, NULL, NULL, szName, NULL);
	return CString(szName);
}

CString CFileInfo::GetFileExt() const
{
	ASSERT(m_pClipboard);
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	CString strPath=(m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(strPath, NULL, NULL, NULL, szExt);
	return CString(szExt);
}

CString CFileInfo::GetFileRoot() const
{
	ASSERT(m_pClipboard);
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	CString strPath=(m_stSrcIndex != std::numeric_limits<size_t>::max()) ? m_pClipboard->GetAt(m_stSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(strPath, szDrive, szDir, NULL, NULL);
	return CString(szDrive)+szDir;
}

CString CFileInfo::GetFileName() const
{
	BOOST_ASSERT(m_pClipboard);
	if(!m_pClipboard)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	CString strPath;
	if(m_pClipboard && m_stSrcIndex != std::numeric_limits<size_t>::max())
		strPath = m_pClipboard->GetAt(m_stSrcIndex)->GetPath() + m_strFilePath;
	else
	{
		ASSERT(m_stSrcIndex == std::numeric_limits<size_t>::max());
		strPath = m_strFilePath;
	}

	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(strPath, NULL, NULL, szName, szExt);
	return CString(szName)+szExt;
}

bool CFileInfo::operator==(const CFileInfo& rInfo)
{
	return (rInfo.m_dwAttributes == m_dwAttributes && rInfo.m_ftCreation.dwHighDateTime == m_ftCreation.dwHighDateTime && rInfo.m_ftCreation.dwLowDateTime == m_ftCreation.dwLowDateTime
		&& rInfo.m_ftLastWrite.dwHighDateTime == m_ftLastWrite.dwHighDateTime && rInfo.m_ftLastWrite.dwLowDateTime == m_ftLastWrite.dwLowDateTime && rInfo.m_uhFileSize == m_uhFileSize);
}

CString CFileInfo::GetFullFilePath() const
{
	CString strPath;
	if(m_stSrcIndex != std::numeric_limits<size_t>::max())
	{
		BOOST_ASSERT(m_pClipboard);
		if(!m_pClipboard)
			THROW(_T("Invalid pointer"), 0, 0, 0);

		strPath += m_pClipboard->GetAt(m_stSrcIndex)->GetPath();
	}
	strPath += m_strFilePath;

	return strPath;
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
