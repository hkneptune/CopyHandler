/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2003 Ixen Gerthannes (ixen@interia.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#include "stdafx.h"
#include "IniFile.h"
#include "ResourceManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLangData::CLangData(const CLangData& ld)
{
	szDefString=0;
	pszFilename=NULL;
	pszLngName=NULL;
	pszBaseFile=NULL;
	pszFontFace=NULL;
	pszHelpName=NULL;
	pszAuthor=NULL;
	pszVersion=NULL;
	pszStrings=NULL;
	tCount=0;

	SetFilename(ld.GetFilename(true));
	SetLangName(ld.GetLangName());
	SetLangCode(ld.GetLangCode());
	SetFontFace(ld.GetFontFace());
	SetCharset(ld.GetCharset());
	SetPointSize(ld.GetPointSize());
	SetDirection(ld.GetDirection());
	SetHelpName(ld.GetHelpName());
	SetAuthor(ld.GetAuthor());
	SetVersion(ld.GetVersion());

	SetStringData(ld.pszStrings, ld.tCount);
}

bool CLangData::ReadInfo(PCTSTR pszFile)
{
	try
	{
		CIniFile file;
		file.Open(pszFile, _T("Info"), true);
		
		TCHAR szData[512];
		SetLangName(file.GetString(_T("000"), _T("Info"), _T("Lang Name"), szData, _T("")));
		if (file.IsDefault())
			return false;
		SetLangCode((WORD)file.GetInt(_T("000"), _T("Info"), _T("Lang Code"), 0, szData));
		if (file.IsDefault())
			return false;
		SetBaseFile(file.GetString(_T("000"), _T("Info"), _T("Base Language"), szData, _T("")));
		if (file.IsDefault())
			return false;
		SetFontFace(file.GetString(_T("000"), _T("Info"), _T("Font Face"), szData, _T("")));
		if (file.IsDefault())
			return false;
		SetCharset((BYTE)file.GetInt(_T("000"), _T("Info"), _T("Charset"), 0, szData));
		if (file.IsDefault())
			return false;
		SetPointSize((WORD)file.GetInt(_T("000"), _T("Info"), _T("Size"), 0, szData));
		if (file.IsDefault())
			return false;
		SetDirection(file.GetBool(_T("000"), _T("Info"), _T("RTL reading order"), false, szData));
		if (file.IsDefault())
			return false;
		SetHelpName(file.GetString(_T("000"), _T("Info"), _T("Help name"), szData, _T("")));
		if (file.IsDefault())
			return false;
		SetAuthor(file.GetString(_T("000"), _T("Info"), _T("Author"), szData, _T("")));
		if (file.IsDefault())
			return false;
		SetVersion(file.GetString(_T("000"), _T("Info"), _T("Version"), szData, _T("")));
		if (file.IsDefault())
			return false;

		SetFilename(pszFile);

		return true;
	}
	catch(...)
	{
		return false;
	}
}

bool CLangData::ReadTranslation(PCTSTR pszFile, bool bUpdate)
{
	try
	{
		// load data from file
		CIniFile file;
		file.Open(pszFile, NULL, true);

		TCHAR szData[512];
		if (!bUpdate)
		{
			// std data
			SetLangName(file.GetString(_T("000"), _T("Info"), _T("Lang Name"), szData, _T("")));
			if (file.IsDefault())
				return false;
			SetLangCode((WORD)file.GetInt(_T("000"), _T("Info"), _T("Lang Code"), 0, szData));
			if (file.IsDefault())
				return false;
			SetBaseFile(file.GetString(_T("000"), _T("Info"), _T("Base Language"), szData, _T("")));
			if (file.IsDefault())
				return false;
			SetFontFace(file.GetString(_T("000"), _T("Info"), _T("Font Face"), szData, _T("")));
			if (file.IsDefault())
				return false;
			SetCharset((BYTE)file.GetInt(_T("000"), _T("Info"), _T("Charset"), 0, szData));
			if (file.IsDefault())
				return false;
			SetPointSize((WORD)file.GetInt(_T("000"), _T("Info"), _T("Size"), 0, szData));
			if (file.IsDefault())
				return false;
			SetDirection(file.GetBool(_T("000"), _T("Info"), _T("RTL reading order"), false, szData));
			if (file.IsDefault())
				return false;
			SetHelpName(file.GetString(_T("000"), _T("Info"), _T("Help name"), szData, _T("")));
			if (file.IsDefault())
				return false;
			SetAuthor(file.GetString(_T("000"), _T("Info"), _T("Author"), szData, _T("")));
			if (file.IsDefault())
				return false;
			SetVersion(file.GetString(_T("000"), _T("Info"), _T("Version"), szData, _T("")));
			if (file.IsDefault())
				return false;
		}
		
		// read strings section
		const _PROFILE* pcfg=file.GetProfile(_T("000"));
		if (pcfg)
		{
			// enum through the sections
			size_t tSkipped=(size_t)-1;
			WORD wHiID, wLoID;
			size_t tDataCount=0;
			
			// 1st phase - count data length
			vector<_SECTION*>::const_iterator sit;
			for (sit=pcfg->vSections.begin();sit != pcfg->vSections.end();sit++)
			{
				// skip "Info" section
				if (tSkipped == -1 && _tcscmp((*sit)->pszSectionName, _T("Info")) == 0)
				{
					tSkipped=sit-pcfg->vSections.begin();
					continue;
				}
				
				// now translate all the section of form [000] to the ID's, enum through the entries
				for (vector<_ENTRY*>::iterator it=(*sit)->vEntries.begin();it != (*sit)->vEntries.end();it++)
				{
					if (!bUpdate || m_mStrings.find((_ttoi((*sit)->pszSectionName) << 16) | (_ttoi((*it)->pszKey))) == m_mStrings.end())
						tDataCount+=_tcslen((*it)->pszValue)+1;
				}
			}
			
			// allocate the buffer for all data
			size_t tOffset=0;
			if (bUpdate)
			{
				if (tDataCount == 0)
					return true;

				// we need to reallocate the buffer
				TCHAR* pszData=new TCHAR[tCount+tDataCount];
				memcpy(pszData, pszStrings, tCount*sizeof(TCHAR));

				delete [] pszStrings;
				pszStrings=pszData;

				tOffset=tCount;
				tCount+=tDataCount;
			}
			else
			{
				// delete old settings
				delete [] pszStrings;
				m_mStrings.clear();

				tCount=tDataCount;
				pszStrings=new TCHAR[tDataCount];
			}
			
			// 2nd phase - copy all the data
			for (sit=pcfg->vSections.begin();sit != pcfg->vSections.end();sit++)
			{
				// skip "Info" section
				if (tSkipped == (size_t)(sit-pcfg->vSections.begin()))
					continue;
				
				// now translate all the section of form [000] to the ID's, enum through the entries
				wHiID=(WORD)_ttoi((*sit)->pszSectionName);
				for (vector<_ENTRY*>::iterator it=(*sit)->vEntries.begin();it != (*sit)->vEntries.end();it++)
				{
					// add to the map
					wLoID=(WORD)_ttoi((*it)->pszKey);
					if (!bUpdate || m_mStrings.find((wHiID << 16) | wLoID) == m_mStrings.end())
					{
						m_mStrings.insert(strings_map::value_type((((DWORD)wHiID) << 16 | wLoID), tOffset));
						
						// copy string
						_tcscpy(pszStrings+tOffset, (*it)->pszValue);
						tOffset+=_tcslen(pszStrings+tOffset)+1;
					}
				}
			}
		}
		
		// free unneded data
		file.Close();
		
		if (!bUpdate)
		{
			// remember the filename
			SetFilename(pszFile);

			// establish path to the base file
			if (_tcslen(GetBaseFile()) != 0)
			{
				const TCHAR* pszName=_tcsrchr(pszFile, _T('\\'));
				if (pszName)
				{
					_tcsncpy(szData, pszFile, pszName-pszFile+1);
					_tcscpy(szData+(pszName-pszFile+1), GetBaseFile());
					TRACE(_t("Base (update) path=%s\n"), szData);
					ReadTranslation(szData, true);
				}
			}
		}
		
		return true;
	}
	catch(...)
	{
		return false;
	}
}

PCTSTR CLangData::GetString(WORD wHiID, WORD wLoID)
{
	strings_map::iterator it=m_mStrings.find((wHiID << 16) | wLoID);
	if (it != m_mStrings.end())
		return pszStrings+(*it).second;
	else
		return &szDefString;
}

void CLangData::SetFilename(PCTSTR psz)
{
	if (pszFilename)
		delete [] pszFilename;

	// copy
	pszFilename=new TCHAR[_tcslen(psz)+1];
	_tcscpy(pszFilename, psz);
}

PCTSTR CLangData::GetFilename(bool bFullPath) const
{
	if (bFullPath)
		return pszFilename;
	else
	{
		TCHAR *pszFnd=_tcsrchr(pszFilename, _T('\\'));
		if (pszFnd)
			return pszFnd+1;
		else
			return pszFilename;
	}
}

void CLangData::SetFnameData(PTSTR *ppszDst, PCTSTR pszSrc)
{
	if (*ppszDst)
		delete [] (*ppszDst);
	const TCHAR* pszLast=NULL;
	if ( (pszLast=_tcsrchr(pszSrc, _T('\\'))) != NULL)
		pszLast++;
	else
		pszLast=pszSrc;

	// copy
	*ppszDst=new TCHAR[_tcslen(pszLast)+1];
	_tcscpy(*ppszDst, pszLast);
}

// requires the param with ending '\\'
void CResourceManager::Scan(LPCTSTR pszFolder, vector<CLangData>* pvData)
{
	TCHAR szPath[_MAX_PATH];
	_tcscpy(szPath, pszFolder);
	_tcscat(szPath, _T("*.lng"));
	
	WIN32_FIND_DATA wfd;
	HANDLE hFind=::FindFirstFile(szPath, &wfd);
	BOOL bFound=TRUE;
	CLangData ld;
	while (bFound && hFind != INVALID_HANDLE_VALUE)
	{
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			_tcscpy(szPath, pszFolder);
			_tcscat(szPath, wfd.cFileName);
			if (ld.ReadInfo(szPath))
				pvData->push_back(ld);
		}

		bFound=::FindNextFile(hFind, &wfd);
	}

	if (hFind != INVALID_HANDLE_VALUE)
		::FindClose(hFind);
}

bool CResourceManager::SetLanguage(PCTSTR pszPath)
{
	EnterCriticalSection(&m_cs);
	WORD wOldLang=m_ld.GetLangCode();
	bool bRet=m_ld.ReadTranslation(pszPath);
	WORD wNewLang=m_ld.GetLangCode();
	LeaveCriticalSection(&m_cs);
	if (!bRet)
		return false;
	
	// update registered dialog boxes
	list<CWnd*>::iterator it=m_lhDialogs.begin();
	while (it != m_lhDialogs.end())
	{
		if (::IsWindow((*it)->m_hWnd))
			(*it)->PostMessage(WM_RMNOTIFY, RMNT_LANGCHANGE, (LPARAM)(wOldLang << 16 | wNewLang));
		it++;
	}
				
	// send the notification stuff to the others
	if (m_pfnCallback)
		(*m_pfnCallback)(ROT_EVERYWHERE, WM_RMNOTIFY, RMNT_LANGCHANGE, (LPARAM)(wOldLang << 16 | wNewLang));

	return bRet;
}

HGLOBAL CResourceManager::LoadResource(LPCTSTR pszType, LPCTSTR pszName)
{
	EnterCriticalSection(&m_cs);

	// find resource
	HGLOBAL hRet=NULL;
	HRSRC hr=FindResource(m_hRes, pszName, pszType);
	if (hr)
		hRet=::LoadResource(m_hRes, hr);

	LeaveCriticalSection(&m_cs);
	return hRet;
}

HACCEL CResourceManager::LoadAccelerators(LPCTSTR pszName)
{
	return ::LoadAccelerators(m_hRes, pszName);
}

HBITMAP CResourceManager::LoadBitmap(LPCTSTR pszName)
{
	return ::LoadBitmap(m_hRes, pszName);
}

HCURSOR CResourceManager::LoadCursor(LPCTSTR pszName)
{
	return ::LoadCursor(m_hRes, pszName);
}

HICON CResourceManager::LoadIcon(LPCTSTR pszName)
{
	return ::LoadIcon(m_hRes, pszName);
}

void CResourceManager::UpdateMenu(HMENU hMenu, WORD wMenuID)
{
	// change the strings inside the menu to the one from txt res file
	int iCount=::GetMenuItemCount(hMenu);
	MENUITEMINFO mif;
	WORD wLoID;
	TCHAR szItem[1024];
	for (int i=0;i<iCount;i++)
	{
		memset(&mif, 0, sizeof(MENUITEMINFO));
		mif.cbSize=sizeof(MENUITEMINFO);
		mif.dwTypeData=szItem;
		mif.cch=1023;
		mif.fMask=MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID | MIIM_DATA | MIIM_STRING;
		if (::GetMenuItemInfo(hMenu, i, TRUE, &mif))
		{
			// has sub items ?
			if (mif.hSubMenu)
				UpdateMenu(mif.hSubMenu, wMenuID);

			// the menu item contains a string to update
			if (mif.fType == MFT_STRING)
			{
				if (mif.hSubMenu)
				{
					if (mif.dwItemData != 0)
						wLoID=(WORD)mif.dwItemData;		// already updated data
					else
					{
						// fresh menu - try to update info from caption
						wLoID=(WORD)_ttoi((PTSTR)mif.dwTypeData);

						// remember this info in item's private storage
						MENUITEMINFO ii;
						ii.cbSize=sizeof(MENUITEMINFO);
						ii.fMask=MIIM_DATA;
						::SetMenuItemInfo(hMenu, i, TRUE, &ii);
					}
				}
				else
					wLoID=(WORD)::GetMenuItemID(hMenu, i);

				mif.fMask=MIIM_STRING | MIIM_FTYPE;
				if (m_ld.GetDirection())
					mif.fType |= MFT_RIGHTORDER;
				else
					mif.fType &= ~MFT_RIGHTORDER;
				mif.dwTypeData=(LPTSTR)(m_ld.GetString(wMenuID, wLoID));
				::SetMenuItemInfo(hMenu, i, TRUE, &mif);
			}
		}
	}
}

HMENU CResourceManager::LoadMenu(LPCTSTR pszName)
{
	EnterCriticalSection(&m_cs);
	HMENU hMenu=::LoadMenu(m_hRes, pszName);

	if (hMenu && IS_INTRESOURCE(pszName))
		UpdateMenu(hMenu, (WORD)pszName);

	LeaveCriticalSection(&m_cs);
	return hMenu;
}

LPDLGTEMPLATE CResourceManager::LoadDialog(LPCTSTR pszName)
{
	HGLOBAL hgl=LoadResource(RT_DIALOG, pszName);
	DLGTEMPLATE *dlgt=(DLGTEMPLATE*)::LockResource(hgl);
	return dlgt;
}

PCTSTR CResourceManager::LoadString(UINT uiID)
{
	EnterCriticalSection(&m_cs);
	PCTSTR pszData=m_ld.GetString(0, (WORD)uiID);
	LeaveCriticalSection(&m_cs);
	
	return pszData;
}

PCTSTR CResourceManager::LoadString(WORD wGroup, WORD wID)
{
	EnterCriticalSection(&m_cs);
	PCTSTR pszData=m_ld.GetString(wGroup, wID);
	LeaveCriticalSection(&m_cs);
	return pszData;
}

PTSTR CResourceManager::LoadStringCopy(UINT uiID, PTSTR pszStr, UINT uiMax)
{
	EnterCriticalSection(&m_cs);
	_tcsncpy(pszStr, m_ld.GetString(0, (WORD)uiID), uiMax-1);
	pszStr[uiMax-1]=_T('\0');
	LeaveCriticalSection(&m_cs);
	return pszStr;
}

HANDLE CResourceManager::LoadImage(LPCTSTR lpszName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad)
{
	EnterCriticalSection(&m_cs);

	HANDLE hImg=::LoadImage(m_hRes, lpszName, uType, cxDesired, cyDesired, fuLoad);

	LeaveCriticalSection(&m_cs);
	return hImg;
}
