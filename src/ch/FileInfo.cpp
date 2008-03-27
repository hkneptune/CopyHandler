/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
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
#include "resource.h"
#include "DataBuffer.h"
#include "Device IO.h"
#include "imagehlp.h"
#include "ch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// finds another name for a copy of src file(folder) in dest location
void FindFreeSubstituteName(CString strSrcPath, CString strDstPath, CString* pstrResult)
{
	// get the name from srcpath
	if (strSrcPath.Right(1) == _T("\\"))
		strSrcPath=strSrcPath.Left(strSrcPath.GetLength()-1);

	int iBarPos=strSrcPath.ReverseFind(_T('\\'));
	CString strFolderName;
	if (iBarPos != -1)
		strFolderName=strSrcPath.Mid(iBarPos+1);
	else
		strFolderName=strSrcPath;	// it shouldn't happen at all

	if (strDstPath.Right(1) != _T("\\"))
		strDstPath+=_T("\\");

	// set the dest path
	CString strCheckPath;
	ictranslate::CFormat fmt(GetResManager()->LoadString(IDS_FIRSTCOPY_STRING));
	fmt.SetParam(_t("%name"), strFolderName);
	strCheckPath = fmt;
	if (strCheckPath.GetLength() > _MAX_PATH)
		strCheckPath=strCheckPath.Left(_MAX_PATH);	// max - 260 chars

	// when adding to strDstPath check if the path already exists - if so - try again
	int iCounter=1;
	CString strFmt = GetResManager()->LoadString(IDS_NEXTCOPY_STRING);
	while (CFileInfo::Exist(strDstPath+strCheckPath))
	{
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%name"), strFolderName);
		fmt.SetParam(_t("%count"), ++iCounter);
		strCheckPath = fmt;
	}

	*pstrResult=strCheckPath;
}

bool _tcicmp(TCHAR c1, TCHAR c2)
{
	TCHAR ch1[2]={c1, 0}, ch2[2]={c2, 0};
	return (_tcsicmp(ch1, ch2) == 0);
}

////////////////////////////////////////////////////////////////////////////
CFileFilter::CFileFilter()
{
	// files mask
	m_bUseMask=false;
	m_astrMask.RemoveAll();

	m_bUseExcludeMask=false;
	m_astrExcludeMask.RemoveAll();

	// size filtering
	m_bUseSize=false;
	m_iSizeType1=GT;
	m_ullSize1=0;
	m_bUseSize2=false;
	m_iSizeType2=LT;
	m_ullSize2=0;

	// date filtering
	m_bUseDate=false;
	m_iDateType=DATE_CREATED;
	m_iDateType1=GT;
	m_bDate1=false;
	m_tDate1=CTime::GetCurrentTime();
	m_bTime1=false;
	m_tTime1=CTime::GetCurrentTime();

	m_bUseDate2=false;
	m_iDateType2=LT;
	m_bDate2=false;
	m_tDate2=CTime::GetCurrentTime();
	m_bTime2=false;
	m_tTime2=CTime::GetCurrentTime();

	// attribute filtering
	m_bUseAttributes=false;
	m_iArchive=2;
	m_iReadOnly=2;
	m_iHidden=2;
	m_iSystem=2;
	m_iDirectory=2;
}

CFileFilter::CFileFilter(const CFileFilter& rFilter)
{
	*this=rFilter;
}

CFileFilter& CFileFilter::operator=(const CFileFilter& rFilter)
{
	// files mask
	m_bUseMask=rFilter.m_bUseMask;
	m_astrMask.Copy(rFilter.m_astrMask);

	m_bUseExcludeMask=rFilter.m_bUseExcludeMask;
	m_astrExcludeMask.Copy(rFilter.m_astrExcludeMask);

	// size filtering
	m_bUseSize=rFilter.m_bUseSize;
	m_iSizeType1=rFilter.m_iSizeType1;
	m_ullSize1=rFilter.m_ullSize1;
	m_bUseSize2=rFilter.m_bUseSize2;
	m_iSizeType2=rFilter.m_iSizeType2;
	m_ullSize2=rFilter.m_ullSize2;

	// date filtering
	m_bUseDate=rFilter.m_bUseDate;
	m_iDateType=rFilter.m_iDateType;
	m_iDateType1=rFilter.m_iDateType1;
	m_bDate1=rFilter.m_bDate1;
	m_tDate1=rFilter.m_tDate1;
	m_bTime1=rFilter.m_bTime1;
	m_tTime1=rFilter.m_tTime1;

	m_bUseDate2=rFilter.m_bUseDate2;
	m_iDateType2=rFilter.m_iDateType2;
	m_bDate2=rFilter.m_bDate2;
	m_tDate2=rFilter.m_tDate2;
	m_bTime2=rFilter.m_bTime2;
	m_tTime2=rFilter.m_tTime2;

	// attribute filtering
	m_bUseAttributes=rFilter.m_bUseAttributes;
	m_iArchive=rFilter.m_iArchive;
	m_iReadOnly=rFilter.m_iReadOnly;
	m_iHidden=rFilter.m_iHidden;
	m_iSystem=rFilter.m_iSystem;
	m_iDirectory=rFilter.m_iDirectory;

	return *this;
}

void CFileFilter::Serialize(CArchive& ar)
{
	ULARGE_INTEGER li;
	if (ar.IsStoring())
	{
		// store
		// files mask
		ar<<static_cast<unsigned char>(m_bUseMask);
		m_astrMask.Serialize(ar);

		ar<<static_cast<unsigned char>(m_bUseExcludeMask);
		m_astrExcludeMask.Serialize(ar);
		
		// size filtering
		ar<<static_cast<unsigned char>(m_bUseSize);
		ar<<m_iSizeType1;
		li.QuadPart=m_ullSize1;
		ar<<li.LowPart;
		ar<<li.HighPart;
		ar<<static_cast<unsigned char>(m_bUseSize2);
		ar<<m_iSizeType2;
		li.QuadPart=m_ullSize2;
		ar<<li.LowPart;
		ar<<li.HighPart;
		
		// date
		ar<<static_cast<unsigned char>(m_bUseDate);
		ar<<m_iDateType;
		ar<<m_iDateType1;
		ar<<static_cast<unsigned char>(m_bDate1);
		ar<<m_tDate1;
		ar<<static_cast<unsigned char>(m_bTime1);
		ar<<m_tTime1;
		
		ar<<static_cast<unsigned char>(m_bUseDate2);
		ar<<m_iDateType2;
		ar<<static_cast<unsigned char>(m_bDate2);
		ar<<m_tDate2;
		ar<<static_cast<unsigned char>(m_bTime2);
		ar<<m_tTime2;
		
		// attributes
		ar<<static_cast<unsigned char>(m_bUseAttributes);
		ar<<m_iArchive;
		ar<<m_iReadOnly;
		ar<<m_iHidden;
		ar<<m_iSystem;
		ar<<m_iDirectory;
	}
	else
	{
		// read
		unsigned char tmp;

		// files mask
		ar>>tmp;
		m_bUseMask=(tmp != 0);
		m_astrMask.Serialize(ar);
		
		ar>>tmp;
		m_bUseExcludeMask=(tmp != 0);
		m_astrExcludeMask.Serialize(ar);

		// size
		ar>>tmp;
		m_bUseSize=(tmp != 0);
		ar>>m_iSizeType1;
		ar>>li.LowPart;
		ar>>li.HighPart;
		m_ullSize1=li.QuadPart;
		ar>>tmp;
		m_bUseSize2=(tmp != 0);
		ar>>m_iSizeType2;
		ar>>li.LowPart;
		ar>>li.HighPart;
		m_ullSize2=li.QuadPart;
		
		// date
		ar>>tmp;
		m_bUseDate=(tmp != 0);
		ar>>m_iDateType;
		ar>>m_iDateType1;
		ar>>tmp;
		m_bDate1=(tmp != 0);
		ar>>m_tDate1;
		ar>>tmp;
		m_bTime1=(tmp != 0);
		ar>>m_tTime1;
		
		ar>>tmp;
		m_bUseDate2=(tmp != 0);
		ar>>m_iDateType2;
		ar>>tmp;
		m_bDate2=(tmp != 0);
		ar>>m_tDate2;
		ar>>tmp;
		m_bTime2=(tmp != 0);
		ar>>m_tTime2;
		
		// attributes
		ar>>tmp;
		m_bUseAttributes=(tmp != 0);
		ar>>m_iArchive;
		ar>>m_iReadOnly;
		ar>>m_iHidden;
		ar>>m_iSystem;
		ar>>m_iDirectory;
	}
}

CString& CFileFilter::GetCombinedMask(CString& pMask) const
{
	pMask.Empty();
	if (m_astrMask.GetSize() > 0)
	{
		pMask=m_astrMask.GetAt(0);
		for (int i=1;i<m_astrMask.GetSize();i++)
			pMask+=_T("|")+m_astrMask.GetAt(i);
	}

	return pMask;
}

void CFileFilter::SetCombinedMask(const CString& pMask)
{
	m_astrMask.RemoveAll();
	
	TCHAR *pszData=new TCHAR[pMask.GetLength()+1];
	_tcscpy(pszData, pMask);

	TCHAR *szToken=_tcstok(pszData, _T("|"));
	while (szToken != NULL)
	{
		// add token to a table
		m_astrMask.Add(szToken);

		// search for next
		szToken=_tcstok(NULL, _T("|"));
	}

	delete [] pszData;
}

CString& CFileFilter::GetCombinedExcludeMask(CString& pMask) const
{
	pMask.Empty();
	if (m_astrExcludeMask.GetSize() > 0)
	{
		pMask=m_astrExcludeMask.GetAt(0);
		for (int i=1;i<m_astrExcludeMask.GetSize();i++)
			pMask+=_T("|")+m_astrExcludeMask.GetAt(i);
	}

	return pMask;
}

void CFileFilter::SetCombinedExcludeMask(const CString& pMask)
{
	m_astrExcludeMask.RemoveAll();
	
	TCHAR *pszData=new TCHAR[pMask.GetLength()+1];
	_tcscpy(pszData, pMask);

	TCHAR *szToken=_tcstok(pszData, _T("|"));
	while (szToken != NULL)
	{
		// add token
		m_astrExcludeMask.Add(szToken);

		// find next
		szToken=_tcstok(NULL, _T("|"));
	}

	delete [] pszData;
}

bool CFileFilter::Match(const CFileInfo& rInfo) const
{
	// check by mask
	if (m_bUseMask)
	{
		bool bRes=false;
		for (int i=0;i<m_astrMask.GetSize();i++)
		{
			if (MatchMask(m_astrMask.GetAt(i), rInfo.GetFileName()))
				bRes=true;
		}
		if (!bRes)
			return false;
	}

	// excluding mask
	if (m_bUseExcludeMask)
	{
		for (int i=0;i<m_astrExcludeMask.GetSize();i++)
		{
			if (MatchMask(m_astrExcludeMask.GetAt(i), rInfo.GetFileName()))
				return false;
		}
	}

	// by size
	if (m_bUseSize)
	{
		switch (m_iSizeType1)
		{
		case LT:
			if (m_ullSize1 <= rInfo.GetLength64())
				return false;
			break;
		case LE:
			if (m_ullSize1 < rInfo.GetLength64())
				return false;
			break;
		case EQ:
			if (m_ullSize1 != rInfo.GetLength64())
				return false;
			break;
		case GE:
			if (m_ullSize1 > rInfo.GetLength64())
				return false;
			break;
		case GT:
			if (m_ullSize1 >= rInfo.GetLength64())
				return false;
			break;
		}

		// second part
		if (m_bUseSize2)
		{
			switch (m_iSizeType2)
			{
			case LT:
				if (m_ullSize2 <= rInfo.GetLength64())
					return false;
				break;
			case LE:
				if (m_ullSize2 < rInfo.GetLength64())
					return false;
				break;
			case EQ:
				if (m_ullSize2 != rInfo.GetLength64())
					return false;
				break;
			case GE:
				if (m_ullSize2 > rInfo.GetLength64())
					return false;
				break;
			case GT:
				if (m_ullSize2 >= rInfo.GetLength64())
					return false;
				break;
			}
		}
	}

	// date - get the time from rInfo
	if (m_bUseDate)
	{
		COleDateTime tm;
		switch (m_iDateType)
		{
		case DATE_CREATED:
			tm=rInfo.GetCreationTime();
			break;
		case DATE_MODIFIED:
			tm=rInfo.GetLastWriteTime();
			break;
		case DATE_LASTACCESSED:
			tm=rInfo.GetLastAccessTime();
			break;
		}

		// counting...
		unsigned long ulInfo=0, ulCheck=0;
		if (m_bDate1)
		{
			ulInfo=(tm.GetYear()-1970)*32140800+tm.GetMonth()*2678400+tm.GetDay()*86400;
			ulCheck=(m_tDate1.GetYear()-1970)*32140800+m_tDate1.GetMonth()*2678400+m_tDate1.GetDay()*86400;
		}

		if (m_bTime1)
		{
			ulInfo+=tm.GetHour()*3600+tm.GetMinute()*60+tm.GetSecond();
			ulCheck+=m_tTime1.GetHour()*3600+m_tTime1.GetMinute()*60+m_tTime1.GetSecond();
		}

		// ... and comparing
		switch (m_iDateType1)
		{
		case LT:
			if (ulInfo >= ulCheck)
				return false;
			break;
		case LE:
			if (ulInfo > ulCheck)
				return false;
			break;
		case EQ:
			if (ulInfo != ulCheck)
				return false;
			break;
		case GE:
			if (ulInfo < ulCheck)
				return false;
			break;
		case GT:
			if (ulInfo <= ulCheck)
				return false;
			break;
		}

		if (m_bUseDate2)
		{
			// counting...
			ulInfo=0, ulCheck=0;
			if (m_bDate2)
			{
				ulInfo=(tm.GetYear()-1970)*32140800+tm.GetMonth()*2678400+tm.GetDay()*86400;
				ulCheck=(m_tDate2.GetYear()-1970)*32140800+m_tDate2.GetMonth()*2678400+m_tDate2.GetDay()*86400;
			}
			
			if (m_bTime2)
			{
				ulInfo+=tm.GetHour()*3600+tm.GetMinute()*60+tm.GetSecond();
				ulCheck+=m_tTime2.GetHour()*3600+m_tTime2.GetMinute()*60+m_tTime2.GetSecond();
			}
			
			// ... comparing
			switch (m_iDateType2)
			{
			case LT:
				if (ulInfo >= ulCheck)
					return false;
				break;
			case LE:
				if (ulInfo > ulCheck)
					return false;
				break;
			case EQ:
				if (ulInfo != ulCheck)
					return false;
				break;
			case GE:
				if (ulInfo < ulCheck)
					return false;
				break;
			case GT:
				if (ulInfo <= ulCheck)
					return false;
				break;
			}
		}
	} // of m_bUseDate

	// attributes
	if (m_bUseAttributes)
	{
		if ( (m_iArchive == 1 && !rInfo.IsArchived()) || (m_iArchive == 0 && rInfo.IsArchived()))
			return false;
		if ( (m_iReadOnly == 1 && !rInfo.IsReadOnly()) || (m_iReadOnly == 0 && rInfo.IsReadOnly()))
			return false;
		if ( (m_iHidden == 1 && !rInfo.IsHidden()) || (m_iHidden == 0 && rInfo.IsHidden()))
			return false;
		if ( (m_iSystem == 1 && !rInfo.IsSystem()) || (m_iSystem == 0 && rInfo.IsSystem()))
			return false;
		if ( (m_iDirectory == 1 && !rInfo.IsDirectory()) || (m_iDirectory == 0 && rInfo.IsDirectory()))
			return false;
	}

	return true;
}

bool CFileFilter::MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const
{
	bool bMatch = 1;

	//iterate and delete '?' and '*' one by one
	while(*lpszMask != _T('\0') && bMatch && *lpszString != _T('\0'))
	{
		if (*lpszMask == _T('?')) lpszString++;
		else if (*lpszMask == _T('*'))
		{
			bMatch = Scan(lpszMask, lpszString);
			lpszMask--;
		}
		else
		{
			bMatch = _tcicmp(*lpszMask, *lpszString);
			lpszString++;
		}
		lpszMask++;
	}
	while (*lpszMask == _T('*') && bMatch) lpszMask++;

	return bMatch && *lpszString == _T('\0') && *lpszMask == _T('\0');
}

// scan '?' and '*'
bool CFileFilter::Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const
{
	// remove the '?' and '*'
	for(lpszMask++; *lpszString != _T('\0') && (*lpszMask == _T('?') || *lpszMask == _T('*')); lpszMask++)
		if (*lpszMask == _T('?')) lpszString++;
	while ( *lpszMask == _T('*')) lpszMask++;
	
	// if lpszString is empty and lpszMask has more characters or,
	// lpszMask is empty, return 
	if (*lpszString == _T('\0') && *lpszMask != _T('\0')) return false;
	if (*lpszString == _T('\0') && *lpszMask == _T('\0')) return true; 
	// else search substring
	else
	{
		LPCTSTR wdsCopy = lpszMask;
		LPCTSTR lpszStringCopy = lpszString;
		bool bMatch = true;
		do 
		{
			if (!MatchMask(lpszMask, lpszString)) lpszStringCopy++;
			lpszMask = wdsCopy;
			lpszString = lpszStringCopy;
			while (!(_tcicmp(*lpszMask, *lpszString)) && (*lpszString != '\0')) lpszString++;
			wdsCopy = lpszMask;
			lpszStringCopy = lpszString;
		}
		while ((*lpszString != _T('\0')) ? !MatchMask(lpszMask, lpszString) : (bMatch = false) != false);

		if (*lpszString == _T('\0') && *lpszMask == _T('\0')) return true;

		return bMatch;
	}
}

bool CFiltersArray::Match(const CFileInfo& rInfo) const
{
	if (GetSize() == 0)
		return true;

	// if only one of the filters matches - return true
	for (int i=0;i<GetSize();i++)
		if (GetAt(i).Match(rInfo))
			return true;

	return false;
}

void CFiltersArray::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar<<GetSize();
		for (int i=0;i<GetSize();i++)
			GetAt(i).Serialize(ar);
	}
	else
	{
		int iSize;
		CFileFilter ff;

		ar>>iSize;
		SetSize(iSize);
		for (int i=0;i<iSize;i++)
		{
			ff.Serialize(ar);
			SetAt(i, ff);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
CFileInfo::CFileInfo()
{
}

CFileInfo::CFileInfo(const CFileInfo& finf)
{
	m_strFilePath = finf.m_strFilePath;
	m_iSrcIndex=finf.m_iSrcIndex;
	m_dwAttributes = finf.m_dwAttributes;
	m_uhFileSize = finf.m_uhFileSize;
	m_timCreation = finf.m_timCreation;
	m_timLastAccess = finf.m_timLastAccess;
	m_timLastWrite = finf.m_timLastWrite;
	m_uiFlags = finf.m_uiFlags;

	m_pClipboard=finf.m_pClipboard;
}

CFileInfo::~CFileInfo()
{
}

bool CFileInfo::Exist(CString strPath)
{
	WIN32_FIND_DATA fd;
	
	// search by exact name
	HANDLE hFind = FindFirstFile(strPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
		return true;

	// another try (add '\\' if needed and '*' for marking that we look for ie. c:\*
	// instead of c:\, which would never be found prev. way)
	if (strPath.Right(1) != _T("\\"))
		strPath+=_T("\\*");
	else
		strPath+=_T("*");

	hFind = FindFirstFile(strPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
		return true;
	else
		return false;
}

void CFileInfo::Create(const WIN32_FIND_DATA* pwfd, LPCTSTR pszFilePath, int iSrcIndex)
{
	ASSERT(m_pClipboard);

	// copy data from W32_F_D
	m_strFilePath = CString(pszFilePath) + pwfd->cFileName;
	
	// if proper index has been passed - reduce the path
	if (iSrcIndex >= 0)
		m_strFilePath=m_strFilePath.Mid(m_pClipboard->GetAt(iSrcIndex)->GetPath().GetLength());	// wytnij œcie¿kê z clipboarda

	m_iSrcIndex=iSrcIndex;
	m_dwAttributes = pwfd->dwFileAttributes;
	m_uhFileSize = (((ULONGLONG) pwfd->nFileSizeHigh) << 32) + pwfd->nFileSizeLow;
	m_timCreation = pwfd->ftCreationTime;
	m_timLastAccess = pwfd->ftLastAccessTime;
	m_timLastWrite = pwfd->ftLastWriteTime;
	m_uiFlags = 0;
}

bool CFileInfo::Create(CString strFilePath, int iSrcIndex)
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
		Create(&wfd, strFilePath.Left(nBarPos+1), iSrcIndex);
		
		return true;
	}
	else
	{
		m_strFilePath=GetResManager()->LoadString(IDS_NOTFOUND_STRING);
		m_iSrcIndex=-1;
		m_dwAttributes = (DWORD)-1;
		m_uhFileSize = (unsigned __int64)-1;
		m_timCreation.SetDateTime(1900, 1, 1, 0, 0, 0);
		m_timLastAccess.SetDateTime(1900, 1, 1, 0, 0, 0);
		m_timLastWrite.SetDateTime(1900, 1, 1, 0, 0, 0);
		m_uiFlags = 0;
		return false;
	}
}

CString CFileInfo::GetFileDrive(void) const
{
	ASSERT(m_pClipboard);

	CString strPath=(m_iSrcIndex != -1) ? m_pClipboard->GetAt(m_iSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szDrive[_MAX_DRIVE];
	_tsplitpath(strPath, szDrive, NULL, NULL, NULL);
	return CString(szDrive);
}

int CFileInfo::GetDriveNumber() const
{
	ASSERT(m_pClipboard);

	if (m_iSrcIndex != -1)
	{
		// read data stored in CClipboardEntry
		return m_pClipboard->GetAt(m_iSrcIndex)->GetDriveNumber();
	}
	else
	{
		// manually
		int iNum;
		GetDriveData(m_strFilePath, &iNum, NULL);
		return iNum;
	}
}

UINT CFileInfo::GetDriveType() const
{
	ASSERT(m_pClipboard);

	if (m_iSrcIndex != -1)
	{
		// read data contained in CClipboardEntry
		return m_pClipboard->GetAt(m_iSrcIndex)->GetDriveType();
	}
	else
	{
		// manually
		UINT uiType;
		GetDriveData(m_strFilePath, NULL, &uiType);
		return uiType;
	}
}

CString CFileInfo::GetFileDir(void) const
{ 
	ASSERT(m_pClipboard);

	CString strPath=(m_iSrcIndex != -1) ? m_pClipboard->GetAt(m_iSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(strPath, NULL, szDir,NULL, NULL);
	return CString(szDir);
}

CString CFileInfo::GetFileTitle(void) const
{
	ASSERT(m_pClipboard);

	CString strPath=(m_iSrcIndex != -1) ? m_pClipboard->GetAt(m_iSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(strPath, NULL, NULL, szName, NULL);
	return CString(szName);
}

CString CFileInfo::GetFileExt(void) const
{
	ASSERT(m_pClipboard);

	CString strPath=(m_iSrcIndex != -1) ? m_pClipboard->GetAt(m_iSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(strPath, NULL, NULL, NULL, szExt);
	return CString(szExt);
}

CString CFileInfo::GetFileRoot(void) const
{
	ASSERT(m_pClipboard);

	CString strPath=(m_iSrcIndex != -1) ? m_pClipboard->GetAt(m_iSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(strPath, szDrive, szDir, NULL, NULL);
	return CString(szDrive)+szDir;
}

CString CFileInfo::GetFileName(void) const
{
	ASSERT(m_pClipboard);

	CString strPath=(m_iSrcIndex != -1) ? m_pClipboard->GetAt(m_iSrcIndex)->GetPath()+m_strFilePath : m_strFilePath;

	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(strPath, NULL, NULL, szName, szExt);
	return CString(szName)+szExt;
}

void CFileInfo::Store(CArchive& ar)
{
	ar<<m_strFilePath;
	ar<<m_iSrcIndex;
	ar<<m_dwAttributes;
	ar<<static_cast<unsigned long>((m_uhFileSize & 0xFFFFFFFF00000000) >> 32);
	ar<<static_cast<unsigned long>(m_uhFileSize & 0x00000000FFFFFFFF);
	ar<<m_timCreation;
	ar<<m_timLastAccess;
	ar<<m_timLastWrite;
}

void CFileInfo::Load(CArchive& ar)
{
	ar>>m_strFilePath;
	ar>>m_iSrcIndex;
	ar>>m_dwAttributes;
	unsigned long part;
	ar>>part;
	m_uhFileSize=(static_cast<unsigned __int64>(part) << 32);
	ar>>part;
	m_uhFileSize+=part;
	ar>>m_timCreation;
	ar>>m_timLastAccess;
	ar>>m_timLastWrite;
	m_uiFlags = 0;
}

bool CFileInfo::operator==(const CFileInfo& rInfo)
{
	return (rInfo.m_dwAttributes == m_dwAttributes && rInfo.m_timCreation == m_timCreation
		&& rInfo.m_timLastWrite == m_timLastWrite && rInfo.m_uhFileSize == m_uhFileSize);
}

CString CFileInfo::GetDestinationPath(CString strPath, unsigned char ucCopyNumber, int iFlags)
{
	// add '\\'
	if (strPath.Right(1) != _T("\\"))
		strPath+=_T("\\");

	// iFlags: bit 0-ignore folders; bit 1-force creating directories
	if (iFlags & 0x02)
	{
		// force create directories
		TCHAR dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_tsplitpath(GetFullFilePath(), NULL, dir, fname, ext);
		
		CString str=dir;
		str.TrimLeft(_T("\\"));

		// force create directory
//		AfxMessageBox("Created multiple level of paths for %s"+strPath+str);
		SHCreateDirectoryEx(NULL, strPath+str, NULL);
//		MakeSureDirectoryPathExists(strPath+str);

//		AfxMessageBox(strPath+str+fname+CString(ext));
		return strPath+str+fname+CString(ext);
	}
	else
	{
		if (!(iFlags & 0x01) && m_iSrcIndex != -1)
		{
			// generate new dest name
			while (ucCopyNumber >= m_pClipboard->GetAt(m_iSrcIndex)->m_astrDstPaths.GetSize())
			{
				CString strNewPath;
				FindFreeSubstituteName(GetFullFilePath(), strPath, &strNewPath);
				m_pClipboard->GetAt(m_iSrcIndex)->m_astrDstPaths.Add(strNewPath);
			}
			
			return strPath+m_pClipboard->GetAt(m_iSrcIndex)->m_astrDstPaths.GetAt(ucCopyNumber)+m_strFilePath;
		}
		else
			return strPath+GetFileName();
	}
}

CString CFileInfo::GetFullFilePath() const
{
	CString strPath;
	if (m_iSrcIndex >= 0)
	{
		ASSERT(m_pClipboard);
		strPath+=m_pClipboard->GetAt(m_iSrcIndex)->GetPath();
	}
	strPath+=m_strFilePath;

	return strPath;
}

int CFileInfo::GetBufferIndex() const
{
	if (m_iSrcIndex != -1)
		return m_pClipboard->GetAt(m_iSrcIndex)->GetBufferIndex();
	else
		return BI_DEFAULT;
}

///////////////////////////////////////////////////////////////////////
// Array

void CFileInfoArray::AddDir(CString strDirName, const CFiltersArray* pFilters, int iSrcIndex,
							const bool bRecurse, const bool bIncludeDirs,
							const volatile bool* pbAbort)
{ 
	WIN32_FIND_DATA wfd;
	CString strText;
	
	// init CFileInfo
	CFileInfo finf;
	finf.SetClipboard(m_pClipboard);	// this is the link table (CClipboardArray)
	
	// append '\\' at the end of path if needed
	if (strDirName.Right(1) != _T("\\"))
		strDirName+=_T("\\");
	
	strText = strDirName + _T("*");
	// Iterate through dirs & files
	HANDLE hFind = FindFirstFile(strText, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				finf.Create(&wfd, strDirName, iSrcIndex);
				if (pFilters->Match(finf))
					Add(finf);
			}
			else if ( _tcscmp(wfd.cFileName, _T(".")) != 0 && _tcscmp(wfd.cFileName, _T("..")) != 0)
			{
				if (bIncludeDirs)
				{
					// Add directory itself
					finf.Create(&wfd, strDirName, iSrcIndex);
					Add(finf);
				}
				if (bRecurse)
				{
					strText = strDirName + wfd.cFileName+_T("\\");
					// Recurse Dirs
					AddDir(strText, pFilters, iSrcIndex, bRecurse, bIncludeDirs, pbAbort);
				}
			}
		}
		while (((pbAbort == NULL) || (!*pbAbort)) && (FindNextFile(hFind, &wfd)));
		
		FindClose(hFind);
	}
}

int CFileInfoArray::AddFile(CString strFilePath, int iSrcIndex)
{
   CFileInfo finf;

   // CUSTOMIZATION3 - cut '\\' at the end of strFilePath, set relative path
   if (strFilePath.Right(1) == _T("\\"))
		strFilePath=strFilePath.Left(strFilePath.GetLength()-1);

   finf.Create(strFilePath, iSrcIndex);
   return Add(finf);
}


//////////////////////////////////////////////////////////////////////////////
// CClipboardArray

void CClipboardArray::Serialize(CArchive& ar, bool bData)
{
	if (ar.IsStoring())
	{
		// write data
		int iSize=GetSize();
		ar<<iSize;
		for (int i=0;i<iSize;i++)
			GetAt(i)->Serialize(ar, bData);
	}
	else
	{
		int iSize;
		ar>>iSize;

		CClipboardEntry* pEntry;
		for (int i=0;i<iSize;i++)
		{
			if (i < GetSize())
				pEntry=GetAt(i);
			else
			{
				pEntry=new CClipboardEntry;
				Add(pEntry);
			}

			pEntry->Serialize(ar, bData);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CClipboardEntry

void CClipboardEntry::SetPath(const CString& strPath)
{
	m_strPath=strPath;			// guaranteed without ending '\\' 
	if (m_strPath.Right(1) == _T('\\'))
		m_strPath=m_strPath.Left(m_strPath.GetLength()-1);

	GetDriveData(m_strPath, &m_iDriveNumber, &m_uiDriveType);
}

void CClipboardEntry::CalcBufferIndex(const CDestPath& dpDestPath)
{
	// what kind of buffer
	if (m_uiDriveType == DRIVE_REMOTE || dpDestPath.GetDriveType() == DRIVE_REMOTE)
		m_iBufferIndex=BI_LAN;
	else if (m_uiDriveType == DRIVE_CDROM || dpDestPath.GetDriveType() == DRIVE_CDROM)
		m_iBufferIndex=BI_CD;
	else if (m_uiDriveType == DRIVE_FIXED && dpDestPath.GetDriveType() == DRIVE_FIXED)
	{
		// two hdd's - is this the same physical disk ?
		if (m_iDriveNumber == dpDestPath.GetDriveNumber() || IsSamePhysicalDisk(m_iDriveNumber, dpDestPath.GetDriveNumber()))
			m_iBufferIndex=BI_ONEDISK;
		else
			m_iBufferIndex=BI_TWODISKS;
	}
	else
		m_iBufferIndex=BI_DEFAULT;
}

void CClipboardEntry::Serialize(CArchive& ar, bool bData)
{
	if (bData)
	{
		if (ar.IsStoring())
		{
			ar<<m_strPath;
			ar<<static_cast<unsigned char>(m_bMove);
			ar<<m_iDriveNumber;
			ar<<m_uiDriveType;
			ar<<m_iBufferIndex;
		}
		else
		{
			ar>>m_strPath;
			unsigned char ucData;
			ar>>ucData;
			m_bMove=ucData != 0;
			ar>>m_iDriveNumber;
			ar>>m_uiDriveType;
			ar>>m_iBufferIndex;
		}
	}
	else
		m_astrDstPaths.Serialize(ar);
}
