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
/** \file cfg.cpp
 *  \brief A placeholder for config class definitions.
 *  \todo Modify the class to use file class as a file access layer.
 */
#include "stdafx.h"
#include "cfg.h"
#include <assert.h>
#include <vector>
#include <set>
#include "cfg_ini.h"
#include <stdexcept>

//////////////////////////////////////////////////////////////////////////////////
// property_tracker class
#define m_psProperties ((std::set<unsigned int>*)m_hProperties)

/** Constructs the property_tracker object.
 */
property_tracker::property_tracker() :
	m_hProperties((void*)new std::set<unsigned int>)
{
}

/** Constructs the property_tracker by copying data from source object.
 *
 * \param[in] rSrc - source property tracker
 */
property_tracker::property_tracker(const property_tracker& rSrc) :
	m_hProperties((void*)new std::set<unsigned int>(*(std::set<unsigned int>*)rSrc.m_hProperties))
{
}

/** Destructs the property tracker object.
 */
property_tracker::~property_tracker()
{
	delete m_psProperties;
}

/** Function adds a new property id to the group.
 *
 * \param[in] uiProp - id of a property to add
 */
void property_tracker::add(unsigned int uiProp)
{
	m_psProperties->insert(uiProp);
}

/** Function searches for a specific property id inside the list.
 *
 * \param[in] uiProp - property id to check for
 * \return True if the property has been found, false if not.
 */
bool property_tracker::is_set(unsigned int uiProp)
{
	return m_psProperties->find(uiProp) != m_psProperties->end();
}

/** Function returns a count of properties contained in the list.
 *
 * \return A count of id's.
 */
size_t property_tracker::count() const
{
	return m_psProperties->size();
}

/** Function retrieves the id's contained in this tracker by copying
 *  them to the given array.
 *
 * \param[out] puiProps - pointer to the array of uint's to receive id's
 * \param[in] stMaxCount - size of the array (max count of elements to retrieve)
 */
size_t property_tracker::get_ids(unsigned int* puiProps, size_t stMaxCount)
{
	size_t tIndex=0;
	for (std::set<unsigned int>::iterator it=m_psProperties->begin();it != m_psProperties->end();++it)
	{
		puiProps[tIndex++]=(*it);
		if(tIndex >= stMaxCount)
			break;
	}

	return tIndex;
}

/** Function enumerates id's contained in this property_tracker using
 *  a callback function.
 *
 * \param[in] pfn - function to be called
 * \param[in] pParam - parameter to pass to the callback
 */
void property_tracker::enum_ids(bool(*pfn)(unsigned int uiProp, void* pParam), void* pParam)
{
	for (std::set<unsigned int>::iterator it=m_psProperties->begin();it != m_psProperties->end();++it)
	{
		if(!(*pfn)((*it), pParam))
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// config class

#define m_pvProps ((std::vector<property>*)m_hProps)

config::config(config_base_types eCfgType) :
	m_lock(),
	m_hProps((void*)new std::vector<property>),
	m_pszCurrentPath(nullptr)
{
	switch(eCfgType)
	{
	//case eXml:
	//	m_pCfgBase = new xml_cfg;
	//	break;
	case eIni:
		m_pCfgBase = new ini_cfg;
		break;
	default:
		throw std::runtime_error("Undefined config base type");
	}
}

/** Destructs the config class.
 */
config::~config()
{
	delete m_pvProps;
	delete [] m_pszCurrentPath;
	delete m_pCfgBase;
}

/** Function opens the specified file using the underlying config base
 *  and converts the values read to a list of properties registered
 *  earlier.
 *
 * \param[in] pszPath - path to a file to be read
 */
void config::read(const wchar_t* pszPath)
{
	assert(pszPath);
	if(!pszPath)
		throw std::runtime_error("Path to the file not provided");

	// remembering path before operation and not freeing it on fail is done on purpose here
	// (to support future write() to this file in case file does not exist yet)
	size_t stLen = _tcslen(pszPath);
	if(stLen)
	{
		delete [] m_pszCurrentPath;
		m_pszCurrentPath = new wchar_t[stLen + 1];
		_tcscpy(m_pszCurrentPath, pszPath);
		m_pszCurrentPath[stLen] = _T('\0');
	}

	m_lock.Lock();
	try
	{
		// read the data using underlying object
		m_pCfgBase->read(pszPath);

		// and transform it to eatable form using registered properties
		load_registered();
	}
	catch(...)
	{
		m_lock.Unlock();
		throw;
	}
	m_lock.Unlock();
}

/** Reads the configuration data from the provided buffer.
 *
 * \param[in] pszData - pointer to the buffer with data
 * \param[in] stSize - size of the data in buffer
 */
void config::read_from_buffer(const wchar_t* pszData, size_t stSize)
{
	m_lock.Lock();
	try
	{
		m_pCfgBase->read_from_buffer(pszData, stSize);

		// and transform it to eatable form using registered properties
		load_registered();
	}
	catch(...)
	{
		m_lock.Unlock();
		throw;
	}
	m_lock.Unlock();
}

/** Writes all the registered properties into the given file using
 *  the underlying config base to do this.
 *
 * \param[in] pszPath - path to a file to write the properties to
 */
void config::write(const wchar_t* pszPath)
{
	if(!m_pszCurrentPath && !pszPath)
		throw std::runtime_error("No path specified");

	m_lock.Lock();

	try
	{
		// store current properties to the underlying object
		store_registered();

		if(pszPath)
		{
			size_t stLen = _tcslen(pszPath);
			if(stLen)
			{
				delete [] m_pszCurrentPath;
				m_pszCurrentPath = new wchar_t[stLen + 1];
				_tcscpy(m_pszCurrentPath, pszPath);
				m_pszCurrentPath[stLen] = _T('\0');
			}
		}

		// and save
		m_pCfgBase->save(m_pszCurrentPath);
	}
	catch(...)
	{
		m_lock.Unlock();
		throw;
	}

	m_lock.Unlock();
}

/** Function returns a property type for a given property id.
 *
 * \param[in] uiProp - property id to get info about
 * \return The property type along with its flags.
 */
unsigned int config::get_type(unsigned int uiProp)
{
	m_lock.Lock();
	unsigned int uiRet=m_pvProps->at(uiProp).get_type();
	m_lock.Unlock();

	return uiRet;
}

/** Retrieves the count of values in the specified property.
 *
 * \param[in] uiProp - property id to retrieve information about
 * \return Count of values.
 */
size_t config::get_value_count(unsigned int uiProp)
{
	m_lock.Lock();
	size_t stRet=m_pvProps->at(uiProp).get_count();
	m_lock.Unlock();

	return stRet;
}

/** Removes an array value at a given index.
 *
 * \param[in] uiProp - property id to have the value removed
 * \param[in] stIndex - index of the value to remove
 */
void config::remove_array_value(unsigned int uiProp, size_t stIndex)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).remove(stIndex);
	m_lock.Unlock();
}

/** Clears the list of values in the given property.
 *
 * \param[in] uiProp - property id to have the values cleared
 */
void config::clear_array_values(unsigned int uiProp)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).clear_array();
	m_lock.Unlock();
}

/** Retrieves the count of registered properties contained in this config.
 *
 * \return Count of properties.
 */
size_t config::count()
{
	return m_pvProps->size();
}

/** Function registers the signed number property. If the underlying base object
 *  contains a string with a specified key - the value is being translated to 
 *  the value of this property.
 *
 * \param[in] pszName - name of the property
 * \param[in] llDef - default value for the property
 * \param[in] llLo - the lower bound of the allowable value range
 * \param[in] llHi - the higher bound of the allowable value range
 * \param[in] uiFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
unsigned int config::register_signed_num(const wchar_t* pszName, long long llDef, long long llLo, long long llHi, unsigned int uiFlags)
{
	// prepare the property to insert
	property prop(pszName, property::type_signed_num | (uiFlags & property::mask_flags));
	prop.set_signed_range(llLo, llHi);

	// and operate inside the internals
	m_lock.Lock();

	// get the value for the property name
	void* hFind=nullptr;
	if( (hFind=m_pCfgBase->find(pszName)) != nullptr )
	{
		PROPINFO pi;
		while(m_pCfgBase->find_next(hFind, pi))
		{
			assert(!pi.bGroup);
			prop.set_value(pi.pszValue, property::action_add);
		}

		m_pCfgBase->find_close(hFind);
	}
	else if(!(uiFlags & property::flag_array))
		prop.set_signed_num(llDef);

	// add to the vector
	m_pvProps->push_back(prop);
	unsigned int uiProp=(unsigned int)(m_pvProps->size()-1);

	m_lock.Unlock();
		
	return uiProp;
}

/** Function registers the unsigned number property. If the underlying base object
 *  contains a string with a specified key - the value is being translated to 
 *  the value of this property.
 *
 * \param[in] pszName - name of the property
 * \param[in] ullDef - default value for the property
 * \param[in] ullLo - the lower bound of the allowable value range
 * \param[in] ullHi - the higher bound of the allowable value range
 * \param[in] uiFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
unsigned int config::register_unsigned_num(const wchar_t* pszName, unsigned long long ullDef, unsigned long long ullLo, unsigned long long ullHi, unsigned int uiFlags)
{
	// prepare the property to insert
	property prop(pszName, property::type_unsigned_num | (uiFlags & property::mask_flags));
	prop.set_unsigned_range(ullLo, ullHi);

	// and operate inside the internals
	m_lock.Lock();

	// get the value for the property name
	void* hFind=nullptr;
	if( (hFind=m_pCfgBase->find(pszName)) != nullptr )
	{
		PROPINFO pi;
		while(m_pCfgBase->find_next(hFind, pi))
		{
			assert(!pi.bGroup);
			prop.set_value(pi.pszValue, property::action_add);
		}

		m_pCfgBase->find_close(hFind);
	}
	else if(!(uiFlags & property::flag_array))
		prop.set_unsigned_num(ullDef);

	// add to the vector
	m_pvProps->push_back(prop);
	unsigned int uiProp=(unsigned int)(m_pvProps->size()-1);

	m_lock.Unlock();
		
	return uiProp;
}

/** Function registers the boolean property. If the underlying base object
 *  contains a string with a specified key - the value is being translated to 
 *  the value of this property.
 *
 * \param[in] pszName - name of the property
 * \param[in] bDef - default value for the property
 * \param[in] uiFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
unsigned int config::register_bool(const wchar_t* pszName, bool bDef, unsigned int uiFlags)
{
	// prepare the property to insert
	property prop(pszName, property::type_bool | (uiFlags & property::mask_flags));

	// and operate inside the internals
	m_lock.Lock();

	// get the value for the property name
	void* hFind=nullptr;
	if( (hFind=m_pCfgBase->find(pszName)) != nullptr )
	{
		PROPINFO pi;
		while(m_pCfgBase->find_next(hFind, pi))
		{
			assert(!pi.bGroup);
			prop.set_value(pi.pszValue, property::action_add);
		}

		m_pCfgBase->find_close(hFind);
	}
	else if(!(uiFlags & property::flag_array))
		prop.set_bool(bDef);

	// add to the vector
	m_pvProps->push_back(prop);
	unsigned int uiProp=(unsigned int)(m_pvProps->size()-1);

	m_lock.Unlock();
		
	return uiProp;
}

/** Function registers the string property. If the underlying base object
 *  contains a string with a specified key - the value is being translated to 
 *  the value of this property.
 *
 * \param[in] pszName - name of the property
 * \param[in] pszDef - default value for the property
 * \param[in] uiFlags - additional flags that should be associated with property
 * \return Property ID of the newly registered property.
 */
unsigned int config::register_string(const wchar_t* pszName, const wchar_t* pszDef, unsigned int uiFlags)
{
	// prepare the property to insert
	property prop(pszName, property::type_string | (uiFlags & property::mask_flags));

	// and operate inside the internals
	m_lock.Lock();

	// get the value for the property name
	void* hFind=nullptr;
	if( (hFind=m_pCfgBase->find(pszName)) != nullptr )
	{
		PROPINFO pi;
		while(m_pCfgBase->find_next(hFind, pi))
		{
			assert(!pi.bGroup);
			prop.set_value(pi.pszValue, property::action_add);
		}

		m_pCfgBase->find_close(hFind);
	}
	else if(!(uiFlags & property::flag_array))
		prop.set_string(pszDef);

	// add to the vector
	m_pvProps->push_back(prop);
	unsigned int uiProp=(unsigned int)(m_pvProps->size()-1);

	m_lock.Unlock();
		
	return uiProp;
}

/** Function retrieves the value as string.
 *
 * \param[in] uiProp - property to retrieve the value of
 * \param[out] pszBuffer - pointer to a buffer to receive the string (unused
 *						   if retrieving a string value)
 * \param[in] stMaxSize - size of the buffer
 * \param[in] stIndex - index of the value to retrieve (meaningful only for
 *						array-based properties)
 * \return Pointer to the string.
 *
 * \note Always use the returned value instead of the buffer contents. Returned
 *		 value may point to some other memory location instead of pszBuffer.
 */
const wchar_t* config::get_value(unsigned int uiProp, wchar_t* pszBuffer, size_t stMaxSize, size_t stIndex)
{
	m_lock.Lock();
	if(uiProp >= m_pvProps->size())
	{
		m_lock.Unlock();
		throw std::runtime_error("Index out of range");
	}
	const wchar_t* psz=m_pvProps->at(uiProp).get_value(pszBuffer, stMaxSize, stIndex);
	m_lock.Unlock();

	return psz;
}

/** Function retrieves the signed number value.
 *
 * \param[in] uiProp - property to retrieve the value of
 * \param[in] stIndex - index of the value to retrieve (meaningful only for
 *						array-based properties)
 * \return Property value.
 */
long long config::get_signed_num(unsigned int uiProp, size_t stIndex)
{
	m_lock.Lock();
	if(uiProp >= m_pvProps->size())
	{
		m_lock.Unlock();
		throw std::runtime_error("Index out of range");
	}
	long long ll=m_pvProps->at(uiProp).get_signed_num(stIndex);
	m_lock.Unlock();
	return ll;
}

/** Function retrieves the unsigned number value.
 *
 * \param[in] uiProp - property to retrieve the value of
 * \param[in] stIndex - index of the value to retrieve (meaningful only for
 *						array-based properties)
 * \return Property value.
 */
unsigned long long config::get_unsigned_num(unsigned int uiProp, size_t stIndex)
{
	m_lock.Lock();
	if(uiProp >= m_pvProps->size())
	{
		m_lock.Unlock();
		throw std::runtime_error("Index out of range");
	}
	unsigned long long ull=m_pvProps->at(uiProp).get_unsigned_num(stIndex);
	m_lock.Unlock();
	return ull;
}

/** Function retrieves the bool value.
 *
 * \param[in] uiProp - property to retrieve the value of
 * \param[in] stIndex - index of the value to retrieve (meaningful only for
 *						array-based properties)
 * \return Property value.
 */
bool config::get_bool(unsigned int uiProp, size_t stIndex)
{
	m_lock.Lock();
	if(uiProp >= m_pvProps->size())
	{
		m_lock.Unlock();
		throw std::runtime_error("Index out of range");
	}
	bool b=m_pvProps->at(uiProp).get_bool(stIndex);
	m_lock.Unlock();
	return b;
}

/** Function retrieves the string value.
 *
 * \param[in] uiProp - property to retrieve the value of
 * \param[in] stIndex - index of the value to retrieve (meaningful only for
 *						array-based properties)
 * \return Property value.
 */
const wchar_t* config::get_string(unsigned int uiProp, size_t stIndex)
{
	m_lock.Lock();
	if(uiProp >= m_pvProps->size())
	{
		m_lock.Unlock();
		throw std::runtime_error("Index out of range");
	}
	const wchar_t* psz=m_pvProps->at(uiProp).get_string(stIndex);
	m_lock.Unlock();

	return psz;
}

/** Function retrieves the string value.
*
* \param[in] uiProp - property to retrieve the value of
* \param[in] stIndex - index of the value to retrieve (meaningful only for
*						array-based properties)
* \return Property value.
*/
const wchar_t* config::get_string(unsigned int uiProp, wchar_t* pszBuffer, size_t stBufferSize, size_t stIndex)
{
	if(!pszBuffer || stBufferSize < 1)
		return nullptr;

	m_lock.Lock();
	if(uiProp >= m_pvProps->size())
	{
		m_lock.Unlock();
		throw std::runtime_error("Index out of range");
	}
	size_t stLen = 0;
	const wchar_t* psz=m_pvProps->at(uiProp).get_string(stIndex);
	if(psz)
	{
		stLen = _tcslen(psz);
		if(stLen >= stBufferSize)
			stLen = stBufferSize - 1;

		_tcsncpy(pszBuffer, psz, stLen);
	}
	pszBuffer[stLen] = _T('\0');
	m_lock.Unlock();
	return pszBuffer;
}

bool config::enum_properties(const wchar_t* pszName, PFNCFGENUMCALLBACK pfn, void* pParam)
{
	void* pFind = m_pCfgBase->find(pszName);
	if(pFind)
	{
		PROPINFO pi;
		while(m_pCfgBase->find_next(pFind, pi))
		{
			(*pfn)(pi.bGroup, pi.pszName, pi.pszValue, pParam);
		}

		m_pCfgBase->find_close(pFind);
		return true;
	}

	return false;
}

/** Function sets the property value from string.
 *
 * \param[in] uiProp - property id to set the value for
 * \param[in] pszVal - string with property value
 * \param[in] a - action to take if the property is array based
 * \param[in] tIndex - index of a value to set at (for action action_setat)
 * \param[out] pTracker - property tracker that collects the property ID's
 */
void config::set_value(unsigned int uiProp, const wchar_t* pszVal, property::actions a, size_t tIndex, property_tracker* pTracker)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).set_value(pszVal, a, tIndex);
	if(pTracker)
		pTracker->add(uiProp);
	m_lock.Unlock();
	property_changed_notify(uiProp);
}

/** Function sets the signed number property value.
 *
 * \param[in] uiProp - property id to set the value for
 * \param[in] llVal - property value to set
 * \param[in] a - action to take if the property is array based
 * \param[in] tIndex - index of a value to set at (for action action_setat)
 * \param[out] pTracker - property tracker that collects the property ID's
 */
void config::set_signed_num(unsigned int uiProp, long long llVal, property::actions a, size_t tIndex, property_tracker* pTracker)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).set_signed_num(llVal, a, tIndex);
	if(pTracker)
		pTracker->add(uiProp);
	m_lock.Unlock();
	property_changed_notify(uiProp);
}

void config::set_unsigned_num(unsigned int uiProp, unsigned long long ullVal, property::actions a, size_t tIndex, property_tracker* pTracker)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).set_unsigned_num(ullVal, a, tIndex);
	if(pTracker)
		pTracker->add(uiProp);
	m_lock.Unlock();
	property_changed_notify(uiProp);
}

void config::set_bool(unsigned int uiProp, bool bVal, property::actions a, size_t tIndex, property_tracker* pTracker)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).set_bool(bVal, a, tIndex);
	if(pTracker)
		pTracker->add(uiProp);
	m_lock.Unlock();
	property_changed_notify(uiProp);
}

void config::set_string(unsigned int uiProp, const wchar_t* pszVal, property::actions a, size_t tIndex, property_tracker* pTracker)
{
	m_lock.Lock();
	m_pvProps->at(uiProp).set_string(pszVal, a, tIndex);
	if(pTracker)
		pTracker->add(uiProp);
	m_lock.Unlock();
	property_changed_notify(uiProp);
}

void config::set_string(const wchar_t* pszName, const wchar_t* pszVal, property::actions a)
{
	config_base::actions action;
	switch(a)
	{
	case property::action_add:
		action = config_base::action_add;
		break;
	case property::action_replace:
		action = config_base::action_replace;
		break;
	default:
		throw std::runtime_error("Undefined or unsupported action");
	}
	m_pCfgBase->set_value(pszName, pszVal, action);
}

/** Function sets the callback function to be called on property change.
 *  \param[in] pfnCallback - pointer to the function
 *  \param[in] pParam - user defined parameter to pass to the callback
 */
void config::set_callback(PFNPROPERTYCHANGED pfnCallback, void* pParam)
{
	m_pfnNotifyCallback = pfnCallback;
	m_pCallbackParam = pParam;
}

/** Function reads the values for the registered properties from the underlying
 *  base config object.
 */
void config::load_registered()
{
	m_lock.Lock();

	for (std::vector<property>::iterator it=m_pvProps->begin();it != m_pvProps->end();++it)
	{
		// is this an array property ?
		if((*it).is_array())
			(*it).clear_array();

		// and fill with value(s)
		void* hFind=nullptr;
		if( (hFind=m_pCfgBase->find((*it).get_name())) != nullptr)
		{
			PROPINFO pi;
			while(m_pCfgBase->find_next(hFind, pi))
			{
				assert(!pi.bGroup);
				(*it).set_value(pi.pszValue, property::action_add);
			}
		}

		m_pCfgBase->find_close(hFind);
	}

	m_lock.Unlock();
}

/** Function stores the values of a registered properties to the underlying
 *  base config object.
 */
void config::store_registered()
{
	m_lock.Lock();

	wchar_t szBuffer[128];
	for (std::vector<property>::iterator it=m_pvProps->begin();it != m_pvProps->end();++it)
	{
		// clear the current attributes for the property
		m_pCfgBase->clear((*it).get_name());

		// and fill with value(s)
		size_t tCount=(*it).get_count();
		for (size_t t=0;t != tCount;t++)
		{
			m_pCfgBase->set_value((*it).get_name(), (*it).get_value(szBuffer, 128, t));
		}
	}

	m_lock.Unlock();
}

/** Function executes the callback to notify about property value change.
 * \param[in] uiPropID - property ID that changed
 */
void config::property_changed_notify(unsigned int uiPropID)
{
	if(m_pfnNotifyCallback)
		(*m_pfnNotifyCallback)(uiPropID, m_pCallbackParam);
}
