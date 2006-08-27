/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   copyhandler@o2.pl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
/** \file cfg.cpp
 *  \brief A placeholder for config class definitions.
 *  \todo Modify the class to use file class as a file access layer.
 */
 
#include "cfg.h"
#include <assert.h>
#include "str.h"
#include <stdio.h>
#include "str_help.h"
#ifdef USE_ENCRYPTION
	#include "crypt.h"
#endif
#include <vector>

/// Specifies maximum line length of the .conf file
#define MAX_LINE 1024

#ifdef _WIN32
	/// A small helper for win32 systems
	#define snprintf _snprintf
#endif

BEGIN_ICPF_NAMESPACE

/// A global instance of a config class
config *__g_cfg=NULL;

// to make access faster
#define m_pvProperties ((std::vector<ulong_t>*)m_pProperties)

//////////////////////////////////////////////////////////////////////////////////
// prop_group class

/** Standard constructor
 */
prop_group::prop_group(ulong_t ulID) :
	m_pProperties(NULL),
	m_ulGroupID(ulID)
{
	m_pProperties=(void*)new std::vector<ulong_t>;
}

/** Standard destructor
 */
prop_group::~prop_group()
{
	try
	{
		delete m_pvProperties;
	}
	catch(...)
	{
	}
}

/** Function adds a new property id to the group.
 * \param[in] ulProp - id of a property to add
 */
void prop_group::add(ulong_t ulProp)
{
	m_pvProperties->push_back(ulProp);
}

/** Function searches for a specific property id inside the list.
 * \return True if the property has been found, false if not.
 */
bool prop_group::is_set(ulong_t ulProp)
{
	for (std::vector<ulong_t>::iterator it=m_pvProperties->begin();it != m_pvProperties->end();it++)
	{
		if ((*it) == ulProp)
			return true;
	}

	return false;
}

/** Function returns a count of properties contained in the list.
 * \return A count of properties.
 */
ulong_t prop_group::count() const
{
	return (ulong_t)m_pvProperties->size();
}

/** Function returns a property ID at a specified index.
 * \param[in] ulIndex - an index
 * \return A property id.
 */
ulong_t prop_group::get_at(ulong_t ulIndex)
{
	return m_pvProperties->at(ulIndex);
}

/** Function returns the group id.
 * \return A group ID.
 */
ulong_t prop_group::get_groupid() const
{
	return m_ulGroupID;
}

/////////////////////////////////////////////////////////////////////////////////////
// config class

#define m_pvProps ((std::vector<_PROP>*)m_pProps)
#define m_pvUnreg ((std::vector<_PROP>*)m_pUnreg)

/** Retrieves a pointer to a global instance of a config class
 * \return Pointer to the config class
 */
config* get_config()
{
	return __g_cfg;
}

/** Constructs a config object.
 * \param[in] bGlobal - specifies if this class should be globally accessible by the get_config() friend
 *				function.
 */
config::config(bool bGlobal) :
	m_lock(),
	m_pProps(NULL),
	m_pUnreg(NULL),
	m_bModified(false),
#ifdef USE_ENCRYPTION
	m_strPassword(),
#endif
	m_clbPropertyChanged()
{
	m_pProps=(void*)new std::vector<_PROP>;
	m_pUnreg=(void*)new std::vector<_PROP>;

	if (bGlobal)
		__g_cfg=this;
}

/** Destructs the config class.
 */
config::~config()
{
	try
	{
		delete m_pvProps;
		delete m_pvUnreg;
	}
	catch(...)
	{
	}
}

/** Function opens the specified file, reads all the lines sequentially
 *  and stores discovered property values in the internal structures.
 * \param[in] pszFile - path to a .conf file (could have any extension)
 * \return -1 on error or 0 if operation finished succesfully
 * \note The properties does not have to be registered prior to use this function.
 */
int_t config::read(const char_t* pszFile)
{
	// NOTE: this function should reset the m_bModified flag
	// open the file
	FILE* pFile;
	if ( (pFile=fopen(pszFile, "r")) == NULL)
		return -1;

	char_t szLine[MAX_LINE], *pszData;

	// lock the instance - has to be done here to make sure all settings are
	// consistent if reloaded on-the-fly
	m_lock.lock();
	while(fgets(szLine, MAX_LINE, pFile))
	{
		szLine[MAX_LINE-1]='\0';		// needed when the line is too int_t
		pszData=szLine;

		// interpret the line
		// skip the whitespaces at the beginning
		while(is_whitespace(*pszData))
			pszData++;

		// skip the comments and empty lines
		if (pszData[0] == '#' || pszData[0] == ';' || pszData[0] == 0)
			continue;

		// split the line to the part on the left of '=' and to the right
		char_t* pVal=strchr(pszData, '=');
		if (pVal != NULL)
		{
			pVal[0]='\0';
			pVal++;

			// so we have name in the pszData and value pVal
			process_line(trim(pszData), trim(pVal));
		}
	}

	m_bModified=false;
	m_lock.unlock();

	m_clbPropertyChanged.exec((ulong_t)-1, NULL);

	fclose(pFile);
	
	return 0;
}

/** Writes all the registered properties into the given file. To avoid
 *  deleting any comments or unused properties from a file - the function
 *  read all the file into memory (!) and modifies only the lines that
 *  contains the requested property key (or adds a new entry at the end).
 *  After that the file is being overwritten.
 * \param[in] pszFile - path to a .conf file to which the properties shoud be written
 * \return -1 on error, 0 on success.
 */
int_t config::write(const char_t* pszFile)
{
	// if the config was not modified - why bother writing
	if (!m_bModified)
		return 0;

	// at first - read the current file (whole) - line by line
	std::vector<string> vLines;
	FILE* pFile=fopen(pszFile, "r");
	if (pFile != NULL)
	{
		char_t szLine[MAX_LINE];
		while(fgets(szLine, MAX_LINE, pFile))
		{
			vLines.push_back((string)trim(szLine));
		}

		fclose(pFile);
	}

	// apply the registered properties to the lines
	// NOTE: properties in the unreg cannot have changed, so we don't need to bother with them
	m_lock.lock();

#ifdef USE_ENCRYPTION
	// encrypt everything if needed
	try
	{
		for (std::vector<_PROP>::iterator it = m_pvProps->begin();it != m_pvProps->end();it++)
		{
			encrypt_property(&(*it));
		}
	}
	catch(...)
	{
		m_lock.unlock();
		throw;
	}
#endif

	bool bFnd=false;
	string str;
	for (std::vector<_PROP>::iterator it=m_pvProps->begin();it != m_pvProps->end();it++)
	{
		// process only if the property was modified
		// NOTE: if the file has been modified manually then they won't be overwritten
		if ((*it).bModified)
		{
			// check every line that has been read for property match
			bFnd=false;
			for (std::vector<string>::iterator sit=vLines.begin();sit != vLines.end();sit++)
			{
				// line matches ?
				if ((*sit).cmpn((*it).pszName, strlen((*it).pszName)) == 0)
				{
					// replace the line with a new value
					prepare_line(&(*it), &(*sit));
					bFnd=true;
					break;		// break the inner 'for'
				}
			}

			// if the line was not found - add the line at the end
			if (!bFnd)
			{
				prepare_line(&(*it), &str);
				vLines.push_back(str);
			}

			// mark as written, although it is a lie ;)
			(*it).bModified=false;
		}
	}
	m_bModified=false;
	m_lock.unlock();

	// save the new copy of file
	// NOTE: at this point the props are all marked as non-modified - if the write fails then all the data are lost
	if ( (pFile=fopen(pszFile, "w")) == NULL)
		return -1;
	
	for (std::vector<string>::iterator vit=vLines.begin();vit != vLines.end();vit++)
	{
		if (fprintf(pFile, STRFMT "\n", (const char_t*)(*vit)) < 0)
			return -1;
	}

	fclose(pFile);

	return 0;
}

/** Function returns a property type for a given property id.
 *  \note The function returns only the type, and not the rest of the property flags.
 * \param[in] ulProp - property id to get info about.
 * \return The property type.
 */
int_t config::get_proptype(ulong_t ulProp)
{
	m_lock.lock();

	int_t iRet=m_pvProps->at(ulProp).iType & PTM_TYPE;

	m_lock.unlock();
	return iRet;
}

/** Function registers property with int_t value, specified range and flags. Also
 *  checks if the property has already been registered if PF_CHECK flag has been specified (if it
 *  is, then the existing identifier is returned and nothing is changed).
 * \param[in] pszName - name of the property (key in key = value)
 * \param[in] iDef - default value for the property
 * \param[in] iLo - the lesser value of value's allowable range
 * \param[in] iHi - the higher value of value's allowable range
 * \param[in] iFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
ulong_t config::register_int(const char_t* pszName, int_t iDef, int_t iLo, int_t iHi, int_t iFlags)
{
	// check if there is already registered value with the given name
	m_lock.lock();
	ulong_t ulRes;
	if (iFlags & PF_CHECK && (ulRes=is_registered(pszName)) != (ulong_t)-1)
	{
		m_lock.unlock();
		return ulRes;
	}
	else
	{
		_PROP prop;
		
		// check if the property is in the unreg container
		if ( (ulRes = is_unreg(pszName)) == (ulong_t)-1 )
		{
			// property not found in the unreg - means that this is quite new stuff
			prop.bModified=true;
			prop.pszName=new char_t[strlen(pszName)+1];
			strcpy(prop.pszName, pszName);
			prop.val.i.iVal=iDef;		// will be overwritten when reading file
		}
		else
		{
			// get the entry
			prop=m_pvUnreg->at(ulRes);
			m_pvUnreg->erase(m_pvUnreg->begin()+ulRes);

			// set the value from a string
			int_t iVal=atol(prop.val.pszVal);
			delete [] prop.val.pszVal;
			prop.val.i.iVal=iVal;
		}
		
		// common part
		prop.iType=PT_INT | iFlags;
		prop.val.i.iLo=iLo;
		prop.val.i.iHi=iHi;
		
		// add to the list
		m_pvProps->push_back(prop);
		m_bModified=true;
		ulRes=(ulong_t)(m_pvProps->size()-1);
		m_lock.unlock();
		
		return ulRes;
	}
}

/** Function registers property with uint_t value, specified range and flags. Also
 *  checks if the property is registered if PF_CHECK flag has been specified (if it
 *  is, then the existing identifier is returned and nothing is changed.
 * \param[in] pszName - name of the property (key in key = value)
 * \param[in] uiDef - default value for the property
 * \param[in] uiLo - the lesser value of value's allowable range
 * \param[in] uiHi - the higher value of value's allowable range
 * \param[in] iFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
ulong_t config::register_uint(const char_t* pszName, uint_t uiDef, uint_t uiLo, uint_t uiHi, int_t iFlags)
{
	// check if there is already registered value with the given name
	m_lock.lock();
	ulong_t ulRes;
	if (iFlags & PF_CHECK && (ulRes=is_registered(pszName)) != (ulong_t)-1)
	{
		m_lock.unlock();
		return ulRes;
	}
	else
	{
		_PROP prop;
		
		if ( (ulRes = is_unreg(pszName)) == (ulong_t)-1 )
		{
			prop.pszName=new char_t[strlen(pszName)+1];
			strcpy(prop.pszName, pszName);
			prop.val.ui.uiVal=uiDef;		// will be overwritten when reading file
			prop.bModified=true;
		}
		else
		{
			// get the entry
			prop=m_pvUnreg->at(ulRes);
			m_pvUnreg->erase(m_pvUnreg->begin()+ulRes);
			
			uint_t uiVal=strtoul(prop.val.pszVal, NULL, 10);
			delete [] prop.val.pszVal;
			prop.val.ui.uiVal=uiVal;
		}

		// common part
		prop.iType=PT_UINT | iFlags;
		prop.val.ui.uiLo=uiLo;
		prop.val.ui.uiHi=uiHi;

		// add to the list
		m_pvProps->push_back(prop);
		m_bModified=true;
		ulRes=(ulong_t)(m_pvProps->size()-1);
		m_lock.unlock();

		return ulRes;
	}
}

/** Function registers property with longlong_t value, specified range and flags. Also
 *  checks if the property is registered if PF_CHECK flag has been specified (if it
 *  is, then the existing identifier is returned and nothing is changed.
 * \param[in] pszName - name of the property (key in key = value)
 * \param[in] llDef - default value for the property
 * \param[in] llLo - the lesser value of value's allowable range
 * \param[in] llHi - the higher value of value's allowable range
 * \param[in] iFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
ulong_t config::register_longlong(const char_t* pszName, longlong_t llDef, longlong_t llLo, longlong_t llHi, int_t iFlags)
{
	// check if there is already registered value with the given name
	m_lock.lock();
	ulong_t ulRes;
	if (iFlags & PF_CHECK && (ulRes=is_registered(pszName)) != (ulong_t)-1)
	{
		m_lock.unlock();
		return ulRes;
	}
	else
	{
		_PROP prop;
		
		if ( (ulRes = is_unreg(pszName)) == (ulong_t)-1 )
		{
			prop.pszName=new char_t[strlen(pszName)+1];
			strcpy(prop.pszName, pszName);
			prop.val.ll.llVal=llDef;		// will be overwritten when reading file
			prop.bModified=true;
		}
		else
		{
			// get the entry
			prop=m_pvUnreg->at(ulRes);
			m_pvUnreg->erase(m_pvUnreg->begin()+ulRes);

			ll_t llVal;
#ifdef _WIN32
			llVal=_atoi64(prop.val.pszVal);
#else
			llVal=atoll(prop.val.pszVal);
#endif
			delete [] prop.val.pszVal;
			prop.val.ll.llVal=llVal;
		}

		// common
		prop.iType=PT_LONGLONG | iFlags;
		prop.val.ll.llLo=llLo;
		prop.val.ll.llHi=llHi;

		// add to the list
		m_pvProps->push_back(prop);
		m_bModified=true;
		ulRes=(ulong_t)(m_pvProps->size()-1);
		m_lock.unlock();

		return ulRes;
	}
}

/** Function registers property with ulonglong_t value, specified range and flags. Also
 *  checks if the property is registered if PF_CHECK flag has been specified (if it
 *  is, then the existing identifier is returned and nothing is changed.
 * \param[in] pszName - name of the property (key in key = value)
 * \param[in] ullDef - default value for the property
 * \param[in] ullLo - the lesser value of value's allowable range
 * \param[in] ullHi - the higher value of value's allowable range
 * \param[in] iFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
ulong_t config::register_ulonglong(const char_t* pszName, ulonglong_t ullDef, ulonglong_t ullLo, ulonglong_t ullHi, int_t iFlags)
{
	// check if there is already registered value with the given name
	m_lock.lock();
	ulong_t ulRes;
	if (iFlags & PF_CHECK && (ulRes=is_registered(pszName)) != (ulong_t)-1)
	{
		m_lock.unlock();
		return ulRes;
	}
	else
	{
		_PROP prop;
		
		if ( (ulRes = is_unreg(pszName)) == (ulong_t)-1 )
		{
			prop.pszName=new char_t[strlen(pszName)+1];
			strcpy(prop.pszName, pszName);
			prop.val.ull.ullVal=ullDef;		// will be overwritten when reading file
			prop.bModified=true;
		}
		else
		{
			prop=m_pvUnreg->at(ulRes);
			m_pvUnreg->erase(m_pvUnreg->begin()+ulRes);

			ull_t ullVal;
#ifdef _WIN32
			ullVal=(ulonglong_t)_atoi64(prop.val.pszVal);
#else
			ullVal=(ulonglong_t)atoll(prop.val.pszVal);
#endif
			delete [] prop.val.pszVal;
			prop.val.ull.ullVal=ullVal;
		}

		// common
		prop.iType=PT_ULONGLONG | iFlags;
		prop.val.ull.ullLo=ullLo;
		prop.val.ull.ullHi=ullHi;

		// add to the list
		m_pvProps->push_back(prop);
		m_bModified=true;
		ulRes=(ulong_t)(m_pvProps->size()-1);
		m_lock.unlock();

		return ulRes;
	}
}

/** Function registers property with bool value and flags. Also
 *  checks if the property is registered if PF_CHECK flag has been specified (if it
 *  is, then the existing identifier is returned and nothing is changed.
 * \param[in] pszName - name of the property (key in key = value)
 * \param[in] bDef - default value for the property
 * \param[in] iFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
ulong_t config::register_bool(const char_t* pszName, bool bDef, int_t iFlags)
{
	// check if there is already registered value with the given name
	m_lock.lock();
	ulong_t ulRes;
	if (iFlags & PF_CHECK && (ulRes=is_registered(pszName)) != (ulong_t)-1)
	{
		m_lock.unlock();
		return ulRes;
	}
	else
	{
		_PROP prop;
		
		if ( (ulRes = is_unreg(pszName)) == (ulong_t)-1 )
		{
			prop.pszName=new char_t[strlen(pszName)+1];
			strcpy(prop.pszName, pszName);
			prop.val.bVal=bDef;		// current
			prop.bModified=true;
		}
		else
		{
			prop=m_pvUnreg->at(ulRes);
			m_pvUnreg->erase(m_pvUnreg->begin()+ulRes);
			
			bool bVal;
			if (strcmp(prop.val.pszVal, "yes") == 0)
				bVal=true;
			else if (strcmp(prop.val.pszVal, "no") == 0)
				bVal=false;
			else
				bVal=atoi(prop.val.pszVal) != 0;

			delete [] prop.val.pszVal;
			prop.val.bVal=bVal;
		}

		// common
		prop.iType=PT_BOOL | iFlags;

		// add to the list
		m_pvProps->push_back(prop);
		m_bModified=true;
		ulRes=(ulong_t)(m_pvProps->size()-1);
		m_lock.unlock();

		return ulRes;
	}
}

/** Function registers property with string value and flags. Also
 *  checks if the property is registered if PF_CHECK flag has been specified (if it
 *  is, then the existing identifier is returned and nothing is changed.
 *  If the property is marked as encrypted, then the default value is being treated as
 *  unencrypted (and the appropriate flag is being set).
 * \param[in] pszName - name of the property (key in key = value)
 * \param[in] pszDef - default value for the property
 * \param[in] iFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
ulong_t config::register_string(const char_t* pszName, const char_t* pszDef, int_t iFlags)
{
	// check if there is already registered value with the given name
	m_lock.lock();
	ulong_t ulRes;
	if (iFlags & PF_CHECK && (ulRes=is_registered(pszName)) != (ulong_t)-1)
	{
		m_lock.unlock();
		return ulRes;
	}
	else
	{
		_PROP prop;
		
		if ( (ulRes = is_unreg(pszName)) == (ulong_t)-1 )
		{
			prop.iType=PT_STRING | iFlags;
#ifdef USE_ENCRYPTION
			if (iFlags & PF_ENCRYPTED)
				prop.iType |= PF_DECODED;
#endif
			prop.pszName=new char_t[strlen(pszName)+1];
			strcpy(prop.pszName, pszName);
	
			prop.val.pszVal=new char_t[strlen(pszDef)+1];
			strcpy(prop.val.pszVal, pszDef);
			prop.bModified=true;
		}
		else
		{
			prop=m_pvUnreg->at(ulRes);
			m_pvUnreg->erase(m_pvUnreg->begin()+ulRes);
			prop.iType = PT_STRING | iFlags;
		}
		
		// add to the list
		m_pvProps->push_back(prop);
		m_bModified=true;
		ulRes=(ulong_t)(m_pvProps->size()-1);
		m_lock.unlock();

		return ulRes;
	}
}

/** Functions retrieves the int_t value associated with a given property ID. Can be called
 *  from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \return The property value.
 */
int_t config::get_int(ulong_t ulProp)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_INT);

	int_t iRet=m_pvProps->at(ulProp).val.i.iVal;
	m_lock.unlock();
	return iRet;
}

/** Functions retrieves the uint_t value associated with a given property ID. Can be called
 *  from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \return The property value.
 */
uint_t config::get_uint(ulong_t ulProp)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_UINT);

	uint_t ulRet=m_pvProps->at(ulProp).val.ui.uiVal;
	m_lock.unlock();
	return ulRet;
}

/** Functions retrieves the longlong_t value associated with a given property ID. Can be called
 *  from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \return The property value.
 */
longlong_t config::get_longlong(ulong_t ulProp)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_LONGLONG);

	longlong_t llRet=m_pvProps->at(ulProp).val.ll.llVal;
	m_lock.unlock();
	return llRet;
}

/** Functions retrieves the ulonglong_t value associated with a given property ID. Can be called
 *  from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \return The property value.
 */
ulonglong_t config::get_ulonglong(ulong_t ulProp)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_ULONGLONG);

	ulonglong_t ullRet=m_pvProps->at(ulProp).val.ull.ullVal;
	m_lock.unlock();
	return ullRet;
}

/** Functions retrieves the bool value associated with a given property ID. Can be called
 *  from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \return The property value.
 */
bool config::get_bool(ulong_t ulProp)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_BOOL);

	bool bRet=m_pvProps->at(ulProp).val.bVal;
	m_lock.unlock();
	return bRet;
}

/** Functions retrieves the string value associated with a given property ID. Can be called
 *  from any thread. The string contained in the internal structure is copied to the buffer
 *  specified by user. Max count of chars that can be copied is specified by the buffer length.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[out] psz - buffer for the string
 * \param[in] tMaxLen - length of the buffer
 */
void config::get_string(ulong_t ulProp, char_t* psz, size_t tMaxLen)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_STRING);

	_PROP& prop=m_pvProps->at(ulProp);

#ifdef USE_ENCRYPTION
	// if the property is encrypted and not decoded yet - decode it
	try
	{
		decrypt_property(&prop);
	}
	catch(...)
	{
		m_lock.unlock();
		throw;
	}
#endif

	char_t* pszSrc=prop.val.pszVal;
	strncpy(psz, pszSrc, tMaxLen);
	psz[tMaxLen-1]='\0';

	m_lock.unlock();
}

/** Functions retrieves the int_t value associated with a given property ID. Can be called
 *  from any thread. Function returns a pointer to a string contained in the internal structures
 *  so it is definitely faster than the other get_string function, but is much nore dangerous.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \return The pointer to a string contained in the internal structure.
 */
char_t* config::get_string(ulong_t ulProp)
{
	m_lock.lock();

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_STRING);

	_PROP& prop=m_pvProps->at(ulProp);

#ifdef USE_ENCRYPTION
	// if the property is encrypted and not decoded yet - decode it
	try
	{
		decrypt_property(&prop);
	}
	catch(...)
	{
		m_lock.unlock();
		throw;
	}
#endif

	char_t* psz=prop.val.pszVal;
	char_t* pszNew=new char_t[strlen(psz)+1];
	strcpy(pszNew, psz);

	m_lock.unlock();
	
	return pszNew;
}

/** Function sets the int_t value for a property with specified ID. Can be
 *  called from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[in] lVal - the new value of property to set
 */
void config::set_int(ulong_t ulProp, int_t iVal, prop_group* pGroup)
{
	m_lock.lock();

	// get the data
	_PROP& prop=m_pvProps->at(ulProp);

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_INT);

	int_t iOldVal=prop.val.i.iVal;

	// check the range
	if (iVal < prop.val.i.iLo)
		prop.val.i.iVal=prop.val.i.iLo;
	else if (iVal > prop.val.i.iHi)
		prop.val.i.iVal=prop.val.i.iHi;
	else
		prop.val.i.iVal=iVal;

	bool bMod=(prop.val.i.iVal != iOldVal);
	if (bMod)
	{
		prop.bModified=true;
		m_bModified=true;
	}
	m_lock.unlock();

	if (bMod)
	{
		if (pGroup)
			pGroup->add(ulProp);
		else
		{
			prop_group* pg=begin_group((ulong_t)-1);
			pg->add(ulProp);
			end_group(pg);
		}
	}
}

/** Function sets the uint_t value for a property with specified ID. Can be
 *  called from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[in] uiVal - the new value of property to set
 */
void config::set_uint(ulong_t ulProp, uint_t uiVal, prop_group* pGroup)
{
	m_lock.lock();

	// get the data
	_PROP& prop=m_pvProps->at(ulProp);

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_UINT);

	uint_t uiOldVal=prop.val.ui.uiVal;

	// check the range
	if (uiVal < prop.val.ui.uiLo)
		prop.val.ui.uiVal=prop.val.ui.uiLo;
	else if (uiVal > prop.val.ui.uiHi)
		prop.val.ui.uiVal=prop.val.ui.uiHi;
	else
		prop.val.ui.uiVal=uiVal;
	
	bool bMod=(prop.val.ui.uiVal != uiOldVal);

	if (bMod)
	{
		prop.bModified=true;
		m_bModified=true;
	}
	m_lock.unlock();

	if (bMod)
	{
		if (pGroup)
			pGroup->add(ulProp);
		else
		{
			prop_group* pg=begin_group((ulong_t)-1);
			pg->add(ulProp);
			end_group(pg);
		}
	}
}

/** Function sets the longlong_t value for a property with specified ID. Can be
 *  called from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[in] llVal - the new value of property to set
 */
void config::set_longlong(ulong_t ulProp, longlong_t llVal, prop_group* pGroup)
{
	m_lock.lock();

	// get the data
	_PROP& prop=m_pvProps->at(ulProp);

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_LONGLONG);

	ll_t llOldVal=prop.val.ll.llVal;

	// check the range
	if (llVal < prop.val.ll.llLo)
		prop.val.ll.llVal=prop.val.ll.llLo;
	else if (llVal > prop.val.ll.llHi)
		prop.val.ll.llVal=prop.val.ll.llHi;
	else
		prop.val.ll.llVal=llVal;
	
	bool bMod=(prop.val.ll.llVal != llOldVal);

	if (bMod)
	{
		prop.bModified=true;
		m_bModified=true;
	}
	m_lock.unlock();

	if (bMod)
	{
		if (pGroup)
			pGroup->add(ulProp);
		else
		{
			prop_group* pg=begin_group((ulong_t)-1);
			pg->add(ulProp);
			end_group(pg);
		}
	}
}

/** Function sets the ulonglong_t value for a property with specified ID. Can be
 *  called from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[in] ullVal - the new value of property to set
 */
void config::set_ulonglong(ulong_t ulProp, ulonglong_t ullVal, prop_group* pGroup)
{
	m_lock.lock();

	// get the data
	_PROP& prop=m_pvProps->at(ulProp);

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_ULONGLONG);

	ull_t ullOldVal=prop.val.ull.ullVal;

	// check the range
	if (ullVal < prop.val.ull.ullLo)
		prop.val.ull.ullVal=prop.val.ull.ullLo;
	else if (ullVal > prop.val.ull.ullHi)
		prop.val.ull.ullVal=prop.val.ull.ullHi;
	else
		prop.val.ull.ullVal=ullVal;
	
	bool bMod=(prop.val.ull.ullVal != ullOldVal);

	if (bMod)
	{
		prop.bModified=true;
		m_bModified=true;
	}
	m_lock.unlock();

	if (bMod)
	{
		if (pGroup)
			pGroup->add(ulProp);
		else
		{
			prop_group* pg=begin_group((ulong_t)-1);
			pg->add(ulProp);
			end_group(pg);
		}
	}
}

/** Function sets the bool value for a property with specified ID. Can be
 *  called from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[in] bVal - the new value of property to set
 */
void config::set_bool(ulong_t ulProp, bool bVal, prop_group* pGroup)
{
	m_lock.lock();

	// get the data
	_PROP& prop=m_pvProps->at(ulProp);
	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_BOOL);

	bool bMod=(prop.val.bVal != bVal);
	if (bMod)
	{
		prop.val.bVal=bVal;
		
		prop.bModified=true;
		m_bModified=true;
	}
	m_lock.unlock();

	if (bMod)
	{
		if (pGroup)
			pGroup->add(ulProp);
		else
		{
			prop_group* pg=begin_group((ulong_t)-1);
			pg->add(ulProp);
			end_group(pg);
		}
	}
}

/** Function sets the string value for a property with specified ID. Can be
 *  called from any thread.
 * \param[in] ulProp - property identifier returned by one of the register_* functions
 * \param[in] pszVal - the new value of property to set
 */
void config::set_string(ulong_t ulProp, const char_t* pszVal, prop_group* pGroup)
{
	m_lock.lock();

	_PROP& prop=m_pvProps->at(ulProp);

	assert((m_pvProps->at(ulProp).iType & PTM_TYPE) == PT_STRING);

	delete [] prop.val.pszVal;
	prop.val.pszVal=new char_t[strlen(pszVal)+1];
	strcpy(prop.val.pszVal, pszVal);
	
	prop.bModified=true;
	m_bModified=true;

#ifdef USE_ENCRYPTION
	// make sure the property is marked decoded
	if (prop.iType & PF_ENCRYPTED)
		prop.iType |= PF_DECODED;
#endif

	m_lock.unlock();

	if (pGroup)
		pGroup->add(ulProp);
	else
	{
		prop_group* pg=begin_group((ulong_t)-1);
		pg->add(ulProp);
		end_group(pg);
	}
}

#ifdef USE_ENCRYPTION

/** The function starts a new property group. Used to group notifications about the property changes (usually
 *  sent by using set_* functions. The notification informations are being passed to the callback object only
 *  after the end_group() was called.
 * \param[in] ulID - group id
 * \return A pointer to a new property group. Must be released using end_group().
 */
prop_group* config::begin_group(ulong_t ulID) const
{
	return new prop_group(ulID);
}

/** Ends a property group started with a begin_group(). Releases the allocated pointer and sends a group property
 *  change information to the callback.
 * \param[in] pGroup - pointer to the property group allocated with begin_group()
 */
void config::end_group(prop_group* pGroup)
{
	assert(pGroup);
	if (pGroup->count() > 0)
		m_clbPropertyChanged.exec(pGroup->get_groupid(), (ptr_t)pGroup);
	delete pGroup;
}

/** This function sets a password to be used with the encrypted properties. If this is the first password to be used
 *  then it is being set only. But if there was another password set previously, then any property encoded with the
 *  previous password will be decrypted using an old password before setting a new one.
 * \param[in] pszPass - a new password to be set.
 */
void config::set_password(const char_t* pszPass)
{
	m_lock.lock();

	if (!m_strPassword.is_empty())
	{
		// decrypt everything (if not already) using old password (if exists)
		try
		{
			for (std::vector<_PROP>::iterator it=m_pvProps->begin();it != m_pvProps->end();it++)
			{
				decrypt_property(&(*it));
			}
		}
		catch(...)
		{
			m_lock.unlock();
			throw;
		}
	}

	// set a new pass
	char_t szPass[64+1];
	str2key256(pszPass, szPass);

	m_strPassword=(const char_t*)szPass;

	m_lock.unlock();
}

/** Internal function that encrypts a one specified property structure. Does make a check regarding
 *  the correctness of the property. If it does not met the criteria, then function does nothing.
 * \param[in/out] prop - address of the structure describing the property.
 */
void config::encrypt_property(_PROP* prop) const
{
	printf("Encrypting...\n");

	if ((prop->iType & (PT_STRING | PF_ENCRYPTED | PF_DECODED)) == (PT_STRING | PF_ENCRYPTED | PF_DECODED))
	{
		printf("Real encrypt...\n");
		ulong_t ulLen=(ulong_t)(((strlen(prop->val.pszVal)+1)*sizeof(char_t)+16)*2);
		char_t *pszOut=new char_t[ulLen];
		try
		{
			strcrypt_aes256(prop->val.pszVal, (const char_t*)m_strPassword, pszOut);
		}
		catch(...)
		{
			delete [] pszOut;
			throw;
		}

		delete [] prop->val.pszVal;
		prop->val.pszVal=pszOut;

		prop->iType &= ~PF_DECODED;
	}
}

/** Internal function that decrypts a one specified property structure. Does make a check regarding
 *  the correctness of the property. If it does not met the criteria, then function does nothing.
 * \param[in/out] prop - address of the structure describing the property.
 */
void config::decrypt_property(_PROP* prop) const
{
	if ((prop->iType & (PT_STRING | PF_ENCRYPTED | PF_DECODED)) == (PT_STRING | PF_ENCRYPTED))
	{
		ulong_t ulLen=(ulong_t)(strlen(prop->val.pszVal)/2);
		char_t *pszOut=new char_t[ulLen];
		try
		{
			strdecrypt_aes256(prop->val.pszVal, (const char_t*)m_strPassword, pszOut);
		}
		catch(...)
		{
			delete [] pszOut;
			throw;
		}

		delete [] prop->val.pszVal;
		prop->val.pszVal=pszOut;

		prop->iType |= PF_DECODED;
	}
}

#endif

/** Function trims whitespaces at the beginning and at the end of a string. 
 *  The data in the string is modified, so any whitespace char_t at the end of a string
 *  is replaced with '\\0', and the function returns a pointer to the first character
 *  in the string that is not a whitespace.
 * \param[in,out] pszString - string to process
 * \return Pointer to the first non-whitespace character in a string.
 */
char_t* config::trim(char_t* pszString) const
{
	char_t *pszData=pszString;

	// skip the whitespaces at the beginning
	while(is_whitespace(*pszData))
		pszData++;

	// the end
	size_t iLen=strlen(pszData);
	if (iLen != 0)
	{
		iLen--;
		while (iLen > 0 && is_whitespace(*(pszData+iLen)))
			pszData[iLen--]='\0';
	}
	return pszData;
}

/** Processes the two strings given (a key = value) - checks if there is any
 *  registered property with a given name, and if there is - the given value
 *  is applied to the property in the format specified by that property.
 * \param[in] pszName - name of the property to find
 * \param[in] pszValue - value of the property; the string is processed using 
 *                       the property type found
 */
void config::process_line(const char_t* pszName, const char_t* pszValue)
{
	// check if the property name is registered
	for (std::vector<_PROP>::iterator it=m_pvProps->begin();it != m_pvProps->end();it++)
	{
		if (strcmp((*it).pszName, pszName) == 0)
		{
			// this function is called from read() which reads the standard stuff from a .conf file
			(*it).bModified=false;

			// this is it - type of property
			switch (it->iType & PTM_TYPE)
			{
			case PT_INT:
				{
					it->val.i.iVal=atol(pszValue);
					break;
				}
			case PT_UINT:
				{
					it->val.ui.uiVal=strtoul(pszValue, NULL, 10);
					break;
				}
			case PT_LONGLONG:
				{
#ifdef _WIN32
					it->val.ll.llVal=_atoi64(pszValue);
#else
					it->val.ll.llVal=atoll(pszValue);
#endif
					break;
				}
			case PT_ULONGLONG:
				{
#ifdef _WIN32
					it->val.ull.ullVal=(ulonglong_t)_atoi64(pszValue);
#else
					it->val.ull.ullVal=(ulonglong_t)atoll(pszValue);
#endif
					break;
				}
			case PT_BOOL:
				{
					if (strcmp(pszValue, "yes") == 0)
						it->val.bVal=true;
					else if (strcmp(pszValue, "no") == 0)
						it->val.bVal=false;
					else
						it->val.bVal=atoi(pszValue) != 0;
					break;
				}
			case PT_STRING:
				{
					// delete the old value and add the new one
					delete [] it->val.pszVal;
					it->val.pszVal=new char_t[strlen(pszValue)+1];
					strcpy(it->val.pszVal, pszValue);

					// update the encryption ststus
#ifdef USE_ENCRYPTION
					(*it).iType &= ~PF_DECODED;
#endif

					break;
				}
			default:
				break;
			}

			// we need not more processing
			return;
		}
	}
	
	// if the property wasn't found - add to the unreg as string prop
	_PROP prop;
	prop.iType=PT_STRING;
	prop.pszName=new char_t[strlen(pszName)+1];
	strcpy(prop.pszName, pszName);
	prop.bModified=false;
	prop.val.pszVal=new char_t[strlen(pszValue)+1];
	strcpy(prop.val.pszVal, pszValue);
	
	m_pvUnreg->push_back(prop);
}

/** Prepares the string with the property value to be written to a file.
 *  There is no sane limitation of the string length (but one should be careful
 *  because such a limitation is integrated with read-related functions.
 * \param[in] prop - pointer to the internal structure with property description
 * \param[out] pres - pointer to a string object that will receive the line of text
 */
void config::prepare_line(const _PROP* prop, string* pres) const
{
	assert(prop && pres);

	char_t szLine[MAX_LINE];
	switch(prop->iType & PTM_TYPE)
	{
	case PT_INT:
		{
			snprintf(szLine, MAX_LINE, STRFMT " = " LFMT, prop->pszName, prop->val.i.iVal);
			break;
		}
	case PT_UINT:
		{
			snprintf(szLine, MAX_LINE, STRFMT " = " ULFMT, prop->pszName, prop->val.ui.uiVal);
			break;
		}
	case PT_LONGLONG:
		{
			snprintf(szLine, MAX_LINE, STRFMT " = " LLFMT, prop->pszName, prop->val.ll.llVal);
			break;
		}
	case PT_ULONGLONG:
		{
			snprintf(szLine, MAX_LINE, STRFMT " = " ULLFMT, prop->pszName, prop->val.ull.ullVal);
			break;
		}
	case PT_BOOL:
		{
			snprintf(szLine, MAX_LINE, STRFMT " = " ULFMT, prop->pszName, (uint_t)prop->val.bVal);
			break;
		}
	case PT_STRING:
		{
			snprintf(szLine, MAX_LINE, STRFMT " = " STRFMT, prop->pszName, prop->val.pszVal);
			break;
		}
	default:
		break;
	}

	szLine[MAX_LINE-1]='\0';
//	printf(szLine);
	*pres=szLine;
}

/** Searches for a property with a given name and returns a property
 *  ID if found.
 * \param[in] pszName - property name to search for
 * \return The property ID if property has been found, or -1 if not.
 */
ulong_t config::is_registered(const char_t* pszName)
{
	// enum through all of the existing nodes
	for (std::vector<_PROP>::iterator it=m_pvProps->begin();it != m_pvProps->end();it++)
	{
		if (strcmp(pszName, (*it).pszName) == 0)
			return (ulong_t)(it-m_pvProps->begin());
	}

	return (ulong_t)-1;		// no property found
}

/** Searches for an unregistered property contained in the unreg container. Returns an
 *  index in the container unreg (if the entry have been found) or -1 (if not).
 * \param[in] pszName - name of the property to search for
 * \return Property index if found, -1 if not.
 */
ulong_t config::is_unreg(const char_t* pszName)
{
	// enum through all of the existing nodes
	for (std::vector<_PROP>::iterator it=m_pvUnreg->begin();it != m_pvUnreg->end();it++)
	{
		if (strcmp(pszName, (*it).pszName) == 0)
			return (ulong_t)(it-m_pvUnreg->begin());
	}

	return (ulong_t)-1;		// no property found
}

END_ICPF_NAMESPACE
