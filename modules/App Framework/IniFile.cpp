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

#define MAX_LINE	8192

///////////////////////////////////////////////////////////////
// Opens .ini file, reads all lines from it and keep in memory
// pszFilename [in] - path to .ini file
// pszOneSection [in] - ptr to a string with section's name
//	which we want to read
///////////////////////////////////////////////////////////////
void CIniFile::Open(LPCTSTR pszFilename, PCTSTR pszOneSection, bool bEscapeConversion)
{
	// remember file name
	m_pszFilename=new TCHAR[_tcslen(pszFilename)+1];
	_tcscpy(m_pszFilename, pszFilename);

	// count the length of a section member
	size_t tSectionLen=(size_t)-1;
	if (pszOneSection)
		tSectionLen=_tcslen(pszOneSection);

	// open file
	CFileEx file;
	file.Open(pszFilename, FA_READ | FA_BUFFERED);	// may throw an exception

	// read all lines from file and process it
	TCHAR szLine[MAX_LINE];
	size_t ulLen;
	
	// config object - zero members
    _PROFILE* pcfg=NULL;		// current config object
	_SECTION* psect=NULL;	// current section
	_ENTRY* pentry=NULL;	// current entry
	TCHAR* pszSeparator;	// used for separating keys from values

	while(file.ReadLine(szLine, MAX_LINE))
	{
		// process
		ulLen=_tcslen(szLine);		// length of read string

		if (ulLen == 0 || szLine[0] == '#')
			continue;

		// if it's config-selector <...>
		if (ulLen > 1 && szLine[0] == _T('<') && szLine[ulLen-1] == _T('>'))
		{
			// config-selector
			// create new profile
			pcfg=new _PROFILE;
			pcfg->pszProfileName=new TCHAR[ulLen-1];
			_tcsncpy(pcfg->pszProfileName, szLine+1, ulLen-2);
			pcfg->pszProfileName[ulLen-2]=_T('\0');
			m_vConfigProfiles.push_back(pcfg);
		}
		else if (ulLen > 1 && szLine[0] == _T('[') && szLine[ulLen-1] == _T(']'))
		{
			if (pszOneSection && psect != NULL)		// break if the needed section has been read already
				break;

			if (!pszOneSection || (ulLen-2 == tSectionLen && _tcsncmp(pszOneSection, szLine+1, ulLen-2) == 0))
			{
				// section-selector
				if (pcfg == NULL)
				{
					// encountered section without config-selector - create default one
					pcfg=new _PROFILE;
					pcfg->pszProfileName=new TCHAR[4];
					_tcscpy(pcfg->pszProfileName, _T("000"));
					m_vConfigProfiles.push_back(pcfg);
				}
				
				// new section
				psect=new _SECTION;
				psect->pszSectionName=new TCHAR[ulLen-1];
				_tcsncpy(psect->pszSectionName, szLine+1, ulLen-2);
				psect->pszSectionName[ulLen-2]=_T('\0');
				pcfg->vSections.push_back(psect);
			}
		}
		else
		{
			if (!pszOneSection || psect != NULL)
			{
				// normal line (not empty)
				if (pcfg == NULL)
				{
					// config-selector doesn't exist - create default
					pcfg=new _PROFILE;
					pcfg->pszProfileName=new TCHAR[4];
					_tcscpy(pcfg->pszProfileName, _T("000"));
					m_vConfigProfiles.push_back(pcfg);
				}
				
				if (psect == NULL)
				{
					// section doesn't exist -create default
					psect=new _SECTION;
					psect->pszSectionName=new TCHAR[4];
					_tcscpy(psect->pszSectionName, _T("000"));
					pcfg->vSections.push_back(psect);
				}
				
				// analyze string and copy data
				pentry=new _ENTRY;
				pszSeparator=_tcschr(szLine, _T('='));
				if (pszSeparator != NULL)
				{
					pszSeparator[0]=_T('\0');
					pszSeparator++;
					
					pentry->pszKey=new TCHAR[_tcslen(szLine)+1];
					_tcscpy(pentry->pszKey, szLine);
					
					// now the value - correct the ansi escape sequences if needed
					if (bEscapeConversion)
					{
//						int iLen=_tcslen(pszSeparator);
						UnescapeString(pszSeparator);
					}
					pentry->pszValue=new TCHAR[_tcslen(pszSeparator)+1];
					_tcscpy(pentry->pszValue, pszSeparator);
				}
				else
				{
					// no '=' sign in the line
					pentry->pszKey=NULL;
					if (bEscapeConversion)
						UnescapeString(szLine);
					pentry->pszValue=new TCHAR[_tcslen(szLine)+1];
					_tcscpy(pentry->pszValue, szLine);
				}

				psect->vEntries.push_back(pentry);
			}
		}
	}
    
	file.Close();
}

void CIniFile::UnescapeString(PTSTR pszData)
{
	PTSTR pszOut=pszData;
	while (*pszData != 0)
	{
		if (*pszData == _T('\\'))
		{
			pszData++;
			switch(*pszData++)
			{
			case _T('t'):
				*pszOut++=_T('\t');
				break;
			case _T('r'):
				*pszOut++=_T('\r');
				break;
			case _T('n'):
				*pszOut++=_T('\n');
				break;
			default:
				*pszOut++=_T('\\');
			}
		}
		else
			*pszOut++=*pszData++;
	}
	*pszOut=_T('\0');
}

///////////////////////////////////////////////////////////////
// Saves data from memory to an .ini file
///////////////////////////////////////////////////////////////
void CIniFile::Save()
{
	// if saving data is needed
	if (!m_bModified)
		return;

	_ASSERT(m_pszFilename);		// is there a filename

	// open file for writing
	CFileEx file;
	file.Open(m_pszFilename, FA_WRITE | FA_BUFFERED);	// may throw an exception

	// enumerate through all data and store it to the file
	TCHAR szLine[MAX_LINE];
	int iLen;
	vector<_PROFILE*>::const_iterator cit=m_vConfigProfiles.begin();
	vector<_SECTION*>::const_iterator sit;
	vector<_ENTRY*>::const_iterator eit;
	while (cit != m_vConfigProfiles.end())
	{
		// store profile name
		iLen=_stprintf(szLine, _T("<%s>"), (*cit)->pszProfileName);
		file.WriteLine(szLine);

		// enumerate through sections
		sit=(*cit)->vSections.begin();
		while (sit != (*cit)->vSections.end())
		{
			// write section name
			iLen=_stprintf(szLine, _T("[%s]"), (*sit)->pszSectionName);
			file.WriteLine(szLine);

			// enumerate through attributes
			eit=(*sit)->vEntries.begin();
			while(eit != (*sit)->vEntries.end())
			{
				// store data
				iLen=_stprintf(szLine, _T("%s=%s"), (*eit)->pszKey, (*eit)->pszValue);
				file.WriteLine(szLine);

				// analyze next element
				eit++;
			}

			// cosmetics
			file.WriteLine(_T(""));

			// analyze next section
			sit++;
		}

		// analyze next profile
		cit++;
	}

	// close file
	file.Close();

	m_bModified=false;
}

///////////////////////////////////////////////////////////////
// Closes the .ini file. Causes data from memory to be saved,
// and frees all the memory.
///////////////////////////////////////////////////////////////
void CIniFile::Close()
{
	// save file and free all data (doesn't matter if save succeeded)
	try
	{
		Save();
	}
	catch(...)
	{
		FreeData();
		throw;
	}
	FreeData();
}

///////////////////////////////////////////////////////////////
// Gets a string from .ini file (more precisely from the data
// from memory).
// pszConfig [in] - profile name
// pszSection [in] - section name
// pszKey [in] - key name
// pszValue [out] - value name (read from file or default)
// pszDefault [in/out] - default value if there wasn't pszKey
//				in .ini file.
// Ret Value [out] - if the pszValue was read from file (=true)
//			or is the default one (=false)
///////////////////////////////////////////////////////////////
bool CIniFile::GetStr(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, LPTSTR pszValue, LPCTSTR pszDefault)
{
	// localize config
	vector<_PROFILE*>::const_iterator cit=m_vConfigProfiles.begin();
	while (cit != m_vConfigProfiles.end())
	{
		// if this is not a config name - enumerate next
		if (_tcsicmp((*cit)->pszProfileName, pszConfig) != 0)
		{
			cit++;
			continue;
		}

		// config found - check for section
		vector<_SECTION*>::const_iterator sit=(*cit)->vSections.begin();
		while (sit != (*cit)->vSections.end())
		{
			// continue if this is not the needed section
			if (_tcsicmp((*sit)->pszSectionName, pszSection) != 0)
			{
				sit++;
				continue;
			}

			// now - localize key in section
			vector<_ENTRY*>::const_iterator eit=(*sit)->vEntries.begin();
			while (eit != (*sit)->vEntries.end())
			{
				// continue if needed
				if (_tcsicmp((*eit)->pszKey, pszKey) != 0)
				{
					eit++;
					continue;
				}

				// read associated value - from file
				_tcscpy(pszValue, (*eit)->pszValue);
				m_bDefault=false;
				return true;
			}

			// section was found, but key in it wasn't
			_tcscpy(pszValue, pszDefault);
			m_bDefault=true;
			return false;
		}

		// section not found
		_tcscpy(pszValue, pszDefault);
		m_bDefault=true;
		return false;
	}

	// nothing
	_tcscpy(pszValue, pszDefault);
	m_bDefault=true;
	return false;
}

///////////////////////////////////////////////////////////////
// Reads string from .ini file
// Look @ CIniFile::GetStr for param desc
// Ret Value [out] - string that was read (or the default one)
///////////////////////////////////////////////////////////////
LPTSTR CIniFile::GetString(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, LPTSTR pszValue, LPCTSTR pszDefault)
{
	GetStr(pszConfig, pszSection, pszKey, pszValue, pszDefault);
	return pszValue;
}

///////////////////////////////////////////////////////////////
// Writes string to .ini file (to memory)
// For param desc's look @GetStr
// pszValue [in] - string to store under pszKey
///////////////////////////////////////////////////////////////
void CIniFile::SetString(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszValue)
{
	// localize config
	vector<_PROFILE*>::const_iterator cit=m_vConfigProfiles.begin();
	_PROFILE* pcfg=NULL;
	while (cit != m_vConfigProfiles.end())
	{
		// if this is not a config name - enumerate next
		if (_tcsicmp((*cit)->pszProfileName, pszConfig) != 0)
		{
			cit++;
			continue;
		}

		// found
		pcfg=*cit;
		break;
	}

	// if section doesn't exist - create it
	if (pcfg == NULL)
	{
		pcfg=new _PROFILE;
		pcfg->pszProfileName=new TCHAR[_tcslen(pszConfig)+1];
		_tcscpy(pcfg->pszProfileName, pszConfig);
		m_vConfigProfiles.push_back(pcfg);
	}

	// config is ready - now search for section
	vector<_SECTION*>::const_iterator sit=pcfg->vSections.begin();
	_SECTION* psect=NULL;
	while (sit != pcfg->vSections.end())
	{
		// continue if this is not the needed section
		if (_tcsicmp((*sit)->pszSectionName, pszSection) != 0)
		{
			sit++;
			continue;
		}

		// found
        psect=*sit;
		break;
	}

	// create section if doesn't exist
	if (psect == NULL)
	{
		psect=new _SECTION;
		psect->pszSectionName=new TCHAR[_tcslen(pszSection)+1];
		_tcscpy(psect->pszSectionName, pszSection);
		pcfg->vSections.push_back(psect);
	}

	// now entry...
	vector<_ENTRY*>::const_iterator eit=psect->vEntries.begin();
	_ENTRY* pentry=NULL;
	while (eit != psect->vEntries.end())
	{
		// continue if needed
		if (_tcsicmp((*eit)->pszKey, pszKey) != 0)
		{
			eit++;
			continue;
		}

		// found
		pentry=*eit;
		break;
	}

	// create entry if needed
	if (pentry == NULL)
	{
		pentry=new _ENTRY;
		pentry->pszKey=new TCHAR[_tcslen(pszKey)+1];
		_tcscpy(pentry->pszKey, pszKey);
		pentry->pszValue=NULL;
		psect->vEntries.push_back(pentry);
	}

	// copy value to entry->pszValue
	if (pentry->pszValue != NULL)
		delete [] pentry->pszValue;

	pentry->pszValue=new TCHAR[_tcslen(pszValue)+1];
	_tcscpy(pentry->pszValue, pszValue);

	m_bModified=true;
}

///////////////////////////////////////////////////////////////
// Sets the int value in the .ini file (memory)
// pszConfig, pszSection, pszKey [in] - position in .ini file
// iValue [in] - value to set
///////////////////////////////////////////////////////////////
void CIniFile::SetInt(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, int iValue)
{
	TCHAR szBuffer[11];
	SetString(pszConfig, pszSection, pszKey, _itot(iValue, szBuffer, 10));
}

///////////////////////////////////////////////////////////////
// Gets the int value from .ini file (memory)
// pszConfig, pszSection, pszKey [in] - position in .ini file
// iDefault [in] - default value - used if the file doesn't
//				contain neede value
// pszBuffer [in] - buffer for internal processing (must
//		contain at last 5 TCHARs).
// Ret Value [out] - value associated with the given position
///////////////////////////////////////////////////////////////
int CIniFile::GetInt(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, int iDefault, LPTSTR pszBuffer)
{
	// get string and process it
	if (GetStr(pszConfig, pszSection, pszKey, pszBuffer, _T("")))
		return _ttoi(pszBuffer);
	else
		return iDefault;
}

///////////////////////////////////////////////////////////////
// Sets the bool value in the .ini file (memory)
// pszConfig, pszSection, pszKey [in] - position in .ini file
// bValue [in] - value to set
///////////////////////////////////////////////////////////////
void CIniFile::SetBool(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, bool bValue)
{
	TCHAR szBuffer[2]={ (TCHAR)(48+(bValue ? 1 : 0)), 0 };
	SetString(pszConfig, pszSection, pszKey, szBuffer);
}

///////////////////////////////////////////////////////////////
// Gets the bool value from .ini file (memory)
// pszConfig, pszSection, pszKey [in] - position in .ini file
// bDefault [in] - default value - used if the file doesn't
//				contain needed value
// pszBuffer [in] - buffer for internal processing (must
//		contain at last 2 TCHARs-better/safer is 1024).
// Ret Value [out] - value associated with the given position
///////////////////////////////////////////////////////////////
bool CIniFile::GetBool(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, bool bDefault, LPTSTR pszBuffer)
{
	// get string and process it
	if (GetStr(pszConfig, pszSection, pszKey, pszBuffer, _T("")))
		return pszBuffer[0] != _T('0');
	else
		return bDefault;
}

///////////////////////////////////////////////////////////////
// Sets the int64 value in the .ini file (memory)
// pszConfig, pszSection, pszKey [in] - position in .ini file
// llValue [in] - value to set
///////////////////////////////////////////////////////////////
void CIniFile::SetInt64(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, __int64 llValue)
{
	TCHAR szBuffer[21];
	SetString(pszConfig, pszSection, pszKey, _i64tot(llValue, szBuffer, 10));
}

///////////////////////////////////////////////////////////////
// Gets the int64 value from .ini file (memory)
// pszConfig, pszSection, pszKey [in] - position in .ini file
// llDefault [in] - default value - used if the file doesn't
//				contain needed value
// pszBuffer [in] - buffer for internal processing (must
//		contain at last 9 TCHARs-better/safer is 1024).
// Ret Value [out] - value associated with the given position
///////////////////////////////////////////////////////////////
__int64 CIniFile::GetInt64(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, __int64 llDefault, LPTSTR pszBuffer)
{
	// get string and process it
	if (GetStr(pszConfig, pszSection, pszKey, pszBuffer, _T("")))
		return _ttoi64(pszBuffer);
	else
		return llDefault;
}

///////////////////////////////////////////////////////////////
// Gets the profile name at a given index in a file
// uiIndex [in] - index of profile to retrieve
// pszName [out] - name of the profile
///////////////////////////////////////////////////////////////
void CIniFile::GetProfileName(UINT uiIndex, LPTSTR pszName)
{
	_tcscpy(pszName, m_vConfigProfiles.at(uiIndex)->pszProfileName);
}

///////////////////////////////////////////////////////////////
// Sets profile name at a given index
// uiIndex [in] - profile index
// pszName [in] - new profile name
///////////////////////////////////////////////////////////////
void CIniFile::SetProfileName(UINT uiIndex, LPCTSTR pszName)
{
	delete [] m_vConfigProfiles.at(uiIndex)->pszProfileName;
	m_vConfigProfiles.at(uiIndex)->pszProfileName=new TCHAR[_tcslen(pszName)+1];
	_tcscpy(m_vConfigProfiles.at(uiIndex)->pszProfileName, pszName);
}

///////////////////////////////////////////////////////////////
// Deletes one of the configurations (completely)
// uiIndex [in] - profile index to delete
///////////////////////////////////////////////////////////////
void CIniFile::DeleteProfile(UINT uiIndex)
{
	m_vConfigProfiles.erase(m_vConfigProfiles.begin()+uiIndex);
}

///////////////////////////////////////////////////////////////
// Creates new profile in .ini file
// pszName [in] - new profile's name
///////////////////////////////////////////////////////////////
void CIniFile::CreateProfile(LPCTSTR pszName)
{
	_PROFILE* pcfg=new _PROFILE;
	pcfg->pszProfileName=new _TCHAR[_tcslen(pszName)+1];
	_tcscpy(pcfg->pszProfileName, pszName);
}

///////////////////////////////////////////////////////////////
// (Internal) Frees all data associated with this class
///////////////////////////////////////////////////////////////
void CIniFile::FreeData()
{
	// free name string
	delete [] m_pszFilename;
	m_pszFilename=NULL;

	// other data
    _PROFILE* pcfg=NULL;		// current config object
	_SECTION* psect=NULL;	// current section
	_ENTRY* pentry=NULL;	// current entry
	size_t uiIndex1=m_vConfigProfiles.size(), uiIndex2, uiIndex3;
	while (uiIndex1--)
	{
		// delete profile name
		pcfg=m_vConfigProfiles.at(uiIndex1);
		delete [] pcfg->pszProfileName;
		
		// delete all sections
		uiIndex2=pcfg->vSections.size();
		while (uiIndex2--)
		{
			// delete section name
			psect=pcfg->vSections.at(uiIndex2);
			delete [] psect->pszSectionName;
			
			// free all key=value strings
			uiIndex3=psect->vEntries.size();
			while (uiIndex3--)
			{
				pentry=psect->vEntries.at(uiIndex3);
				
				// delete all values and this entry
				delete [] pentry->pszKey;
				delete [] pentry->pszValue;
				delete pentry;
			}

			// free this section
			delete psect;
		}

		// delete this profile
		delete pcfg;
	}

	m_vConfigProfiles.clear();
	m_bModified=false;
	m_bDefault=false;
}

void CIniFile::RemoveSection(LPCTSTR pszConfig, LPCTSTR pszSection)
{
	// localize section
	_PROFILE *pcfg=NULL;
	_SECTION *psect=NULL;

	vector<_PROFILE*>::iterator it=m_vConfigProfiles.begin();
	while (it != m_vConfigProfiles.end())
	{
		pcfg=(*it);
		if (_tcscmp(pcfg->pszProfileName, pszConfig) == 0)
			break;
		pcfg=NULL;
		it++;
	}

	if (pcfg == NULL)
		return;

	// find the section
	vector<_SECTION*>::iterator sit=pcfg->vSections.begin();
	while(sit != pcfg->vSections.end())
	{
		psect=(*sit);
		if (_tcscmp(psect->pszSectionName, pszSection) == 0)
			break;
		psect=NULL;
		sit++;
	}

	if (psect == NULL)
		return;

	// delete
	delete [] psect->pszSectionName;
	
	// free all key=value strings
	size_t tIndex=psect->vEntries.size();
	_ENTRY* pentry;
	while (tIndex--)
	{
		pentry=psect->vEntries.at(tIndex);
		
		// delete all values and this entry
		delete [] pentry->pszKey;
		delete [] pentry->pszValue;
		delete pentry;
	}
	
	// free this section
	delete psect;
	pcfg->vSections.erase(sit);
}

void CIniFile::RemoveKey(LPCTSTR pszConfig, LPCTSTR pszSection, LPCTSTR pszKey, bool bAllAfter)
{
	// localize section
	_PROFILE *pcfg=NULL;
	_SECTION *psect=NULL;

	vector<_PROFILE*>::iterator it=m_vConfigProfiles.begin();
	while (it != m_vConfigProfiles.end())
	{
		pcfg=(*it);
		if (_tcscmp(pcfg->pszProfileName, pszConfig) == 0)
			break;
		pcfg=NULL;
		it++;
	}

	if (pcfg == NULL)
		return;

	// find the section
	vector<_SECTION*>::iterator sit=pcfg->vSections.begin();
	while(sit != pcfg->vSections.end())
	{
		psect=(*sit);
		if (_tcscmp(psect->pszSectionName, pszSection) == 0)
			break;
		psect=NULL;
		sit++;
	}

	if (psect == NULL)
		return;

	// localize the key(s)
	_ENTRY* pentry=NULL;
	vector<_ENTRY*>::iterator eit=psect->vEntries.begin();
	while (eit != psect->vEntries.end())
	{
		pentry=(*eit);
		if (_tcscmp(pentry->pszKey, pszKey) == 0)
			break;
		pentry=NULL;
		eit++;
	}

	if (pentry == NULL)
		return;

	vector<_ENTRY*>::iterator eit2=eit;
	if (bAllAfter)
	{
		while(eit != psect->vEntries.end())
		{
			pentry=(*eit);
			delete [] pentry->pszKey;
			delete [] pentry->pszValue;
			delete pentry;
			eit++;
		}

		psect->vEntries.erase(eit2, psect->vEntries.end());
	}
	else
	{
		delete [] pentry->pszKey;
		delete [] pentry->pszValue;
		delete pentry;
		psect->vEntries.erase(eit);
	}
}

const _PROFILE* CIniFile::GetProfile(PCTSTR pszConfig)
{
	_PROFILE *pcfg=NULL;

	vector<_PROFILE*>::iterator it=m_vConfigProfiles.begin();
	while (it != m_vConfigProfiles.end())
	{
		pcfg=(*it);
		if (_tcscmp(pcfg->pszProfileName, pszConfig) == 0)
			break;
		pcfg=NULL;
		it++;
	}

	if (pcfg == NULL)
		return NULL;
	else
		return pcfg;
}

const _SECTION* CIniFile::GetSection(PCTSTR pszConfig, PCTSTR pszSection)
{
	// localize section
	_PROFILE *pcfg=NULL;
	_SECTION *psect=NULL;

	vector<_PROFILE*>::iterator it=m_vConfigProfiles.begin();
	while (it != m_vConfigProfiles.end())
	{
		pcfg=(*it);
		if (_tcscmp(pcfg->pszProfileName, pszConfig) == 0)
			break;
		pcfg=NULL;
		it++;
	}

	if (pcfg == NULL)
		return NULL;

	// find the section
	vector<_SECTION*>::iterator sit=pcfg->vSections.begin();
	while(sit != pcfg->vSections.end())
	{
		psect=(*sit);
		if (_tcscmp(psect->pszSectionName, pszSection) == 0)
			break;
		psect=NULL;
		sit++;
	}

	if (psect == NULL)
		return NULL;
	else
		return psect;
}
