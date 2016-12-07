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
#include "stdafx.h"
#include "ResourceManager.h"
#include <assert.h>
#include <sstream>
#include "cfg.h"
#include <stdexcept>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TRANSLATION_FORMAT_VERSION _T("2")
#define EMPTY_STRING _T("")

namespace ictranslate
{
	CResourceManager CResourceManager::S_ResourceManager;

	CFormat::CFormat(const wchar_t* pszFormat) :
		m_strText(pszFormat)
	{
	}

	CFormat::CFormat()
	{
	}

	CFormat::~CFormat()
	{
	}

	void CFormat::SetFormat(const wchar_t* pszFormat)
	{
		m_strText = pszFormat;
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, PCTSTR pszText)
	{
		assert(pszName);
		if(!pszName)
			return *this;

		size_t stLen = _tcslen(pszName);
		std::wstring::size_type stPos = 0;
		while((stPos = m_strText.find(pszName)) != std::wstring::npos)
		{
			m_strText.replace(stPos, stLen, pszText);
		}

		return *this;
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, unsigned long long ullData)
	{
		wchar_t szBuffer[ 64 ];
		_sntprintf(szBuffer, 63, L"%I64u", ullData);
		szBuffer[ 63 ] = _T('\0');

		return SetParam(pszName, szBuffer);
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, long long llData)
	{
		wchar_t szBuffer[ 64 ];
		_sntprintf(szBuffer, 63, L"%I64d", llData);
		szBuffer[ 63 ] = _T('\0');

		return SetParam(pszName, szBuffer);
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, unsigned long ulData)
	{
		wchar_t szBuffer[ 64 ];
		_sntprintf(szBuffer, 63, L"%lu", ulData);
		szBuffer[ 63 ] = _T('\0');

		return SetParam(pszName, szBuffer);
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, unsigned int uiData)
	{
		wchar_t szBuffer[ 64 ];
		_sntprintf(szBuffer, 63, L"%u", uiData);
		szBuffer[ 63 ] = _T('\0');

		return SetParam(pszName, szBuffer);
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, int iData)
	{
		wchar_t szBuffer[ 64 ];
		_sntprintf(szBuffer, 63, L"%d", iData);
		szBuffer[ 63 ] = _T('\0');

		return SetParam(pszName, szBuffer);
	}

	CFormat& CFormat::SetParam(PCTSTR pszName, bool bData)
	{
		wchar_t szBuffer[ 64 ];
		_sntprintf(szBuffer, 63, L"%hu", (unsigned short)bData);
		szBuffer[ 63 ] = _T('\0');

		return SetParam(pszName, szBuffer);
	}

	CTranslationItem::CTranslationItem()
	{
	}

	CTranslationItem::CTranslationItem(const wchar_t* pszText, unsigned int uiChecksum)
	{
		if(pszText)
		{
			m_stTextLength = _tcslen(pszText);
			if(m_stTextLength > 0)
			{
				m_pszText = new wchar_t[ m_stTextLength + 1 ];
				_tcscpy(m_pszText, pszText);

				UnescapeString();
			}

			m_uiChecksum = uiChecksum;
		}
	}

	ictranslate::CTranslationItem::CTranslationItem(const CTranslationItem& rSrc)
	{
		if(rSrc.m_pszText)
		{
			m_stTextLength = _tcslen(rSrc.m_pszText);
			if(m_stTextLength > 0)
			{
				m_pszText = new wchar_t[ m_stTextLength + 1 ];
				_tcscpy(m_pszText, rSrc.m_pszText);

				UnescapeString();
			}
		}
	}

	CTranslationItem::~CTranslationItem()
	{
		Clear();
	}

	CTranslationItem& CTranslationItem::operator=(const CTranslationItem& rSrc)
	{
		if(this != &rSrc)
		{
			Clear();
			if(rSrc.m_pszText)
			{
				m_stTextLength = rSrc.m_stTextLength;
				if(m_stTextLength > 0)
				{
					m_pszText = new wchar_t[ rSrc.m_stTextLength + 1 ];
					_tcscpy(m_pszText, rSrc.m_pszText);
				}
			}
			m_uiChecksum = rSrc.m_uiChecksum;
		}

		return *this;
	}

	void CTranslationItem::Clear()
	{
		delete[] m_pszText;
		m_pszText = nullptr;
		m_stTextLength = 0;
		m_uiChecksum = 0;
	}

	const wchar_t* CTranslationItem::GetText() const
	{
		return m_pszText ? m_pszText : _T("");
	}

	void CTranslationItem::SetText(const wchar_t* pszText, bool bUnescapeString)
	{
		delete[] m_pszText;
		if(pszText)
		{
			m_stTextLength = _tcslen(pszText);
			if(m_stTextLength > 0)
			{
				m_pszText = new wchar_t[ m_stTextLength + 1 ];
				_tcscpy(m_pszText, pszText);
				if(bUnescapeString)
					UnescapeString();
				return;
			}
		}

		m_pszText = nullptr;
		m_stTextLength = 0;
	}

	void CTranslationItem::UnescapeString()
	{
		if(!m_pszText)
			return;

		const wchar_t* pszIn = m_pszText;
		wchar_t* pszOut = m_pszText;
		while(*pszIn != 0)
		{
			if(*pszIn == _T('\\'))
			{
				pszIn++;
				switch(*pszIn++)
				{
				case _T('t'):
					*pszOut++ = _T('\t');
					break;
				case _T('r'):
					*pszOut++ = _T('\r');
					break;
				case _T('n'):
					*pszOut++ = _T('\n');
					break;
				default:
					*pszOut++ = _T('\\');
				}
			}
			else
				*pszOut++ = *pszIn++;
		}
		*pszOut = _T('\0');
	}

	CTranslationItem::ECompareResult CTranslationItem::Compare(const CTranslationItem& rReferenceItem)
	{
		if(!m_pszText || !rReferenceItem.m_pszText)
			return eResult_Invalid;

		if(m_uiChecksum != rReferenceItem.m_uiChecksum)
			return eResult_Invalid;

		// space check
		if(rReferenceItem.m_pszText[ 0 ] == _T(' ') && m_pszText[ 0 ] != _T(' '))
			return eResult_ContentWarning;

		size_t stReferenceLen = _tcslen(rReferenceItem.m_pszText);
		size_t stOwnLen = _tcslen(m_pszText);
		if(stReferenceLen > 0 && stOwnLen > 0 && rReferenceItem.m_pszText[ stReferenceLen - 1 ] == _T(' ') && m_pszText[ stOwnLen - 1 ] != _T(' '))
			return eResult_ContentWarning;

		// formatting strings check
		std::set<std::wstring> setRefFmt;
		if(!rReferenceItem.GetFormatStrings(setRefFmt))
			return eResult_ContentWarning;

		std::set<std::wstring> setThisFmt;
		if(!GetFormatStrings(setThisFmt))
			return eResult_ContentWarning;

		if(setRefFmt != setThisFmt)
			return eResult_ContentWarning;

		return eResult_Valid;
	}

	bool CTranslationItem::GetFormatStrings(std::set<std::wstring>& setFmtStrings) const
	{
		setFmtStrings.clear();

		const wchar_t* pszData = m_pszText;
		const size_t stMaxFmt = 256;
		wchar_t szFmt[ stMaxFmt ];
		while((pszData = _tcschr(pszData, _T('%'))) != nullptr)
		{
			pszData++;		// it works assuming the string is null-terminated

			// search the end of fmt string
			const wchar_t* pszNext = pszData;
			while(*pszNext && isalpha(*pszNext))
				pszNext++;

			// if we have bigger buffer needs than is available
			if(pszNext - pszData >= stMaxFmt)
				return false;

			// copy data
			_tcsncpy(szFmt, pszData, pszNext - pszData);
			szFmt[ pszNext - pszData ] = _T('\0');

			setFmtStrings.insert(std::wstring(szFmt));
		}

		return true;
	}

	CLangData::CLangData() :
		m_pszFilename(nullptr),
		m_pszLngName(nullptr),
		m_pszFontFace(nullptr),
		m_wPointSize(0),
		m_pszHelpName(nullptr),
		m_pszAuthor(nullptr),
		m_bRTL(false),
		m_uiSectionID(0),
		m_bUpdating(false),
		m_bModified(false)
	{
	}

	CLangData::~CLangData()
	{
		delete[] m_pszFilename;
		delete[] m_pszLngName;
		delete[] m_pszFontFace;
		delete[] m_pszHelpName;
		delete[] m_pszAuthor;
	}

	void CLangData::Clear()
	{
		delete[] m_pszFilename;
		m_pszFilename = nullptr;
		delete[] m_pszLngName;
		m_pszLngName = nullptr;
		delete[] m_pszFontFace;
		m_pszFontFace = nullptr;
		delete[] m_pszHelpName;
		m_pszHelpName = nullptr;
		delete[] m_pszAuthor;
		m_pszAuthor = nullptr;
		m_bModified = false;
		m_bRTL = false;
		m_bUpdating = false;
		m_uiSectionID = 0;
		m_wPointSize = 0;

		m_mapTranslation.clear();
	}

	CLangData::CLangData(const CLangData& ld) :
		m_pszFilename(nullptr),
		m_pszLngName(nullptr),
		m_pszFontFace(nullptr),
		m_wPointSize(ld.m_wPointSize),
		m_pszHelpName(nullptr),
		m_pszAuthor(nullptr),
		m_bRTL(ld.m_bRTL),
		m_uiSectionID(ld.m_uiSectionID),
		m_bUpdating(ld.m_bUpdating),
		m_bModified(false)
	{
		SetFilename(ld.GetFilename(true));
		SetLangName(ld.GetLangName());
		SetFontFace(ld.GetFontFace());
		SetPointSize(ld.GetPointSize());
		SetDirection(ld.GetDirection());
		SetHelpName(ld.GetHelpName());
		SetAuthor(ld.GetAuthor());

		m_mapTranslation.insert(ld.m_mapTranslation.begin(), ld.m_mapTranslation.end());
	}

	CLangData& CLangData::operator=(const CLangData& rSrc)
	{
		if(this != &rSrc)
		{
			SetFilename(rSrc.GetFilename(true));
			SetLangName(rSrc.GetLangName());
			SetFontFace(rSrc.GetFontFace());
			SetPointSize(rSrc.GetPointSize());
			SetDirection(rSrc.GetDirection());
			SetHelpName(rSrc.GetHelpName());
			SetAuthor(rSrc.GetAuthor());
			m_bRTL = rSrc.m_bRTL;
			m_bUpdating = rSrc.m_bUpdating;
			m_uiSectionID = rSrc.m_uiSectionID;
			m_wPointSize = rSrc.m_wPointSize;
			m_bModified = false;

			m_mapTranslation.insert(rSrc.m_mapTranslation.begin(), rSrc.m_mapTranslation.end());
		}

		return *this;
	}

	bool CLangData::ReadInfo(PCTSTR pszFile)
	{
		try
		{
			Clear();

			config cfg(config::eIni);
			const unsigned int uiLangName = cfg.register_string(_T("Info/Lang Name"), _T(""));
			const unsigned int uiFontFace = cfg.register_string(_T("Info/Font Face"), _T(""));
			const unsigned int uiSize = cfg.register_signed_num(_T("Info/Size"), 0, 0, 0xffff);
			const unsigned int uiRTL = cfg.register_bool(_T("Info/RTL reading order"), false);
			const unsigned int uiHelpName = cfg.register_string(_T("Info/Help name"), _T(""));
			const unsigned int uiAuthor = cfg.register_string(_T("Info/Author"), _T(""));
			const unsigned int uiVersion = cfg.register_string(_T("Info/Format version"), _T("1"));

			cfg.read(pszFile);

			// we don't support old language versions
			const wchar_t* pszVersion = cfg.get_string(uiVersion);
			if(_tcscmp(pszVersion, TRANSLATION_FORMAT_VERSION) != 0)
				return false;

			const wchar_t* psz = cfg.get_string(uiLangName);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetLangName(psz);

			psz = cfg.get_string(uiFontFace);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetFontFace(psz);

			long long ll = cfg.get_signed_num(uiSize);
			if(ll == 0)
				return false;
			SetPointSize((WORD)ll);

			SetDirection(cfg.get_bool(uiRTL));

			psz = cfg.get_string(uiHelpName);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetHelpName(psz);

			psz = cfg.get_string(uiAuthor);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetAuthor(psz);

			SetFilename(pszFile);

			m_bModified = false;

			return true;
		}
		catch(...)
		{
			return false;
		}
	}

	void CLangData::EnumAttributesCallback(bool bGroup, const wchar_t* pszName, const wchar_t* pszValue, void* pData)
	{
		CLangData* pLangData = (CLangData*)pData;
		assert(pLangData);
		assert(pszName);
		if(!pLangData || !pszName)
			return;

		if(bGroup && _tcsicmp(pszName, _T("Info")) == 0)
			return;
		if(bGroup)
		{
			// new section - remember in member
			pLangData->m_uiSectionID = _ttoi(pszName);
		}
		else
		{
			unsigned int uiID = 0;
			unsigned int uiChecksum = 0;

			// parse the pszName to get both the string id and checksum
			const wchar_t* pszChecksum = _tcschr(pszName, _T('['));
			if(pszChecksum == nullptr)
			{
				TRACE(_T("Warning! Old-style translation string %s.\n"), pszName);

				int iCount = _stscanf(pszName, L"%u", &uiID);
				if(iCount != 1)
				{
					TRACE(_T("Warning! Problem retrieving id from string '%s'\n"), pszName);
					return;
				}
			}
			else
			{
				int iCount = _stscanf(pszName, L"%u[0x%x]", &uiID, &uiChecksum);
				if(iCount != 2)
				{
					TRACE(_T("Warning! Problem retrieving id/checksum from string '%s'\n"), pszName);
					return;
				}
			}

			unsigned int uiKey = pLangData->m_uiSectionID << 16 | uiID;
			translation_map::iterator itTranslation = pLangData->m_mapTranslation.end();
			if(pLangData->m_bUpdating)
			{
				// check if the checksum exists and matches
				itTranslation = pLangData->m_mapTranslation.find(uiKey);
				if(itTranslation == pLangData->m_mapTranslation.end())
				{
					TRACE(_T("Warning! Superfluous entry %lu in processed language file\n"), uiKey);
					return;		// entry not found - probably superfluous entry in the language file
				}

				if((*itTranslation).second.GetChecksum() != uiChecksum)
				{
					TRACE(_T("Warning! Invalid checksum for string ID %lu in processed language file\n"), uiKey);
					return;		// entry has invalid checksum (older version of translation)
				}
			}
			else
			{
				std::pair<translation_map::iterator, bool> pairTranslation = pLangData->m_mapTranslation.insert(translation_map::value_type(uiKey, CTranslationItem()));
				itTranslation = pairTranslation.first;
			}

			assert(itTranslation != pLangData->m_mapTranslation.end());
			if(itTranslation != pLangData->m_mapTranslation.end())
			{
				(*itTranslation).second.SetText(pszValue, true);
				if(!pLangData->m_bUpdating)
					(*itTranslation).second.SetChecksum(uiChecksum);
			}
		}
	}

	void CLangData::UnescapeString(wchar_t* pszData)
	{
		wchar_t* pszOut = pszData;
		while(*pszData != 0)
		{
			if(*pszData == _T('\\'))
			{
				pszData++;
				switch(*pszData++)
				{
				case _T('t'):
					*pszOut++ = _T('\t');
					break;
				case _T('r'):
					*pszOut++ = _T('\r');
					break;
				case _T('n'):
					*pszOut++ = _T('\n');
					break;
				default:
					*pszOut++ = _T('\\');
				}
			}
			else
				*pszOut++ = *pszData++;
		}
		*pszOut = _T('\0');

	}

	bool CLangData::ReadTranslation(PCTSTR pszFile, bool bUpdateTranslation, bool bIgnoreVersion)
	{
		try
		{
			if(!bUpdateTranslation)
				Clear();

			// load data from file
			config cfg(config::eIni);
			const unsigned int uiLangName = cfg.register_string(_T("Info/Lang Name"), _T(""));
			const unsigned int uiFontFace = cfg.register_string(_T("Info/Font Face"), _T(""));
			const unsigned int uiSize = cfg.register_signed_num(_T("Info/Size"), 0, 0, 0xffff);
			const unsigned int uiRTL = cfg.register_bool(_T("Info/RTL reading order"), false);
			const unsigned int uiHelpName = cfg.register_string(_T("Info/Help name"), _T(""));
			const unsigned int uiAuthor = cfg.register_string(_T("Info/Author"), _T(""));
			const unsigned int uiVersion = cfg.register_string(_T("Info/Format version"), _T("1"));

			cfg.read(pszFile);

			// we don't support old language versions unless requested specifically
			if(!bIgnoreVersion)
			{
				const wchar_t* pszVersion = cfg.get_string(uiVersion);
				if(_tcscmp(pszVersion, TRANSLATION_FORMAT_VERSION) != 0)
					return false;
			}

			const wchar_t* psz = cfg.get_string(uiLangName);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetLangName(psz);

			psz = cfg.get_string(uiFontFace);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetFontFace(psz);

			long long ll = cfg.get_signed_num(uiSize);
			if(ll == 0)
				return false;
			SetPointSize((WORD)ll);

			SetDirection(cfg.get_bool(uiRTL));

			psz = cfg.get_string(uiHelpName);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetHelpName(psz);

			psz = cfg.get_string(uiAuthor);
			if(!psz || psz[ 0 ] == _T('\0'))
				return false;
			SetAuthor(psz);

			m_bUpdating = bUpdateTranslation;
			m_uiSectionID = 0;
			if(!cfg.enum_properties(_T("*"), EnumAttributesCallback, this))
			{
				m_bUpdating = false;
				return false;
			}
			m_bUpdating = false;

			SetFilename(pszFile);

			m_bModified = false;

			return true;
		}
		catch(...)
		{
			return false;
		}
	}

	void CLangData::WriteTranslation(PCTSTR pszPath)
	{
		if(!IsValidDescription())
			throw std::runtime_error("Invalid translation information (author, name or point size)");

		// real writing
		const int iBufferSize = 256;
		wchar_t szTemp[ iBufferSize ];

		// load data from file
		config cfg(config::eIni);
		cfg.set_string(_T("Info/Lang Name"), m_pszLngName);
		cfg.set_string(_T("Info/Font Face"), m_pszFontFace);
		cfg.set_string(_T("Info/Size"), _itot(m_wPointSize, szTemp, 10));
		cfg.set_string(_T("Info/RTL reading order"), m_bRTL ? _T("1") : _T("0"));
		cfg.set_string(_T("Info/Help name"), m_pszHelpName);
		cfg.set_string(_T("Info/Author"), m_pszAuthor);
		cfg.set_string(_T("Info/Format version"), TRANSLATION_FORMAT_VERSION);

		std::wstring strText;
		for(translation_map::iterator it = m_mapTranslation.begin(); it != m_mapTranslation.end(); ++it)
		{
			unsigned int uiKey = (*it).first;
			_sntprintf(szTemp, iBufferSize - 1, L"%u/%u[0x%x]", (uiKey >> 16), uiKey & 0x0000ffff, (*it).second.GetChecksum());

			strText = (*it).second.GetText();
			std::wstring::size_type stPos;
			while((stPos = strText.find_first_of(_T("\r\n\t"))) != std::wstring::npos)
			{
				switch(strText[ stPos ])
				{
				case _T('\r'):
					strText.replace(stPos, 1, _T("\\r"));
					break;
				case _T('\n'):
					strText.replace(stPos, 1, _T("\\n"));
					break;
				case _T('\t'):
					strText.replace(stPos, 1, _T("\\t"));
					break;
				}
			}

			cfg.set_string(szTemp, strText.c_str());
		}

		if(pszPath == nullptr)
			pszPath = m_pszFilename;
		else
			SetFilename(pszPath);
		cfg.write(pszPath);

		m_bModified = false;
	}

	PCTSTR CLangData::GetString(WORD wHiID, WORD wLoID)
	{
		translation_map::const_iterator it = m_mapTranslation.find((wHiID << 16) | wLoID);
		if(it != m_mapTranslation.end())
			return (*it).second.GetText();
		else
			return EMPTY_STRING;
	}

	void CLangData::EnumStrings(PFNENUMCALLBACK pfnCallback, void* pData)
	{
		for(translation_map::const_iterator iterTranslation = m_mapTranslation.begin(); iterTranslation != m_mapTranslation.end(); ++iterTranslation)
		{
			(*pfnCallback)((*iterTranslation).first, &(*iterTranslation).second, pData);
		}
	}

	CTranslationItem* CLangData::GetTranslationItem(unsigned int uiTranslationKey, bool bCreate)
	{
		translation_map::iterator iterTranslation = m_mapTranslation.find(uiTranslationKey);
		if(iterTranslation != m_mapTranslation.end())
			return &(*iterTranslation).second;
		else
		{
			if(bCreate)
			{
				std::pair<translation_map::iterator, bool> pairTranslation = m_mapTranslation.insert(std::make_pair(uiTranslationKey, CTranslationItem()));
				if(pairTranslation.second)
				{
					m_bModified = true;
					return &(*pairTranslation.first).second;
				}
			}
		}

		return nullptr;
	}

	bool CLangData::Exists(unsigned int uiTranslationKey) const
	{
		return m_mapTranslation.find(uiTranslationKey) != m_mapTranslation.end();
	}

	// removes strings that does not exist in the reference translation
	void CLangData::CleanupTranslation(const CLangData& rReferenceTranslation)
	{
		translation_map::iterator iterTranslation = m_mapTranslation.begin();
		while(iterTranslation != m_mapTranslation.end())
		{
			if(!rReferenceTranslation.Exists((*iterTranslation).first))
			{
				m_bModified = true;
				m_mapTranslation.erase(iterTranslation++);
			}
			else
				++iterTranslation;
		}
	}

	void CLangData::SetFilename(PCTSTR psz)
	{
		if(m_pszFilename)
			delete[] m_pszFilename;

		// copy
		m_pszFilename = new TCHAR[ _tcslen(psz) + 1 ];
		_tcscpy(m_pszFilename, psz);

		m_bModified = true;
	}

	PCTSTR CLangData::GetFilename(bool bFullPath) const
	{
		if(bFullPath)
			return m_pszFilename;
		else
		{
			if(m_pszFilename)
			{
				TCHAR *pszFnd = _tcsrchr(m_pszFilename, _T('\\'));
				if(pszFnd)
					return pszFnd + 1;
			}

			return m_pszFilename;
		}
	}

	void CLangData::SetLangName(PCTSTR psz)
	{
		if(m_pszLngName)
			delete[] m_pszLngName;
		m_pszLngName = new TCHAR[ _tcslen(psz) + 1 ];
		_tcscpy(m_pszLngName, psz);
		m_bModified = true;
	}

	void CLangData::SetFontFace(PCTSTR psz)
	{
		if(m_pszFontFace)
			delete[] m_pszFontFace;
		m_pszFontFace = new TCHAR[ _tcslen(psz) + 1 ];
		_tcscpy(m_pszFontFace, psz);
		m_bModified = true;
	}

	void CLangData::SetHelpName(PCTSTR psz)
	{
		SetFnameData(&m_pszHelpName, psz);
		m_bModified = true;
	}

	void CLangData::SetAuthor(PCTSTR psz)
	{
		if(m_pszAuthor)
			delete[] m_pszAuthor;
		m_pszAuthor = new TCHAR[ _tcslen(psz) + 1 ];
		_tcscpy(m_pszAuthor, psz);
		m_bModified = true;
	}

	bool CLangData::IsValidDescription() const
	{
		// basic sanity checks
		if(!m_pszAuthor || m_pszAuthor[ 0 ] == _T('\0') ||
			!m_pszLngName || m_pszLngName[ 0 ] == _T('\0') ||
			!m_pszFontFace || m_pszFontFace[ 0 ] == _T('\0') ||
			m_wPointSize == 0)
			return false;
		return true;
	}

	void CLangData::SetFnameData(PTSTR *ppszDst, PCTSTR pszSrc)
	{
		if(*ppszDst)
			delete[](*ppszDst);
		const TCHAR* pszLast = nullptr;
		if((pszLast = _tcsrchr(pszSrc, _T('\\'))) != nullptr)
			pszLast++;
		else
			pszLast = pszSrc;

		// copy
		*ppszDst = new TCHAR[ _tcslen(pszLast) + 1 ];
		_tcscpy(*ppszDst, pszLast);
	}

	CResourceManager::CResourceManager() :
		m_hRes(nullptr),
		m_pfnCallback(nullptr)
	{
		InitializeCriticalSection(&m_cs);
	}

	CResourceManager::~CResourceManager()
	{
		DeleteCriticalSection(&m_cs);
	}

	CResourceManager& CResourceManager::Acquire()
	{
		return CResourceManager::S_ResourceManager;
	}

	void CResourceManager::Init(HMODULE hrc)
	{
		m_hRes = hrc;
	}

	// requires the param with ending '\\'
	void CResourceManager::Scan(LPCTSTR pszFolder, std::vector<CLangData>* pvData)
	{
		assert(pszFolder);
		assert(pvData);
		if(!pszFolder || !pvData)
			return;

		CString strPath = pszFolder;
		strPath += _T("*.lng");

		WIN32_FIND_DATA wfd;
		HANDLE hFind = ::FindFirstFile(strPath, &wfd);
		BOOL bFound = TRUE;
		CLangData ld;
		while(bFound && hFind != INVALID_HANDLE_VALUE)
		{
			if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strPath = pszFolder;
				strPath += wfd.cFileName;
				if(ld.ReadInfo(strPath))
					pvData->push_back(ld);
			}

			bFound = ::FindNextFile(hFind, &wfd);
		}

		if(hFind != INVALID_HANDLE_VALUE)
			::FindClose(hFind);
	}

	bool CResourceManager::SetLanguage(PCTSTR pszPath)
	{
		CString strBasePath;
		CString strRealPath = pszPath;

		// parse the path to allow reading the English language first
		const CString strEnglishFileName = _T("english.lng");

		CString strTestPath = pszPath;
		strTestPath.MakeLower();
		if(strTestPath.Right(strEnglishFileName.GetLength()) != strEnglishFileName)
		{
			int iPos = strRealPath.ReverseFind(L'\\');
			if(iPos <= 0)
				strBasePath = strEnglishFileName;
			else
				strBasePath = strRealPath.Left(iPos + 1) + strEnglishFileName;
		}
		else
		{
			strBasePath = strRealPath;
			strRealPath.Empty();
		}

		// and load everything
		bool bRet = true;

		EnterCriticalSection(&m_cs);
		try
		{
			if(!strBasePath.IsEmpty())
				bRet = m_ld.ReadTranslation(strBasePath);		// base language
			if(bRet && !strRealPath.IsEmpty())
				bRet = m_ld.ReadTranslation(strRealPath, true);	// real language
		}
		catch(...)
		{
			LeaveCriticalSection(&m_cs);
			return false;
		}
		LeaveCriticalSection(&m_cs);
		if(!bRet)
			return false;

		// update registered dialog boxes
		std::list<CWnd*>::iterator it = m_lhDialogs.begin();
		while(it != m_lhDialogs.end())
		{
			if(::IsWindow((*it)->m_hWnd))
				(*it)->PostMessage(WM_RMNOTIFY, RMNT_LANGCHANGE, 0);
			++it;
		}

		// send the notification stuff to the others
		if(m_pfnCallback)
			(*m_pfnCallback)(RMNT_LANGCHANGE);

		return bRet;
	}

	HGLOBAL CResourceManager::LoadResource(LPCTSTR pszType, LPCTSTR pszName)
	{
		EnterCriticalSection(&m_cs);

		// find resource
		HGLOBAL hRet = nullptr;
		HRSRC hr = FindResource(m_hRes, pszName, pszType);
		if(hr)
			hRet = ::LoadResource(m_hRes, hr);

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
		int iCount = ::GetMenuItemCount(hMenu);
		MENUITEMINFO mif;
		WORD wLoID;
		TCHAR szItem[ 1024 ];
		memset(szItem, 0, 1024);
		for(int i = 0; i < iCount; i++)
		{
			memset(&mif, 0, sizeof(MENUITEMINFO));
			mif.cbSize = sizeof(MENUITEMINFO);
			mif.dwTypeData = szItem;
			mif.cch = 1023;
			mif.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID | MIIM_DATA | MIIM_STRING;
			if(::GetMenuItemInfo(hMenu, i, TRUE, &mif))
			{
				// has sub items ?
				if(mif.hSubMenu)
					UpdateMenu(mif.hSubMenu, wMenuID);

				// the menu item contains a string to update
				if(mif.fType == MFT_STRING)
				{
					if(mif.hSubMenu)
					{
						if(mif.dwItemData != 0)
							wLoID = (WORD)mif.dwItemData;		// already updated data
						else
						{
							// fresh menu - try to update info from caption
							wLoID = (WORD)_ttoi((PCTSTR)mif.dwTypeData);

							// remember this info in item's private storage
							MENUITEMINFO ii;
							ii.cbSize = sizeof(MENUITEMINFO);
							ii.fMask = MIIM_DATA;
							::SetMenuItemInfo(hMenu, i, TRUE, &ii);
						}
					}
					else
						wLoID = (WORD)::GetMenuItemID(hMenu, i);

					mif.fMask = MIIM_STRING | MIIM_FTYPE;
					if(m_ld.GetDirection())
						mif.fType |= MFT_RIGHTORDER;
					else
						mif.fType &= ~MFT_RIGHTORDER;
					mif.dwTypeData = (LPTSTR)(m_ld.GetString(wMenuID, wLoID));
					::SetMenuItemInfo(hMenu, i, TRUE, &mif);
				}
			}
		}
	}

	HMENU CResourceManager::LoadMenu(LPCTSTR pszName)
	{
		EnterCriticalSection(&m_cs);
		HMENU hMenu = ::LoadMenu(m_hRes, pszName);

		if(hMenu && IS_INTRESOURCE(pszName))
			UpdateMenu(hMenu, (WORD)(ULONG_PTR)pszName);

		LeaveCriticalSection(&m_cs);
		return hMenu;
	}

	LPDLGTEMPLATE CResourceManager::LoadDialog(LPCTSTR pszName)
	{
		HGLOBAL hgl = LoadResource(RT_DIALOG, pszName);
		DLGTEMPLATE *dlgt = (DLGTEMPLATE*)::LockResource(hgl);
		return dlgt;
	}

	PCTSTR CResourceManager::LoadString(UINT uiID)
	{
		EnterCriticalSection(&m_cs);
		PCTSTR pszData = m_ld.GetString(0, (WORD)uiID);
		LeaveCriticalSection(&m_cs);

		return pszData;
	}

	PCTSTR CResourceManager::LoadString(WORD wGroup, WORD wID)
	{
		EnterCriticalSection(&m_cs);
		PCTSTR pszData = m_ld.GetString(wGroup, wID);
		LeaveCriticalSection(&m_cs);
		return pszData;
	}

	HANDLE CResourceManager::LoadImage(LPCTSTR lpszName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad)
	{
		EnterCriticalSection(&m_cs);

		HANDLE hImg = ::LoadImage(m_hRes, lpszName, uType, cxDesired, cyDesired, fuLoad);

		LeaveCriticalSection(&m_cs);
		return hImg;
	}
}
