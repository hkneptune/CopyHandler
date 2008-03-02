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
#include "../libicpf/cfg.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EMPTY_STRING _t("")

CLangData::CLangData() :
	m_pszFilename(NULL),
	m_pszLngName(NULL),
	m_pszBaseFile(NULL),
	m_pszFontFace(NULL),
	m_pszHelpName(NULL),
	m_pszAuthor(NULL),
	m_pszVersion(NULL),
	m_uiSectionID(0)
{
}

CLangData::~CLangData()
{
	delete [] m_pszFilename;
	delete [] m_pszLngName;
	delete [] m_pszBaseFile;
	delete [] m_pszFontFace;
	delete [] m_pszHelpName;
	delete [] m_pszAuthor;
	delete [] m_pszVersion;

	for(strings_map::iterator it = m_mStrings.begin(); it != m_mStrings.end(); it++)
	{
		delete [] (*it).second;
	}
}

CLangData::CLangData(const CLangData& ld) :
	m_pszFilename(NULL),
	m_pszLngName(NULL),
	m_pszBaseFile(NULL),
	m_pszFontFace(NULL),
	m_pszHelpName(NULL),
	m_pszAuthor(NULL),
	m_pszVersion(NULL)
{
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
}

bool CLangData::ReadInfo(PCTSTR pszFile)
{
	try
	{
		icpf::config cfg(icpf::config::eIni);
		const uint_t uiLangName = cfg.register_string(_T("Info/Lang Name"), _t(""));
		const uint_t uiLangCode = cfg.register_signed_num(_T("Info/Lang Code"), 0, 0, 0xffff);
		const uint_t uiBaseLanguage = cfg.register_string(_T("Info/Base Language"), _T(""));
		const uint_t uiFontFace = cfg.register_string(_T("Info/Font Face"), _T(""));
		const uint_t uiCharset = cfg.register_signed_num(_T("Info/Charset"), 0, 0, 0xffff);
		const uint_t uiSize = cfg.register_signed_num(_T("Info/Size"), 0, 0, 0xffff);
		const uint_t uiRTL = cfg.register_bool(_T("Info/RTL reading order"), false);
		const uint_t uiHelpName = cfg.register_string(_T("Info/Help name"), _T(""));
		const uint_t uiAuthor = cfg.register_string(_T("Info/Author"), _T(""));
		const uint_t uiVersion = cfg.register_string(_T("Info/Version"), _T(""));
		cfg.read(pszFile);
		
		const tchar_t* psz = cfg.get_string(uiLangName);
		if(!psz || psz[0] == _t('\0'))
			return false;
		SetLangName(psz);

		ll_t ll = cfg.get_signed_num(uiLangCode);
		if(ll == 0)
			return false;
		SetLangCode((WORD)ll);

		psz = cfg.get_string(uiBaseLanguage);
		if(!psz || psz[0] == _t('\0'))
			return false;
		SetBaseFile(psz);

		psz = cfg.get_string(uiFontFace);
		if(!psz || psz[0] == _t('\0'))
			return false;
		SetFontFace(psz);

		ll = cfg.get_signed_num(uiCharset);
		if(ll == 0)
			return false;
		SetCharset((BYTE)ll);

		ll = cfg.get_signed_num(uiSize);
		if(ll == 0)
			return false;
		SetPointSize((WORD)ll);

		SetDirection(cfg.get_bool(uiRTL));

		psz = cfg.get_string(uiHelpName);
		if(!psz || psz[0] == _t('\0'))
			return false;
		SetHelpName(psz);

		psz = cfg.get_string(uiAuthor);
		if(!psz || psz[0] == _t('\0'))
			return false;
		SetAuthor(psz);

		psz = cfg.get_string(uiVersion);
		if(!psz || psz[0] == _t('\0'))
			return false;
		SetVersion(psz);

		SetFilename(pszFile);

		return true;
	}
	catch(...)
	{
		return false;
	}
}

void CLangData::EnumAttributesCallback(bool bGroup, const tchar_t* pszName, const tchar_t* pszValue, ptr_t pData)
{
	CLangData* pLangData = (CLangData*)pData;
	assert(pLangData);
	assert(pszName);

	if(bGroup && _tcsicmp(pszName, _t("Info")) != 0)
	{
		// new section - remember in member
		pLangData->m_uiSectionID = _ttoi(pszName);
	}
	else
	{
		uint_t uiVal = _ttoi(pszName);
		if(pLangData->m_bUpdating)
		{
			// check if the entry already exists
			strings_map::iterator it = pLangData->m_mStrings.find(pLangData->m_uiSectionID << 16 | uiVal);
			if(it != pLangData->m_mStrings.end())
				return;
		}
		size_t stLen = _tcslen(pszValue);
		tchar_t* pszStr = new tchar_t[stLen + 1];
		_tcscpy(pszStr, pszValue);
		pLangData->m_mStrings.insert(strings_map::value_type(pLangData->m_uiSectionID << 16 | uiVal, pszStr));
	}
}

bool CLangData::ReadTranslation(PCTSTR pszFile, bool bUpdate)
{
	try
	{
		// load data from file
		icpf::config cfg(icpf::config::eIni);
		const uint_t uiLangName = cfg.register_string(_T("Info/Lang Name"), _t(""));
		const uint_t uiLangCode = cfg.register_signed_num(_T("Info/Lang Code"), 0, 0, 0xffff);
		const uint_t uiBaseLanguage = cfg.register_string(_T("Info/Base Language"), _T(""));
		const uint_t uiFontFace = cfg.register_string(_T("Info/Font Face"), _T(""));
		const uint_t uiCharset = cfg.register_signed_num(_T("Info/Charset"), 0, 0, 0xffff);
		const uint_t uiSize = cfg.register_signed_num(_T("Info/Size"), 0, 0, 0xffff);
		const uint_t uiRTL = cfg.register_bool(_T("Info/RTL reading order"), false);
		const uint_t uiHelpName = cfg.register_string(_T("Info/Help name"), _T(""));
		const uint_t uiAuthor = cfg.register_string(_T("Info/Author"), _T(""));
		const uint_t uiVersion = cfg.register_string(_T("Info/Version"), _T(""));
		cfg.read(pszFile);

		TCHAR szData[512];
		if(!bUpdate)
		{
			const tchar_t* psz = cfg.get_string(uiLangName);
			if(!psz || psz[0] == _t('\0'))
				return false;
			SetLangName(psz);

			ll_t ll = cfg.get_signed_num(uiLangCode);
			if(ll == 0)
				return false;
			SetLangCode((WORD)ll);

			psz = cfg.get_string(uiBaseLanguage);
			if(!psz || psz[0] == _t('\0'))
				return false;
			SetBaseFile(psz);

			psz = cfg.get_string(uiFontFace);
			if(!psz || psz[0] == _t('\0'))
				return false;
			SetFontFace(psz);

			ll = cfg.get_signed_num(uiCharset);
			if(ll == 0)
				return false;
			SetCharset((BYTE)ll);

			ll = cfg.get_signed_num(uiSize);
			if(ll == 0)
				return false;
			SetPointSize((WORD)ll);

			SetDirection(cfg.get_bool(uiRTL));

			psz = cfg.get_string(uiHelpName);
			if(!psz || psz[0] == _t('\0'))
				return false;
			SetHelpName(psz);

			psz = cfg.get_string(uiAuthor);
			if(!psz || psz[0] == _t('\0'))
				return false;
			SetAuthor(psz);

			psz = cfg.get_string(uiVersion);
			if(!psz || psz[0] == _t('\0'))
				return false;
			SetVersion(psz);
		}
		
		m_bUpdating = bUpdate;
		m_uiSectionID = 0;
		if(!cfg.enum_properties(_t("*"), EnumAttributesCallback, this))
			return false;

		if(!bUpdate)
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
		return (*it).second;
	else
		return EMPTY_STRING;
}

void CLangData::SetFilename(PCTSTR psz)
{
	if (m_pszFilename)
		delete [] m_pszFilename;

	// copy
	m_pszFilename=new TCHAR[_tcslen(psz)+1];
	_tcscpy(m_pszFilename, psz);
}

PCTSTR CLangData::GetFilename(bool bFullPath) const
{
	if (bFullPath)
		return m_pszFilename;
	else
	{
		TCHAR *pszFnd=_tcsrchr(m_pszFilename, _T('\\'));
		if (pszFnd)
			return pszFnd+1;
		else
			return m_pszFilename;
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
