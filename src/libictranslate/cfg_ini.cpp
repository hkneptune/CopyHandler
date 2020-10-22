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
#include "cfg_ini.h"
#include <string>
#include <map>
#include <assert.h>
#include <stdexcept>

/// Buffer size for reading xml data from a file
#define INI_BUFFER	65536

/// String storage (key(s)=>value(s))
typedef std::multimap<std::wstring, std::wstring> attr_storage;
/// Ini node storage
typedef std::map<std::wstring, attr_storage> ini_storage;

/** Xml find handle structure - used for searching.
*/
struct INIFINDHANDLE
{
	attr_storage::iterator itAttr;			///< Iterator of currently retrieved string
	attr_storage::iterator itAttrEnd;		///< Iterator of a last string matching the criteria

	ini_storage::iterator itSection;	///< Section iterator
	ini_storage::iterator itSectionEnd;	///< End of section enumeration

	bool bOnlyAttributes;				///< Enumeration type - true = only attributes (does not use section iterators), false = sections + all attributes inside
	bool bSection;						///< Is section to be enumerated first ?
};

/// Macro for faster access to the xml storage
#define m_pMainNode ((ini_storage*)m_hMainNode)

/** Constructs the ini_cfg object.
*/
ini_cfg::ini_cfg() :
	m_hMainNode((void*)new ini_storage)
{

}

/** Destructs the xml config object.
*/
ini_cfg::~ini_cfg()
{
	delete m_pMainNode;
}

/** Function reads the contents of the xml file, parses itAttr using expat parser
*  and then creates xml nodes in memory that could be read using find functions.
*
* \param[in] pszPath - path to the file to be read
*/
void ini_cfg::read(const wchar_t* pszPath)
{
	// clear current contents
	clear();

	// read the data from file
#if defined(_UNICODE) && (defined(_WIN32) || defined(_WIN64))
	FILE* pFile=_tfopen(pszPath, _T("rb"));
#else
	FILE* pFile=_tfopen(pszPath, _T("rt"));
#endif

	if(pFile == nullptr)
		throw std::runtime_error("Cannot open file for reading");

	// prepare buffer for data
	wchar_t* pszBuffer = new wchar_t[INI_BUFFER];
	wchar_t* pszLine = nullptr;
	bool bFirstLine = true;

	while((pszLine = _fgetts(pszBuffer, INI_BUFFER, pFile)) != nullptr)
	{
		if(bFirstLine)
		{
			bFirstLine = false;
			// check BOM
			if(pszBuffer[0] != _T('\0') && *(unsigned short*)pszBuffer == 0xfeff)
				parse_line(pszBuffer + 1);
			else
				parse_line(pszBuffer);
		}
		else
			parse_line(pszBuffer);
	}

	delete [] pszBuffer;

	// check if that was eof or error
	if(feof(pFile) == 0)
	{
		fclose(pFile);
		// error while reading file
		throw std::runtime_error("Error while reading ini file");
	}

	// close the file
	fclose(pFile);
}

/// Processes the data from a given buffer
void ini_cfg::read_from_buffer(const wchar_t* pszBuffer, size_t stLen)
{
	// clear current contents
	clear();

	wchar_t* pszLine = new wchar_t[INI_BUFFER];
	size_t stLineLen = 0;
	const wchar_t* pszCurrent = pszBuffer;
	const wchar_t* pszLast = pszBuffer;
	bool bFirstLine = true;
	while(stLen--)
	{
		if(*pszCurrent == _T('\n'))
		{
			// there is a line [pszLast, pszCurrent)
			stLineLen = pszCurrent - pszLast;
			if(stLineLen)
			{
				if(stLineLen >= INI_BUFFER)
					stLineLen = INI_BUFFER - 1;
				_tcsncpy(pszLine, pszLast, stLineLen);
				pszLine[stLineLen] = _T('\0');

				if(bFirstLine)
				{
					bFirstLine = false;
					// check BOM
					if(pszLine[0] != _T('\0') && *(unsigned short*)pszLine == 0xfeff)
						parse_line(pszLine + 1);
					else
						parse_line(pszLine);
				}
				else
				{
					// process the line
					parse_line(pszLine);
				}
			}
			pszLast = pszCurrent + 1;
		}
		++pszCurrent;
	}
	if(pszCurrent != pszLast)
	{
		// there is a line [pszLast, pszCurrent)
		stLineLen = pszCurrent - pszLast;
		if(stLineLen)
		{
			if(stLineLen >= INI_BUFFER)
				stLineLen = INI_BUFFER - 1;

			_tcsncpy(pszLine, pszLast, stLineLen);
			pszLine[stLineLen] = _T('\0');

			// process the line
			parse_line(pszLine);
		}
	}

	delete [] pszLine;
}

/** Saves the internal xml nodes to the specified xml file.
*
* \param[in] pszPath - path to the file the data should be written to
*
* \note Function overwrites the contents of a file
*/
void ini_cfg::save(const wchar_t* pszPath)
{
	FILE* pFile=_tfopen(pszPath, _T("wb"));
	if(pFile == nullptr)
		throw std::runtime_error("Cannot open file for writing");

	// put BOM into the file

#if(defined(_WIN32) || defined(_WIN64))
	// utf-16le
	const unsigned int uiBOM=0x0000feff;
	const unsigned int uiCount=2;
#else
	// utf-8
	const unsigned int uiBOM=0x00bfbbef;
	const unsigned int uiCount=3;
#endif


	try
	{
		// write bom, check if itAttr succeeded
		if(fwrite(&uiBOM, 1, uiCount, pFile) != uiCount)
			throw std::runtime_error("Cannot write BOM to the file");

		// and write
		std::wstring strLine;
		for(ini_storage::iterator iterSections = m_pMainNode->begin(); iterSections != m_pMainNode->end(); ++iterSections)
		{
			strLine = _T("[") + (*iterSections).first + _T("]\r\n");
			if(_fputts(strLine.c_str(), pFile) == WEOF)
				throw std::runtime_error("Cannot put section name");

			for(attr_storage::iterator iterAttribute = (*iterSections).second.begin(); iterAttribute != (*iterSections).second.end(); ++iterAttribute)
			{
				strLine = (*iterAttribute).first + _T("=") + (*iterAttribute).second + L"\r\n";
				if(_fputts(strLine.c_str(), pFile) == WEOF)
					throw std::runtime_error("Cannot put attribute");
			}

			if(_fputts(L"\r\n", pFile) == WEOF)
				throw std::runtime_error("Cannot put end-of-line marker into the file");
		}
	}
	catch(...)
	{
		fclose(pFile);
		throw;
	}

	// close the file
	fclose(pFile);
}

/** Function starts a search operation. Given the name of the property
*  to be searched for(ie. "ch/program/startup"), funtion searches for
*  itAttr and returns a handle that can be used by subsequent calls to the
*  find_next(). Free the handle using find_close() after finish.
*
* \param[in] pszName - name of the property to search for(in the form of
*						"ch/program/startup"
* \return Handle to the search (nullptr if not found).
*/
void* ini_cfg::find(const wchar_t* pszName)
{
	if(pszName == nullptr || pszName[0] == _T('*'))
	{
		INIFINDHANDLE* pHandle = new INIFINDHANDLE;
		pHandle->bOnlyAttributes = false;
		pHandle->bSection = true;
		pHandle->itSection = m_pMainNode->begin();
		pHandle->itSectionEnd = m_pMainNode->end();

		return pHandle;
	}

	// parse the path
	std::wstring strSection;
	std::wstring strAttr;
	if(!parse_property_name(pszName, strSection, strAttr))
		return nullptr;

	ini_storage::iterator iterSection = m_pMainNode->find(strSection);
	if(iterSection == m_pMainNode->end())
		return nullptr;

	std::pair<attr_storage::iterator, attr_storage::iterator> pairRange;
	if(strAttr == _T("*"))
	{
		pairRange.first = (*iterSection).second.begin();
		pairRange.second = (*iterSection).second.end();
	}
	else
		pairRange = (*iterSection).second.equal_range(strAttr);
	if(pairRange.first != (*iterSection).second.end() && pairRange.first != pairRange.second)
	{
		INIFINDHANDLE* pHandle = new INIFINDHANDLE;
		pHandle->bSection = false;
		pHandle->bOnlyAttributes = true;
		pHandle->itAttr = pairRange.first;
		pHandle->itAttrEnd = pairRange.second;

		return pHandle;
	}

	return nullptr;
}

/** Finds the next string that belong to a specific key (as defined in
*  a call to find() function.
*
* \param[in] pFindHandle - handle to the search (as returned from find())
* \return Pointer to a next string found, nullptr if none.
*/
bool ini_cfg::find_next(void* pFindHandle, PROPINFO& pi)
{
	assert(pFindHandle);
	if(!pFindHandle)
		return false;
	INIFINDHANDLE* pfh=(INIFINDHANDLE*)pFindHandle;

	if(pfh->bOnlyAttributes)
	{
		if(pfh->itAttr != pfh->itAttrEnd)
		{
			pi.pszName = (*pfh->itAttr).first.c_str();
			pi.pszValue = (*pfh->itAttr).second.c_str();
			pi.bGroup = false;
			++pfh->itAttr;
			return true;
		}

		return false;
	}

	if(pfh->bSection)
	{
		if(pfh->itSection == pfh->itSectionEnd)
			return false;
		pfh->bSection = false;
		pfh->itAttr = (*pfh->itSection).second.begin();
		pfh->itAttrEnd = (*pfh->itSection).second.end();

		// section name
		pi.bGroup = true;
		pi.pszName = (*pfh->itSection++).first.c_str();
		pi.pszValue = nullptr;
		return true;
	}

	if(pfh->itAttr != pfh->itAttrEnd)
	{
		pi.bGroup = false;
		pi.pszName = (*pfh->itAttr).first.c_str();
		pi.pszValue = (*pfh->itAttr).second.c_str();

		++pfh->itAttr;
		if(pfh->itAttr == pfh->itAttrEnd)
			pfh->bSection = true;
		return true;

	}

	// should not happen
	assert(false);
	return false;
}

/** Closes the find handle.
*
* \param[in] pFindHandle - handle to the search (as returned from find())
*/
void ini_cfg::find_close(void* pFindHandle)
{
	delete ((INIFINDHANDLE*)pFindHandle);
}

/** Sets the specified value in the given key name. Value can be either added to
*  the current ones (multi-string support) or replace them completely.
*
* \param[in] pszName - key name for which the string should be set at
* \param[in] pszValue - value to set
* \param[in] a - action to take while setting
*/
void ini_cfg::set_value(const wchar_t* pszName, const wchar_t* pszValue, actions a)
{
	// parse the path
	std::wstring strSection;
	std::wstring strAttr;
	if(!parse_property_name(pszName, strSection, strAttr))
		throw std::runtime_error("Property not found");

	if(strAttr == _T("*"))
		throw std::runtime_error("Wildcards not available in set_value mode");

	// search
	ini_storage::iterator iterSection = m_pMainNode->find(strSection.c_str());
	if(iterSection == m_pMainNode->end())
	{
		std::pair<ini_storage::iterator, bool> pairSection = m_pMainNode->insert(ini_storage::value_type(strSection, attr_storage()));
		iterSection = pairSection.first;
		if(iterSection == m_pMainNode->end())
			throw std::runtime_error("Problem with creating section");
	}

	attr_storage& rAttrs = (*iterSection).second;

	// clear if we're replacing
	switch(a)
	{
	case config_base::action_replace:
		{
			std::pair<attr_storage::iterator, attr_storage::iterator> pairRange = (*iterSection).second.equal_range(strAttr);
			rAttrs.erase(pairRange.first, pairRange.second);
			// do not break here - we are about to insert the data
		}
	case config_base::action_add:
		{
			rAttrs.insert(attr_storage::value_type(strAttr, pszValue ? pszValue : std::wstring(_T(""))));
			break;
		}
	default:
		assert(false);
	}
}

void ini_cfg::clear()
{
	m_pMainNode->clear();
}

void ini_cfg::clear(const wchar_t* pszName)
{
	if(pszName == nullptr || pszName[0] == _T('*'))
		m_pMainNode->clear();
	else
	{
		std::wstring strSection;
		std::wstring strAttr;
		if(!parse_property_name(pszName, strSection, strAttr))
			throw std::runtime_error("Invalid name");

		ini_storage::iterator iterSection = m_pMainNode->find(strSection);
		if(iterSection != m_pMainNode->end())
		{
			attr_storage& rAttrs = (*iterSection).second;
			std::pair<attr_storage::iterator, attr_storage::iterator> pairRange;

			if(strAttr == _T("*"))
			{
				pairRange.first = rAttrs.begin();
				pairRange.second = rAttrs.end();
			}
			else
				pairRange = (*iterSection).second.equal_range(strAttr);
			rAttrs.erase(pairRange.first, pairRange.second);
		}
	}
}

void ini_cfg::parse_line(const wchar_t* pszLine)
{
	assert(pszLine);
	if(!pszLine)
		throw std::runtime_error("Invalid parameter");

	std::wstring strLine = pszLine;

	// trim whitespaces on the left
	while(strLine.begin() != strLine.end() && iswspace(*strLine.begin()))
	{
		strLine.erase(strLine.begin());
	}

	while(strLine.rbegin() != strLine.rend() && (*strLine.rbegin() == _T('\r') || *strLine.rbegin() == _T('\n')))
	{
		strLine.erase(strLine.end() - 1);
	}


	// detect line type
	if(strLine.begin() == strLine.end())			// empty line
		return;
	if(strLine[0] == _T('#') || strLine[0] == _T(';'))	// comment
		return;
	if(strLine[0] == _T('['))
	{
		// trim whitespaces and ']' on the right
		while(strLine.rbegin() != strLine.rend() && (iswspace(*strLine.rbegin()) || *strLine.rbegin() == _T(']')))
		{
			strLine.erase(strLine.end() - 1);
		}
		// trim [
		strLine.erase(strLine.begin());

		// a new section
		m_strCurrentSection = strLine;
		m_pMainNode->insert(ini_storage::value_type(strLine, attr_storage()));
	}
	else
	{
		// do not trim whitespaces on the right - the spaces may be meaningful
		// key=value
		std::wstring::size_type stPos = strLine.find_first_of(_T('='));
		if(stPos != std::wstring::npos)
		{
			ini_storage::iterator iterSection = m_pMainNode->find(m_strCurrentSection);
			if(iterSection == m_pMainNode->end())
				throw std::runtime_error("Internal processing error. Section should already be included.");

			std::wstring strLeft, strRight;
			strLeft.insert(strLeft.begin(), strLine.begin(), strLine.begin() + stPos);
			strRight.insert(strRight.begin(), strLine.begin() + stPos + 1, strLine.end());
			(*iterSection).second.insert(attr_storage::value_type(strLeft, strRight));
		}
	}
}

bool ini_cfg::parse_property_name(const wchar_t* pszName, std::wstring& rstrSection, std::wstring& rstrName)
{
	// parse the path
	std::wstring strPath = pszName;
	std::wstring::size_type stPos = strPath.find_first_of(_T('/'));
	if(stPos == std::wstring::npos)
		return false;
	std::wstring::size_type stPos2 = strPath.find_first_of(_T('/'), stPos + 1);
	if(stPos2 != std::wstring::npos && stPos2 != stPos)
		return false;											// paths with two or more '/' are not supported

	rstrName.clear();
	rstrName.clear();
	rstrSection.insert(rstrSection.begin(), strPath.begin(), strPath.begin() + stPos);
	rstrName.insert(rstrName.begin(), strPath.begin() + stPos + 1, strPath.end());

	return true;
}
