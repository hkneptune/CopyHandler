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
#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <map>
#include <vector>
#include <list>
#include "af_defs.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////
// types of notifications
// RMNT_LANGCHANGE, LPARAM - HIWORD - old language, LOWORD - new language
#define RMNT_LANGCHANGE		0x0001

///////////////////////////////////////////////////////////
// language description structure
typedef map<DWORD, size_t> strings_map;

class CLangData
{
public:
// construction/destruction
	CLangData() { szDefString=0; pszFilename=NULL; pszLngName=NULL; pszBaseFile=NULL; pszFontFace=NULL; pszHelpName=NULL; pszAuthor=NULL; pszVersion=NULL; pszStrings=NULL; tCount=0; }
	~CLangData() { delete [] pszFilename; delete [] pszLngName; delete [] pszBaseFile; delete [] pszFontFace; delete [] pszHelpName; delete [] pszAuthor; delete [] pszVersion; delete [] pszStrings; };
	CLangData(const CLangData& ld);

protected:
	void SetFnameData(PTSTR *ppszDst, PCTSTR pszSrc);

public:
// operations
	bool ReadInfo(PCTSTR pszFile);
	bool ReadTranslation(PCTSTR pszFile, bool bUpdate=false);

	PCTSTR GetString(WORD wHiID, WORD wLoID);

// attributes
	void SetFilename(PCTSTR psz);
	PCTSTR GetFilename(bool bFullPath) const;

	void SetLangName(PCTSTR psz) { if (pszLngName) delete [] pszLngName; pszLngName=new TCHAR[_tcslen(psz)+1]; _tcscpy(pszLngName, psz); };
	PCTSTR GetLangName() const { return pszLngName; };

	void SetBaseFile(PCTSTR psz) { SetFnameData(&pszBaseFile, psz); };
	PCTSTR GetBaseFile() const { return pszBaseFile; };

	void SetLangCode(WORD wLang) { wLangCode=wLang; };
	WORD GetLangCode() const { return wLangCode; };

	void SetFontFace(PCTSTR psz) { if (pszFontFace) delete [] pszFontFace; pszFontFace=new TCHAR[_tcslen(psz)+1]; _tcscpy(pszFontFace, psz); };
	PCTSTR GetFontFace() const { return pszFontFace; };

	void SetCharset(BYTE byChar) { byCharset=byChar; };
	BYTE GetCharset() const { return byCharset; };

	void SetPointSize(WORD wSize) { wPointSize=wSize; };
	WORD GetPointSize() const { return wPointSize; };

	void SetDirection(bool brtl) { bRTL=brtl; };
	bool GetDirection() const { return bRTL; };

	void SetHelpName(PCTSTR psz) { SetFnameData(&pszHelpName, psz); };
	PCTSTR GetHelpName() const { return pszHelpName; };

	void SetAuthor(PCTSTR psz) { if (pszAuthor) delete [] pszAuthor; pszAuthor=new TCHAR[_tcslen(psz)+1]; _tcscpy(pszAuthor, psz); };
	PCTSTR GetAuthor() const { return pszAuthor; };

	void SetVersion(PCTSTR psz) { if (pszVersion) delete [] pszVersion; pszVersion=new TCHAR[_tcslen(psz)+1]; _tcscpy(pszVersion, psz); };
	PCTSTR GetVersion() const { return pszVersion; };

	void SetStringData(PCTSTR psz, size_t tCnt) { tCount=tCnt; if (pszStrings) delete [] pszStrings; if (tCount > 0) { pszStrings=new TCHAR[tCnt]; memcpy(pszStrings, psz, tCnt*sizeof(TCHAR)); } };

public:
	TCHAR *pszFilename;		// file name of the language data (with path)
	TCHAR *pszLngName;		// name of the language (ie. Chinese (PRC))
	TCHAR *pszBaseFile;		// file with base language data (wo path)
	TCHAR *pszFontFace;		// face name of the font that will be used in dialogs
	WORD wLangCode;			// language code
	WORD wPointSize;		// font point size
	TCHAR *pszHelpName;		// help name (wo the directory) for this language
	TCHAR *pszAuthor;		// author name
	TCHAR *pszVersion;		// version of this file
	BYTE byCharset;			// charset for use with the font
	bool bRTL;				// does the language require right-to-left reading order ?

	// strings (for controls in dialog boxes the ID contains hi:dlg ID, lo:ctrl ID, for strings hi part is 0)
	strings_map m_mStrings;	// maps string ID to the offset in pszStrings
	TCHAR *pszStrings;				// contains all the strings - NULL separated
	size_t tCount;					// length of the string table
	TCHAR szDefString;				// default empty string
};

/////////////////////////////////////////////////////////////////////////////////////

class CResourceManager
{
public:
	CResourceManager() { m_pfnCallback=NULL; m_hRes=NULL; InitializeCriticalSection(&m_cs); };
	~CResourceManager() { DeleteCriticalSection(&m_cs); };

	void Init(HMODULE hrc) { m_hRes=hrc; };

	void SetCallback(PFNNOTIFYCALLBACK pfn) { m_pfnCallback=pfn; };

	void Scan(LPCTSTR pszFolder, vector<CLangData>* pvData);
	bool SetLanguage(PCTSTR pszPath);

	// loading functions
	HGLOBAL LoadResource(LPCTSTR pszType, LPCTSTR pszName);
	HACCEL LoadAccelerators(LPCTSTR pszName);
	HBITMAP LoadBitmap(LPCTSTR pszName);
	HCURSOR LoadCursor(LPCTSTR pszName);
	HICON LoadIcon(LPCTSTR pszName);
	HANDLE LoadImage(LPCTSTR lpszName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad);
	HMENU LoadMenu(LPCTSTR pszName);
	LPDLGTEMPLATE LoadDialog(LPCTSTR pszName);

	PCTSTR LoadString(UINT uiID);
	PCTSTR LoadString(WORD wGroup, WORD wID);
	PTSTR LoadStringCopy(UINT uiID, PTSTR pszStr, UINT uiMax);

	// res updating functions
	void UpdateMenu(HMENU hMenu, WORD wMenuID);

public:
	CLangData m_ld;				// current language data
	list<CWnd*> m_lhDialogs;	// currently displayed dialog boxes (even hidden)

	HMODULE m_hRes;
	PFNNOTIFYCALLBACK m_pfnCallback;
//	UINT m_uiMsg;
	CRITICAL_SECTION m_cs;
};

#endif