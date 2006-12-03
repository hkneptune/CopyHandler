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
	File: IniFile.h
	Version: 1.0
	Author: Ixen Gerthannes (ixen@interia.pl)
	File description:
		Contains classes/structures providing functionality of ini
		file. Ini file in this case is a bit modified, because it
		provides something called profile. It could be used to
		maintain more than one set of options in an ini file.
	Classes:
		CIniFile
		- handles ini files in format:
			- <profile name>
			- [section name]
			- key=value
		- work with/without mfc.
		- NOT thread-safe
	Structures:
		_PROFILE - contains profile name and array of sections
		_SECTION - contains section name and array of entries
		_ENTRY - contains key name, value in string format
*************************************************************************/

#ifndef __INIFILE_H__
#define __INIFILE_H__

#include "FileEx.h"
#include <vector>

using namespace std;

// internal structures used in managing profiles, keys, ...
struct _ENTRY
{
	TCHAR* pszKey;			// key name - common for all config profiles
	TCHAR* pszValue;		// set of values bound to above key - differs between config-profiles
};

struct _SECTION
{
	TCHAR* pszSectionName;
    vector<_ENTRY*> vEntries;
};

struct _PROFILE
{
	TCHAR* pszProfileName;
	vector<_SECTION*> vSections;
};

// class
class CIniFile
{
public:
	// construction/destruction
	CIniFile() { m_pszFilename=NULL; m_bModified=false; m_bDefault=false; };
	~CIniFile() { Close(); };

	// opening/closing
	void Open(LPCTSTR pszFilename, PCTSTR pszOneSection=NULL, bool bEscapeConversion=false);			// loads data from file and interpretes it
	void Save();							// saves data to file (without closing it)
	void Close();							// saves file and closes it

	// reading/writing some data from/to .ini file
	bool GetStr(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, LPTSTR pszValue, LPCTSTR pszDefault);	// gets string from .ini file
	LPTSTR GetString(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, LPTSTR pszValue, LPCTSTR pszDefault);	// gets string from .ini file
	void SetString(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszValue);	// sets string in .ini file

	void SetInt(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, int iValue);
	int GetInt(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, int iDefault, LPTSTR pszBuffer);

	void SetBool(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, bool bValue);
	bool GetBool(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, bool bDefault, LPTSTR pszBuffer);

	void SetInt64(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, __int64 llValue);
	__int64 GetInt64(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, __int64 llDefault, LPTSTR pszBuffer);

	// remove functions
	void RemoveSection(LPCTSTR pszConfig, LPCTSTR pszSection);
	void RemoveKey(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, bool bAllAfter);

	// Get functions
	const _PROFILE* GetProfile(PCTSTR pszConfig);
	const _SECTION* GetSection(PCTSTR pszConfig, PCTSTR pszSection);

	// config-selection funcs
	UINT GetProfilesCount() { return (UINT)m_vConfigProfiles.size(); };	// returns count of profiles
	void GetProfileName(UINT uiIndex, LPTSTR pszName);					// return profile name
	void SetProfileName(UINT uiIndex, LPCTSTR pszName);					// sets the profile name
	void DeleteProfile(UINT uiIndex);									// deletes whole profile
	void CreateProfile(LPCTSTR pszName);								// creates new profile

	// def
	bool IsDefault() const { return m_bDefault; };

protected:
	// helpers
	void FreeData();						// frees data from m_vConfigProfiles
	void UnescapeString(PTSTR pszData);

protected:
	TCHAR* m_pszFilename;	// this configuration's file filename
	bool m_bModified;		// global modification flag
	bool m_bDefault;		// every GetXXX refreshed member - if returned value was the default one or
							// read from file

	vector<_PROFILE*> m_vConfigProfiles;	// contains configuration profiles's names (user names)
										// each config profile contains the same set of sections and keys
};

#endif