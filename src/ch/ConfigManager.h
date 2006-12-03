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
/*************************************************************************
	File: ConfigManager.h
	Version: 1.0
	Author: Ixen Gerthannes (ixen@interia.pl)
	File description:
		Contains structs/classes for handling configuration
		settings in application.
	Classes:
		CConfigManager
			- support access to properties by it's ID
			- supports registering additional properties when
				the program works.
			- supports multiple levels of property detail
			- special meaning - <Common>; [Config];
				"Current configuration name=xxx" - specifies
				current active profile (the one that will be used
				as a source for getting data for properties).
	Structures:
		_CFGENTRY - contain information about one property.
*************************************************************************/

#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include "IniFile.h"
#include "charvect.h"
#include "af_defs.h"

using namespace std;

// property types in CConfigEntry
#define PT_INT64	0
#define PT_STRING	1
#define PT_BOOL		2
#define PT_INT		3
#define PT_ASTRING	4	/* array of strings */

// flags when registering
#define RF_NONE			0x00
#define RF_PATH			0x01

#ifndef DISABLE_CRYPT
#define RF_ENCRYPTED	0x02
#endif

// property detail level
// general properties - for everyone
#define PDL_GENERAL			0
// advanced properties - for advanced users
#define PDL_ADVANCED		1
// paranoid type - for only a few individuals who want to configure everything
#define PDL_PARANOID		2

// types of notifications (going out)
// CNFT_PROFILECHANGE - this notification is sent when something changes the current configuration profile
//	LPARAM - address of new profile string
#define CNFT_PROFILECHANGE		0x0001
// CNFT_PROPERTYCHANGE - tells about changing value of a property
//	LPARAM - (UINT) property ID
#define CNFT_PROPERTYCHANGE		0x0002

// messages that goes into this class
#define CCMI_REGISTERPROPERTY	0x0001
#define CCMI_GETINTVALUE		0x0002
#define CCMI_SETINTVALUE		0x0003
#define CCMI_GETINT64VALUE		0x0004
#define CCMI_SETINT64VALUE		0x0005
#define CCMI_GETBOOLVALUE		0x0006
#define CCMI_SETBOOLVALUE		0x0007
#define CCMI_GETSTRINGVALUE		0x0008
#define CCMI_SETSTRINGVALUE		0x0009
#define CCMI_SELECTPROFILE		0x000a


// defines one of the attributes
struct _CFGENTRY
{
	TCHAR* pszSection;		// section name
	TCHAR* pszKey;			// key name
	char cDetailLevel;		// detail level of this property (GENERAL (for everyone), ADVANCED (for advanced users), PARANOID)
	bool bModified;			// if property is in modified state (it wasn't saved from last change)
	char cType;				// type of property
	DWORD dwFlags;			// if set and property is string then we need to append '\\' at the end
	union DATA
	{
		__int64 llValue;	// __int64 value type
		TCHAR* pszValue;	// string value type
		int iValue;			// int value type
		bool bValue;		// bool value type
		char_vector* pvszValue;	// compound string table
	} val, def;
};


class CConfigManager : public CIniFile
{
public:
	CConfigManager() : CIniFile() { m_pszCurrentConfig=NULL; m_pfnCallback=NULL;
#ifndef DISABLE_CRYPT
	m_pszPassword=NULL;
#endif
	InitializeCriticalSection(&m_cs); };
	~CConfigManager() { Close();
#ifndef DISABLE_CRYPT
	delete [] m_pszPassword;
#endif
	DeleteCriticalSection(&m_cs); };

	void SetCallback(PFNNOTIFYCALLBACK pfn) { m_pfnCallback=pfn; };

	LRESULT MsgRouter(UINT uiMsg, WPARAM wParam, LPARAM lParam);

	// open/close funcs
	void Open(LPCTSTR pszFilename);		// reads all registered attributes from file
	void Save();						// saves data to file
	void Close();						// destroys everything

#ifndef DISABLE_CRYPT
	// encryption options
	void SetPassword(PCTSTR pszPass);	// sets the encrypt/decrypt password
#endif

	// properties registration funcs
	UINT RegisterProperty(_CFGENTRY* pEntry);	// just adds pointer to a vector
	UINT RegisterInt64Property(LPCTSTR pszSection, LPCTSTR pszKey, __int64 llDefault, char cDetailLevel=PDL_GENERAL);
	UINT RegisterIntProperty(LPCTSTR pszSection, LPCTSTR pszKey, int iDefault, char cDetailLevel=PDL_GENERAL);
	UINT RegisterBoolProperty(LPCTSTR pszSection, LPCTSTR pszKey, bool bDefault, char cDetailLevel=PDL_GENERAL);
	UINT RegisterStringProperty(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, DWORD dwFlags=RF_NONE, char cDetailLevel=PDL_GENERAL);
	UINT RegisterStringArrayProperty(LPCTSTR pszSection, char cDetailLevel=PDL_GENERAL);

	// property get/set funcs
	int GetIntValue(UINT uiPropID);
	void SetIntValue(UINT uiPropID, int iValue);
	__int64 GetInt64Value(UINT uiPropID);
	void SetInt64Value(UINT uiPropID, __int64 llValue);
	bool GetBoolValue(UINT uiPropID);
	void SetBoolValue(UINT uiPropID, bool bValue);
	PCTSTR GetStringValue(UINT uiPropID, PTSTR pszBuffer, int iSize);
	void SetStringValue(UINT uiPropID, LPCTSTR pszValue);
	void GetStringArrayValue(UINT uiPropID, char_vector* pcVector);
	void SetStringArrayValue(UINT uiPropID, char_vector* pcVector);

	// config profiles management
	void SelectProfile(LPCTSTR pszCfgName);

protected:
	void ReadProperty(_CFGENTRY* pEntry);		// reads value from ini file...
	void WriteProperty(_CFGENTRY* pEntry);		// store value to ini file
	void FreeEntry(_CFGENTRY* pEntry);			// frees entry with data

	void Report(LPCTSTR /*pszReportString*/) { /* currently nothing */ };		// reports errors and others

protected:
	TCHAR* m_pszCurrentConfig;			// name of current config
	vector<_CFGENTRY*> m_vCfgData;		// config data - all attributes
	TCHAR m_szBuffer[1024];				// internal buffer
    
#ifndef DISABLE_CRYPT
	// passwd
	TCHAR* m_pszPassword;				// password for the encrypted properties
#endif

	PFNNOTIFYCALLBACK m_pfnCallback;	// callback func for reporting some data
	CRITICAL_SECTION m_cs;
};

#endif