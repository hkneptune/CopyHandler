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

#include "libictranslate.h"
#include <map>
#include <vector>
#include <list>
#include "../libicpf/gen_types.h"

using namespace std;

BEGIN_ICTRANSLATE_NAMESPACE

/////////////////////////////////////////////////////////////////////////
// types of notifications
#define WM_RMNOTIFY (WM_USER + 2)

// RMNT_LANGCHANGE, LPARAM - HIWORD - old language, LOWORD - new language
#define RMNT_LANGCHANGE		0x0001

typedef void(*PFNNOTIFYCALLBACK)(uint_t, uint_t);

///////////////////////////////////////////////////////////
// language description structure
class CTranslationItem
{
public:
	CTranslationItem();
	CTranslationItem(const tchar_t* pszText, uint_t uiChecksum);
	~CTranslationItem();

	CTranslationItem& operator=(const CTranslationItem& rSrc);

	void Clear();
	void CalculateChecksum();

	const tchar_t* GetText() const { return m_pszText; }
	void SetText(const tchar_t* pszText);
	uint_t GetChecksum() const { return m_uiChecksum; }
	void SetChecksum(uint_t uiChecksum) { m_uiChecksum = uiChecksum; }

	void UnescapeString();

protected:
	tchar_t* m_pszText;
	size_t m_stTextLength;
	uint_t m_uiChecksum;
};

typedef void(*PFNENUMCALLBACK)(uint_t, const CTranslationItem*, ptr_t);
typedef std::map<uint_t, CTranslationItem> translation_map;

class LIBICTRANSLATE_API CLangData
{
public:
// construction/destruction
	CLangData();
	CLangData(const CLangData& ld);
	~CLangData();

	CLangData& operator=(const CLangData& rSrc);

	void Clear();
// operations
	bool ReadInfo(PCTSTR pszFile);
	bool ReadTranslation(PCTSTR pszFile, bool bReadBase = false);

	PCTSTR GetString(WORD wHiID, WORD wLoID);

	void EnumStrings(PFNENUMCALLBACK pfnCallback, ptr_t pData);

// attributes
	void SetFilename(PCTSTR psz);
	PCTSTR GetFilename(bool bFullPath) const;

	void SetLangName(PCTSTR psz);
	PCTSTR GetLangName() const { return m_pszLngName; };

	void SetBaseFile(PCTSTR psz);
	PCTSTR GetBaseFile() const { return m_pszBaseFile; };

	void SetLangCode(WORD wLang) { m_wLangCode=wLang; };
	WORD GetLangCode() const { return m_wLangCode; };

	void SetFontFace(PCTSTR psz);
	PCTSTR GetFontFace() const { return m_pszFontFace; };

	void SetCharset(BYTE byChar) { m_byCharset=byChar; };
	BYTE GetCharset() const { return m_byCharset; };

	void SetPointSize(WORD wSize) { m_wPointSize=wSize; };
	WORD GetPointSize() const { return m_wPointSize; };

	void SetDirection(bool brtl) { m_bRTL=brtl; };
	bool GetDirection() const { return m_bRTL; };

	void SetHelpName(PCTSTR psz);
	PCTSTR GetHelpName() const { return m_pszHelpName; };

	void SetAuthor(PCTSTR psz);
	PCTSTR GetAuthor() const { return m_pszAuthor; };

	void SetVersion(PCTSTR psz);
	PCTSTR GetVersion() const { return m_pszVersion; };

protected:
	void SetFnameData(PTSTR *ppszDst, PCTSTR pszSrc);
	static void EnumAttributesCallback(bool bGroup, const tchar_t* pszName, const tchar_t* pszValue, ptr_t pData);
	static void UnescapeString(tchar_t* pszData);

protected:
	TCHAR *m_pszFilename;		// file name of the language data (with path)
	TCHAR *m_pszLngName;		// name of the language (ie. Chinese (PRC))
	TCHAR *m_pszBaseFile;		// file with base language data (wo path)
	TCHAR *m_pszFontFace;		// face name of the font that will be used in dialogs
	WORD m_wLangCode;			// language code
	WORD m_wPointSize;		// font point size
	TCHAR *m_pszHelpName;		// help name (wo the directory) for this language
	TCHAR *m_pszAuthor;		// author name
	TCHAR *m_pszVersion;		// version of this file
	BYTE m_byCharset;			// charset for use with the font
	bool m_bRTL;				// does the language require right-to-left reading order ?

	// strings (for controls in dialog boxes the ID contains hi:dlg ID, lo:ctrl ID, for strings hi part is 0)
	translation_map m_mapTranslation;		// maps string ID to the offset in pszStrings

private:
	uint_t m_uiSectionID;			///< ID of the currently processed section
	bool m_bUpdating;				///< Are we updating the language with base language ?
};

/////////////////////////////////////////////////////////////////////////////////////

class LIBICTRANSLATE_API CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	void Init(HMODULE hrc);

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

	uint_t m_uiNotificationMsgID;	// window message to send to windows
	HMODULE m_hRes;
	PFNNOTIFYCALLBACK m_pfnCallback;
//	UINT m_uiMsg;
	CRITICAL_SECTION m_cs;
};

END_ICTRANSLATE_NAMESPACE

#endif