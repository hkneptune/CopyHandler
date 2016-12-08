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
#ifndef __CFG_H__
#define __CFG_H__

/** \file cfg.h
 *  \brief A placeholder for config class.
 */
#include "config_base.h"
#include "config_property.h"
#include <afxmt.h>

/// Callback function definition
typedef void(*PFNPROPERTYCHANGED)(unsigned int, void*);
/// Enumeration callback
typedef void(*PFNCFGENUMCALLBACK)(bool, const wchar_t*, const wchar_t*, void*);

/** \brief Property group handling class
 *
 *  Class is being used to manipulate the property groups (in connection with config::begin_group() and
 *  config::end_group().
 */
class property_tracker
{
public:
/** \name Construction/destruction/operators */
/**@{*/
	property_tracker();											///< Standard constructor
	property_tracker(const property_tracker& rSrc);				///< Copy constructor
	~property_tracker();										///< Standard destructor

	property_tracker& operator=(const property_tracker& rSrc) = delete;	///< Assignment operator
/**@}*/

/** \name Operations */
/**@{*/
	void add(unsigned int uiProp);			///< Adds a new property id to the list
	bool is_set(unsigned int uiProp);			///< Checks if a property id is set inside this list
	size_t count() const;				///< Returns a count of properties in a list

	/// Retrieves the list of ID's
	size_t get_ids(unsigned int* puiProps, size_t stMaxCount);
	/// Retrieves the list of ID's using an enumeration function
	void enum_ids(bool(*pfn)(unsigned int uiProp, void* pParam), void* pParam);
/**@}*/

protected:
	void* m_hProperties;				///< Internal member. Pointer to a storage structure with an int.
};

/** \brief Configuration management class.
 *
 *  Class allows user to read and write configuration file in standard unix
 *  format (comments, empty lines and key=value strings). Class is fully thread-safe.
 *  Access to the properties is done by registering one and then getting or setting
 *  a value using the property identifier.
 */
class config
{
public:
	enum config_base_types
	{
		eXml,
		eIni
	};
public:
/** \name Construction/destruction */
/**@{*/
	explicit config(config_base_types eCfgType);	///< Standard constructor
	config(const config& rSrc) = delete;
	virtual ~config();						///< Standard destructor

	config& operator=(const config& rSrc) = delete;
	/**@}*/
	
/** \name Reading and writing to the external medium */
/**@{*/
	void read(const wchar_t *pszPath);		///< Reads the properties from the source file
	void read_from_buffer(const wchar_t* pszData, size_t stSize);
	void write(const wchar_t* pszPath);		///< Saves the properties to the file
/**@}*/

/** \name Class lock/unlock functions */
/**@{*/
	/// Locks the config class for one thread
	void lock() { m_lock.Lock(); }
	/// Unlocks the class
	void unlock() { m_lock.Unlock(); }
/**@}*/
	
	// property type management
/** Property types */
/**@{*/
	unsigned int get_type(unsigned int uiProp);						///< Retrieves the property type
	size_t get_value_count(unsigned int uiProp);				///< Retrieves the count of values for array-based property types
	void remove_array_value(unsigned int uiProp, size_t stIndex);	///< Removes a value at a specified index in array-based property type
	void clear_array_values(unsigned int uiProp);				///< Removes all values in array-based property
	size_t count();										///< Retrieves the count of properties contained in this config
/**@}*/

	// registering the properties
/** \name Properties registration functions */
/**@{*/
	/// Registers signed number-type property
	unsigned int register_signed_num(const wchar_t* pszName, long long llDef, long long llLo, long long llHi, unsigned int uiFlags=property::flag_none);
	/// Registers unsigned number-type property
	unsigned int register_unsigned_num(const wchar_t* pszName, unsigned long long ullDef, unsigned long long ullLo, unsigned long long ullHi, unsigned int uiFlags=property::flag_none);
	/// Registers bool-type property
	unsigned int register_bool(const wchar_t* pszName, bool bDef, unsigned int uiFlags=property::flag_none);
	/// Registers string-type property
	unsigned int register_string(const wchar_t* pszName, const wchar_t* pszDef, unsigned int uiFlags=property::flag_none);
/**@}*/
	
	// getting property data
/** \name Getting and setting values */
/**@{*/
	/// Gets the value of string-type property
	const wchar_t* get_value(unsigned int uiProp, wchar_t* pszBuffer, size_t stMaxSize, size_t stIndex=0);
	/// Gets the value of longlong_t-type property
	long long get_signed_num(unsigned int uiProp, size_t stIndex=0);
	/// Gets the value of ulonglong_t-type property
	unsigned long long get_unsigned_num(unsigned int uiProp, size_t stIndex=0);
	/// Gets the value of bool-type property
	bool get_bool(unsigned int uiProp, size_t stIndex=0);
	/// Gets the value of string-type property
	const wchar_t* get_string(unsigned int uiProp, size_t stIndex=0);
	/// Retrieves the copy of the string
	const wchar_t* get_string(unsigned int uiProp, wchar_t* pszBuffer, size_t stBufferSize, size_t stIndex=0);

	/// Enumerates attributes (and groups)
	bool enum_properties(const wchar_t* pszName, PFNCFGENUMCALLBACK pfn, void* pParam);

	// setting property data
	/// Sets the value from the string
	void set_value(unsigned int uiProp, const wchar_t* pszVal, property::actions a=property::action_replace, size_t tIndex=0, property_tracker* pTracker=nullptr);
	/// Sets the value of longlong_t-type property
	void set_signed_num(unsigned int uiProp, long long llVal, property::actions a=property::action_replace, size_t tIndex=0, property_tracker* pTracker=nullptr);
	/// Sets the value of ulonglong_t-type property
	void set_unsigned_num(unsigned int uiProp, unsigned long long ullVal, property::actions a=property::action_replace, size_t tIndex=0, property_tracker* pTracker=nullptr);
	/// Sets the value of bool-type property
	void set_bool(unsigned int uiProp, bool bVal, property::actions a=property::action_replace, size_t tIndex=0, property_tracker* pTracker=nullptr);
	/// Sets the value of string-type property
	void set_string(unsigned int uiProp, const wchar_t* pszVal, property::actions a=property::action_replace, size_t tIndex=0, property_tracker* pTracker=nullptr);
	/// Sets the string manually, without using registered properties; does not notify about change.
	void set_string(const wchar_t* pszName, const wchar_t* pszVal, property::actions a=property::action_replace);
/**@}*/

/** \name Notifications */
/**@{*/
	void set_callback(PFNPROPERTYCHANGED pfnCallback, void* pParam);
/**@}*/

protected:
	void load_registered();			///< Loads the registered property values from the underlying config base
	void store_registered();		///< Stores the registered property values to the underlying config base

	void property_changed_notify(unsigned int uiPropID);	///< Calls the callback function to notify about the property value change
protected:
	CCriticalSection m_lock;					///< Lock for the multi-threaded access to the properties
	void* m_hProps = nullptr;					///< Handle to the registered property storage
	config_base* m_pCfgBase = nullptr;		///< Underlying base for this class
	wchar_t* m_pszCurrentPath = nullptr;		///< Current path (one specified when reading the file)
	PFNPROPERTYCHANGED m_pfnNotifyCallback = nullptr;	///< Function to be called when property changes
	void* m_pCallbackParam = nullptr;					///< User-defined parameter to pass to the callback function
};

#endif
