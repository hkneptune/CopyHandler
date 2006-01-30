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
#include "ConfigManager.h"
#ifndef DISABLE_CRYPT
#include "crypt.h"
#include "conv.h"
#endif

#define CFG_PROFILE			_T("Common")
#define CFG_SECTION			_T("Config")
#define CFG_KEY				_T("Current configuration name")
#define CFG_DEFAULTSECTION	_T("Default")

#define CFG_MAXCOMPOUND		500	/* max count of items in a compound property */

///////////////////////////////////////////////////////////////
// Opens configuration file.
// This function should be called only once - isn't thread safe
// Doesn't throw any file exception. If physical file couldn't
// be opened - this class does not require it. Could work
// without saving options to file, or with read-only files.
// pszFilename [in] - path to the configuration file
///////////////////////////////////////////////////////////////
void CConfigManager::Open(LPCTSTR pszFilename)
{
	// load text file, and process it in ini file
	try
	{
		((CIniFile*)this)->Open(pszFilename, NULL, false);
	}
	catch(CFileExceptionEx* e)
	{
		// report error
		TCHAR szData[1024];
		szData[1023]=_T('\0');
		_sntprintf(szData, 1023, _T("[Config Manager]: Couldn't open file %s to read initial data;\r\n\tReason: %s."), pszFilename, e->m_pszReason);
		Report(szData);
		
		delete e;		// get rid of this exception (file works doesn't work)

		// file couldn't be loaded... - create default section
		m_pszCurrentConfig=new TCHAR[8];
		_tcscpy(m_pszCurrentConfig, CFG_DEFAULTSECTION);
		SetString(CFG_PROFILE, CFG_SECTION, CFG_KEY, m_pszCurrentConfig);
		return;
	}

	// now - read current config name from cfg
	if (!GetStr(CFG_PROFILE, CFG_SECTION, CFG_KEY, m_szBuffer, _T("")))
	{
		// current config not found - the first found would be current
		vector<_PROFILE*>::const_iterator it=m_vConfigProfiles.begin();

		while (it != m_vConfigProfiles.end())
		{
			// check if element 0 isn't <common>
			if (_tcsicmp((*it)->pszProfileName, CFG_PROFILE) != 0)
			{
				// set current cfg to the found one
				m_pszCurrentConfig=new TCHAR[_tcslen((*it)->pszProfileName)+1];
				_tcscpy(m_pszCurrentConfig, (*it)->pszProfileName);

				// save current selection
				SetString(CFG_PROFILE, CFG_SECTION, CFG_KEY, m_pszCurrentConfig);
				break;	// we've found what was needed - stop processing
			}
			it++;
		}

		// if not found - create default
		if (m_pszCurrentConfig == NULL)
		{
			m_pszCurrentConfig=new TCHAR[8];
			_tcscpy(m_pszCurrentConfig, CFG_DEFAULTSECTION);
			SetString(CFG_PROFILE, CFG_SECTION, CFG_KEY, m_pszCurrentConfig);
		}
	}
	else
	{
		// section found - copy it
		m_pszCurrentConfig=new TCHAR[_tcslen(m_szBuffer)+1];
		_tcscpy(m_pszCurrentConfig, m_szBuffer);
	}
}

///////////////////////////////////////////////////////////////
// Tries to save settings to the configuration file. If
// couldn't be saved - function reports it through Report func
// and doesn't save data to file (this allows to use read-only
// attribute on config file).
///////////////////////////////////////////////////////////////
void CConfigManager::Save()
{
	EnterCriticalSection(&m_cs);
	// copy data into ini file object
	vector<_CFGENTRY*>::const_iterator it=m_vCfgData.begin();
	while (it != m_vCfgData.end())
		WriteProperty(*it++);

	// save file
	try
	{
		((CIniFile*)this)->Save();
	}
	catch(CFileExceptionEx* e)
	{
		TCHAR szData[1024];
		e->GetInfo(_T("[Config Manager]: Couldn't save configuration file."), szData, 1024);
		Report(szData);
		delete e;

		LeaveCriticalSection(&m_cs);
		return;
	}
	catch(...)
	{
		LeaveCriticalSection(&m_cs);
		throw;
	}

	LeaveCriticalSection(&m_cs);
}

///////////////////////////////////////////////////////////////
// Closes this configuration file (saves data before closing).
// If couldn't be saved/closed - it reports the fact and allows
// to continue (allows usage of read-only files).
///////////////////////////////////////////////////////////////
void CConfigManager::Close()
{
	// store all data into .ini object
	EnterCriticalSection(&m_cs);
	vector<_CFGENTRY*>::const_iterator it=m_vCfgData.begin();
	while (it != m_vCfgData.end())
	{
		WriteProperty(*it);
		FreeEntry(*it++);
	}

	// free all
	m_vCfgData.clear();

	// free current config name
	delete [] m_pszCurrentConfig;
	m_pszCurrentConfig=NULL;

	// destroy base...
	try
	{
		((CIniFile*)this)->Close();		// storing data, closing file in a base class
	}
	catch(CFileExceptionEx* e)
	{
		TCHAR szData[1024];
		e->GetInfo(_T("[Config Manager]: Couldn't properly save/close file."), szData, 1024);
		Report(szData);

		delete e;

		LeaveCriticalSection(&m_cs);
		return;
	}
	catch(...)
	{
		LeaveCriticalSection(&m_cs);
		throw;
	}
	
	LeaveCriticalSection(&m_cs);
}

#ifndef DISABLE_CRYPT
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CConfigManager::SetPassword(PCTSTR pszPass)
{
	// gen new 256b password
	TCHAR szNewPass[64+1], szLine[8192];
	StringToKey256(pszPass, szNewPass);

	// recrypt the old data if needed
	if (m_pszPassword)
	{
		// if there already was a password
		for (vector<_CFGENTRY*>::iterator it=m_vCfgData.begin();it!=m_vCfgData.end();it++)
		{
			// reencrypt every encrypted item
			if ((*it)->dwFlags & RF_ENCRYPTED && (*it)->cType == PT_STRING)
			{
				// decrypt the old data
				if (AES256DecipherString((*it)->val.pszValue, m_pszPassword, szLine) < 0)
					continue;

				// encrypt to the new one
				if (AES256CipherString(szLine, szNewPass, szLine) < 0)
					continue;

				// store the new value
				delete [] (*it)->val.pszValue;
				(*it)->val.pszValue=new TCHAR[_tcslen(szLine)+1];
				_tcscpy((*it)->val.pszValue, szLine);
				(*it)->bModified=true;		// set the modified flag
			}
		}

		delete [] m_pszPassword;		// delete old password
	}
	m_pszPassword=new TCHAR[64+1];
	_tcscpy(m_pszPassword, szNewPass);
}

#endif
///////////////////////////////////////////////////////////////
// Registers some property given filled _CFGENTRY struct.
// pEntry [in] - specifies all possible information about the
//		attribute being registered.
// Ret Value [out] - ID of registered property. It must be
//		used when accessing this property.
///////////////////////////////////////////////////////////////
UINT CConfigManager::RegisterProperty(_CFGENTRY* pEntry)
{
	_ASSERT(pEntry != NULL);

	EnterCriticalSection(&m_cs);
	ReadProperty(pEntry);
	m_vCfgData.push_back(pEntry);
	UINT uiRet=(UINT)(m_vCfgData.size()-1);
	LeaveCriticalSection(&m_cs);
	return uiRet;
}

///////////////////////////////////////////////////////////////
// Registers int64 property.
// pszSection [in] - section in which resides given property
// pszKey [in] - key in which resides given property
// llDefault [in] - default value of this property (used if the
//				config file doesn't contain it)
// cDetailLevel [in] - specifies "level" of this property. It 
//			could be simple property (for all users) or
//			something that only some paranoics will use.
// Ret Value [out] - ID of registered property. It must be
//		used when accessing this property.
///////////////////////////////////////////////////////////////
UINT CConfigManager::RegisterInt64Property(LPCTSTR pszSection, LPCTSTR pszKey, __int64 llDefault, char cDetailLevel)
{
	// create config entry
	_CFGENTRY* pcfg=new _CFGENTRY;
	pcfg->pszSection=new TCHAR[_tcslen(pszSection)+1];
	_tcscpy(pcfg->pszSection, pszSection);
	pcfg->pszKey=new TCHAR[_tcslen(pszKey)+1];
	_tcscpy(pcfg->pszKey, pszKey);
	pcfg->cType=PT_INT64;
	pcfg->def.llValue=llDefault;
	pcfg->bModified=false;
	pcfg->cDetailLevel=cDetailLevel;
	pcfg->dwFlags=RF_NONE;

	// register
	return RegisterProperty(pcfg);
}

///////////////////////////////////////////////////////////////
// Registers int property.
// pszSection [in] - section in which resides given property
// pszKey [in] - key in which resides given property
// iDefault [in] - default value of this property (used if the
//				config file doesn't contain it)
// cDetailLevel [in] - specifies "level" of this property. It 
//			could be simple property (for all users) or
//			something that only some paranoics will use.
// Ret Value [out] - ID of registered property. It must be
//		used when accessing this property.
///////////////////////////////////////////////////////////////
UINT CConfigManager::RegisterIntProperty(LPCTSTR pszSection, LPCTSTR pszKey, int iDefault, char cDetailLevel)
{
	// create config entry
	_CFGENTRY* pcfg=new _CFGENTRY;
	pcfg->pszSection=new TCHAR[_tcslen(pszSection)+1];
	_tcscpy(pcfg->pszSection, pszSection);
	pcfg->pszKey=new TCHAR[_tcslen(pszKey)+1];
	_tcscpy(pcfg->pszKey, pszKey);
	pcfg->cType=PT_INT;
	pcfg->def.iValue=iDefault;
	pcfg->bModified=false;
	pcfg->cDetailLevel=cDetailLevel;
	pcfg->dwFlags=RF_NONE;

	// register
	return RegisterProperty(pcfg);
}

///////////////////////////////////////////////////////////////
// Registers bool property.
// pszSection [in] - section in which resides given property
// pszKey [in] - key in which resides given property
// bDefault [in] - default value of this property (used if the
//				config file doesn't contain it)
// cDetailLevel [in] - specifies "level" of this property. It 
//			could be simple property (for all users) or
//			something that only some paranoics will use.
// Ret Value [out] - ID of registered property. It must be
//		used when accessing this property.
///////////////////////////////////////////////////////////////
UINT CConfigManager::RegisterBoolProperty(LPCTSTR pszSection, LPCTSTR pszKey, bool bDefault, char cDetailLevel)
{
	// create config entry
	_CFGENTRY* pcfg=new _CFGENTRY;
	pcfg->pszSection=new TCHAR[_tcslen(pszSection)+1];
	_tcscpy(pcfg->pszSection, pszSection);
	pcfg->pszKey=new TCHAR[_tcslen(pszKey)+1];
	_tcscpy(pcfg->pszKey, pszKey);
	pcfg->cType=PT_BOOL;
	pcfg->def.bValue=bDefault;
	pcfg->bModified=false;
	pcfg->cDetailLevel=cDetailLevel;
	pcfg->dwFlags=RF_NONE;

	// register
	return RegisterProperty(pcfg);
}

///////////////////////////////////////////////////////////////
// Registers string property.
// pszSection [in] - section in which resides given property
// pszKey [in] - key in which resides given property
// pszDefault [in] - default value of this property (used if
//				the config file doesn't contain it)
// cDetailLevel [in] - specifies "level" of this property. It 
//			could be simple property (for all users) or
//			something that only some paranoics will use.
// Ret Value [out] - ID of registered property. It must be
//		used when accessing this property.
///////////////////////////////////////////////////////////////
UINT CConfigManager::RegisterStringProperty(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, DWORD dwFlags, char cDetailLevel)
{
	// create config entry
	_CFGENTRY* pcfg=new _CFGENTRY;
	pcfg->pszSection=new TCHAR[_tcslen(pszSection)+1];
	_tcscpy(pcfg->pszSection, pszSection);
	pcfg->pszKey=new TCHAR[_tcslen(pszKey)+1];
	_tcscpy(pcfg->pszKey, pszKey);
	pcfg->cType=PT_STRING;
	pcfg->def.pszValue=new TCHAR[_tcslen(pszDefault)+1];
	_tcscpy(pcfg->def.pszValue, pszDefault);
	pcfg->bModified=false;
	pcfg->cDetailLevel=cDetailLevel;
	pcfg->dwFlags=dwFlags;

	// register
	return RegisterProperty(pcfg);
}

UINT CConfigManager::RegisterStringArrayProperty(LPCTSTR pszSection, char cDetailLevel)
{
	_CFGENTRY* pcfg=new _CFGENTRY;
	pcfg->pszSection=new TCHAR[_tcslen(pszSection)+1];
	_tcscpy(pcfg->pszSection, pszSection);
	pcfg->pszKey=NULL;
	pcfg->cType=PT_ASTRING;
	pcfg->bModified=false;
	pcfg->cDetailLevel=cDetailLevel;
	pcfg->val.pvszValue=new char_vector;
	pcfg->dwFlags=RF_NONE;

	return RegisterProperty(pcfg);
}

///////////////////////////////////////////////////////////////
// Returns value of int property given property ID
// uiPropID [in] - property ID (returned by RegisterProperty..)
// Ret Value [out] - value of this property (either from file
//				or the default one specified at registration)
///////////////////////////////////////////////////////////////
int CConfigManager::GetIntValue(UINT uiPropID)
{
	EnterCriticalSection(&m_cs);
	int iRet=m_vCfgData.at(uiPropID)->val.iValue;
	LeaveCriticalSection(&m_cs);
	return iRet;
}

///////////////////////////////////////////////////////////////
// Sets value of a given property (by it's ID)
// uiPropID [in] - property ID (returned by RegisterProperty..)
// iValue [in] - the new value of this property
///////////////////////////////////////////////////////////////
void CConfigManager::SetIntValue(UINT uiPropID, int iValue)
{
	EnterCriticalSection(&m_cs);
	m_vCfgData.at(uiPropID)->val.iValue=iValue;
	m_vCfgData.at(uiPropID)->bModified=true;
	LeaveCriticalSection(&m_cs);

	if (m_pfnCallback)
		(*m_pfnCallback)(0, WM_CFGNOTIFY, CNFT_PROPERTYCHANGE, uiPropID);
}

///////////////////////////////////////////////////////////////
// Returns value of int64 property given property ID
// uiPropID [in] - property ID (returned by RegisterProperty..)
// Ret Value [out] - value of this property (either from file
//				or the default one specified at registration)
///////////////////////////////////////////////////////////////
__int64 CConfigManager::GetInt64Value(UINT uiPropID)
{
	EnterCriticalSection(&m_cs);
	__int64 llRet=m_vCfgData.at(uiPropID)->val.llValue;
	LeaveCriticalSection(&m_cs);
	return llRet;
}

///////////////////////////////////////////////////////////////
// Sets value of a given property (by it's ID)
// uiPropID [in] - property ID (returned by RegisterProperty..)
// llValue [in] - the new value of this property
///////////////////////////////////////////////////////////////
void CConfigManager::SetInt64Value(UINT uiPropID, __int64 llValue)
{
	EnterCriticalSection(&m_cs);
	m_vCfgData.at(uiPropID)->val.llValue=llValue;
	m_vCfgData.at(uiPropID)->bModified=true;
	LeaveCriticalSection(&m_cs);

	if (m_pfnCallback)
		(*m_pfnCallback)(0, WM_CFGNOTIFY, CNFT_PROPERTYCHANGE, uiPropID);
}

///////////////////////////////////////////////////////////////
// Returns value of bool property given property ID
// uiPropID [in] - property ID (returned by RegisterProperty..)
// Ret Value [out] - value of this property (either from file
//				or the default one specified at registration)
///////////////////////////////////////////////////////////////
bool CConfigManager::GetBoolValue(UINT uiPropID)
{
	EnterCriticalSection(&m_cs);
	bool bRet=m_vCfgData.at(uiPropID)->val.bValue;
	LeaveCriticalSection(&m_cs);
	return bRet;
}

///////////////////////////////////////////////////////////////
// Sets value of a given property (by it's ID)
// uiPropID [in] - property ID (returned by RegisterProperty..)
// bValue [in] - the new value of this property
///////////////////////////////////////////////////////////////
void CConfigManager::SetBoolValue(UINT uiPropID, bool bValue)
{
	EnterCriticalSection(&m_cs);
	m_vCfgData.at(uiPropID)->val.bValue=bValue;
	m_vCfgData.at(uiPropID)->bModified=true;
	LeaveCriticalSection(&m_cs);

	if (m_pfnCallback)
		(*m_pfnCallback)(0, WM_CFGNOTIFY, CNFT_PROPERTYCHANGE, uiPropID);
}

///////////////////////////////////////////////////////////////
// Returns value of string property given property ID
// uiPropID [in] - property ID (returned by RegisterProperty..)
// Ret Value [out] - value of this property (either from file
//				or the default one specified at registration)
///////////////////////////////////////////////////////////////
PCTSTR CConfigManager::GetStringValue(UINT uiPropID, PTSTR pszBuffer, int iSize)
{
	EnterCriticalSection(&m_cs);

#ifndef DISABLE_CRYPT
	if (m_vCfgData.at(uiPropID)->dwFlags & RF_ENCRYPTED)
	{
		// make sure password has been set already
		_ASSERT(m_pszPassword);

		TCHAR szLine[8192];
		if (AES256DecipherString(m_vCfgData.at(uiPropID)->val.pszValue, m_pszPassword, szLine) <= 0)
		{
			// return 
			_tcscpy(pszBuffer, _T(""));
			LeaveCriticalSection(&m_cs);
			return pszBuffer;
		}

		// copy the decrypted data
		_tcsncpy(pszBuffer, szLine, iSize);
		pszBuffer[iSize-1]=_T('\0');
	}
	else
#endif
		_tcsncpy(pszBuffer, m_vCfgData.at(uiPropID)->val.pszValue, iSize);

	if (m_vCfgData.at(uiPropID)->dwFlags & RF_PATH)
	{
		if (pszBuffer[_tcslen(pszBuffer)-1] != _T('\\'))
			_tcscat(pszBuffer, _T("\\"));
	}

	LeaveCriticalSection(&m_cs);

	return pszBuffer;
}

///////////////////////////////////////////////////////////////
// Sets value of a given property (by it's ID)
// uiPropID [in] - property ID (returned by RegisterProperty..)
// pszValue [in] - the new value of this property
///////////////////////////////////////////////////////////////
void CConfigManager::SetStringValue(UINT uiPropID, LPCTSTR pszValue)
{
	EnterCriticalSection(&m_cs);
#ifndef DISABLE_CRYPT
	// encrypt the data before setting the property text
	if (m_vCfgData.at(uiPropID)->dwFlags & RF_ENCRYPTED)
	{
		// make sure password has been set already
		_ASSERT(m_pszPassword);

		TCHAR szLine[8192];
		if (AES256CipherString(pszValue, m_pszPassword, szLine) < 0)
		{
			_ASSERT(false);
			TRACE("Cannot cipher string - what to do ????\n");
		}

		// store encrypted value
		delete [] m_vCfgData.at(uiPropID)->val.pszValue;		// delete old value
		m_vCfgData.at(uiPropID)->val.pszValue=new TCHAR[_tcslen(szLine)+1];	// create space for new
		_tcscpy(m_vCfgData.at(uiPropID)->val.pszValue, szLine);	// copy the new one
		m_vCfgData.at(uiPropID)->bModified=true;			// modified state
	}
	else
	{
#endif
		delete [] m_vCfgData.at(uiPropID)->val.pszValue;		// delete old value
		m_vCfgData.at(uiPropID)->val.pszValue=new TCHAR[_tcslen(pszValue)+1];	// create space for new
		_tcscpy(m_vCfgData.at(uiPropID)->val.pszValue, pszValue);	// copy the new one
		m_vCfgData.at(uiPropID)->bModified=true;			// modified state
#ifndef DISABLE_CRYPT
	}
#endif
	LeaveCriticalSection(&m_cs);

	if (m_pfnCallback)
		(*m_pfnCallback)(0, WM_CFGNOTIFY, CNFT_PROPERTYCHANGE, uiPropID);
}

void CConfigManager::GetStringArrayValue(UINT uiPropID, char_vector* pcVector)
{
	EnterCriticalSection(&m_cs);
	_CFGENTRY* pentry=m_vCfgData.at(uiPropID);
	pcVector->assign(pentry->val.pvszValue->begin(), pentry->val.pvszValue->end(), true, true);
	LeaveCriticalSection(&m_cs);
}

void CConfigManager::SetStringArrayValue(UINT uiPropID, char_vector* pcVector)
{
	EnterCriticalSection(&m_cs);
	_CFGENTRY* pentry=m_vCfgData.at(uiPropID);
	pentry->val.pvszValue->assign(pcVector->begin(), pcVector->end(), true, true);
	LeaveCriticalSection(&m_cs);

	if (m_pfnCallback)
		(*m_pfnCallback)(0, WM_CFGNOTIFY, CNFT_PROPERTYCHANGE, uiPropID);
}

///////////////////////////////////////////////////////////////
// Selects new profile
// pszCfgName [in] - new profile name
///////////////////////////////////////////////////////////////
void CConfigManager::SelectProfile(LPCTSTR pszCfgName)
{
	// store config into ini file
	vector<_CFGENTRY*>::const_iterator it=m_vCfgData.begin();
	while (it != m_vCfgData.end())
		WriteProperty(*it++);

	// change config
	delete [] m_pszCurrentConfig;
	m_pszCurrentConfig=new TCHAR[_tcslen(pszCfgName)+1];
	_tcscpy(m_pszCurrentConfig, pszCfgName);

	// read data from ini file
	it=m_vCfgData.begin();
	while (it != m_vCfgData.end())
		ReadProperty(*it++);

	// set data in ini file
	SetString(CFG_PROFILE, CFG_SECTION, CFG_KEY, pszCfgName);

	if (m_pfnCallback)
		(*m_pfnCallback)(0, WM_CFGNOTIFY, CNFT_PROFILECHANGE, (LPARAM)pszCfgName);
}

///////////////////////////////////////////////////////////////
// (Internal) Frees one of the profile entries
// pEntry [in] - address of profile entry
///////////////////////////////////////////////////////////////
void CConfigManager::FreeEntry(_CFGENTRY* pEntry)
{
	// free section and key
	delete [] pEntry->pszSection;
	delete [] pEntry->pszKey;

	// if this is a string property - free it
	switch(pEntry->cType)
	{
	case PT_STRING:
		delete [] pEntry->def.pszValue;
		delete [] pEntry->val.pszValue;
		break;
	case PT_ASTRING:
		{
			pEntry->val.pvszValue->clear(true);
			delete pEntry->val.pvszValue;
			break;
		}
	}

	// last - delete object itself
	delete pEntry;
}

///////////////////////////////////////////////////////////////
// Reads value of a given property entry from .ini file
// pEntry [in] - filled property entry structure
///////////////////////////////////////////////////////////////
void CConfigManager::ReadProperty(_CFGENTRY* pEntry)
{
	// process each attribute
	switch (pEntry->cType)
	{
	case PT_INT64:
		{
			pEntry->val.llValue=GetInt64(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->def.llValue, m_szBuffer);
			break;
		}
	case PT_STRING:
		{
			GetString(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, m_szBuffer, pEntry->def.pszValue);
			pEntry->val.pszValue=new TCHAR[_tcslen(m_szBuffer)+1];
			_tcscpy(pEntry->val.pszValue, m_szBuffer);
			break;
		}
	case PT_BOOL:
		{
			pEntry->val.bValue=GetBool(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->def.bValue, m_szBuffer);
			break;
		}
	case PT_INT:
		{
			pEntry->val.iValue=GetInt(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->def.iValue, m_szBuffer);
			break;
		}
	case PT_ASTRING:
		{
			TCHAR szNum[8];
			for (int i=0;i<CFG_MAXCOMPOUND;i++)
			{
				_itot(i, szNum, 10);
				if (GetStr(m_pszCurrentConfig, pEntry->pszSection, szNum, m_szBuffer, _T("")))
					pEntry->val.pvszValue->push_back(m_szBuffer, true);
				else
					break;
			}

			break;
		}
	default:
		_ASSERT(false);		// unknown property type
	}

	// set modified if this wasn't read from a file
	pEntry->bModified=m_bDefault;	// if this is a default value - store it when needed
}

///////////////////////////////////////////////////////////////
// Writes property to the .ini file
// pEntry [in] - filled property entry struct
///////////////////////////////////////////////////////////////
void CConfigManager::WriteProperty(_CFGENTRY* pEntry)
{
	if (pEntry->bModified)
	{
		switch (pEntry->cType)
		{
		case PT_INT64:
			{
				SetInt64(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->val.llValue);
				pEntry->bModified=false;
				break;
			}
		case PT_STRING:
			{
				SetString(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->val.pszValue);
				pEntry->bModified=false;
				break;
			}
		case PT_BOOL:
			{
				SetBool(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->val.bValue);
				pEntry->bModified=false;
				break;
			}
		case PT_INT:
			{
				SetInt(m_pszCurrentConfig, pEntry->pszSection, pEntry->pszKey, pEntry->val.iValue);
				pEntry->bModified=false;
				break;
			}
		case PT_ASTRING:
			{
				TCHAR szNum[8];
				for (int i=0;i<CFG_MAXCOMPOUND;i++)
				{
					_itot(i, szNum, 10);
					if (i < (int)pEntry->val.pvszValue->size())
					{
						// add string
						SetString(m_pszCurrentConfig, pEntry->pszSection, szNum, pEntry->val.pvszValue->at(i));
					}
					else
					{
						// remove string
						RemoveKey(m_pszCurrentConfig, pEntry->pszSection, szNum, true);
						break;
					}
				}
				pEntry->bModified=false;

				break;
			}
		}
	}
}

LRESULT CConfigManager::MsgRouter(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
	case CCMI_REGISTERPROPERTY:
		return (LRESULT)RegisterProperty((_CFGENTRY*)lParam);
	case CCMI_GETINTVALUE:
		return (LRESULT)GetIntValue((UINT)wParam);
	case CCMI_SETINTVALUE:
		SetIntValue((UINT)wParam, (int)lParam);
		return (LRESULT)0;
	case CCMI_GETINT64VALUE:
		{
			if (lParam)
			{
				*((__int64*)lParam)=GetInt64Value((UINT)wParam);
				return (LRESULT)0;
			}
			else
				return (LRESULT)1;
		}
	case CCMI_SETINT64VALUE:
		{
			if (lParam)
			{
				SetInt64Value((UINT)wParam, *((__int64*)lParam));
				return (LRESULT)0;
			}
			else
				return (LRESULT)1;
		}
	case CCMI_GETBOOLVALUE:
		return (LRESULT)GetBoolValue((UINT)wParam);
	case CCMI_SETBOOLVALUE:
		SetBoolValue((UINT)wParam, lParam != 0);
		return (LRESULT)0;
	case CCMI_GETSTRINGVALUE:
		GetStringValue((UINT)wParam, (LPTSTR)lParam, 1024);
		return (LRESULT)0;
	case CCMI_SETSTRINGVALUE:
		SetStringValue((UINT)wParam, (LPCTSTR)lParam);
		return (LRESULT)0;
	case CCMI_SELECTPROFILE:
		SelectProfile((LPCTSTR)lParam);
		return (LRESULT)0;
	}
	return (LRESULT)1;
}
