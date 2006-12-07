/***************************************************************************
 *   Copyright (C) 2004-2006 by Józef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
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
#ifndef __CFG_H__
#define __CFG_H__

/** \file cfg.h
 *  \brief A placeholder for config class.
 */
#include "mutex.h"
#include "libicpf.h"
#include "gen_types.h"
#include "callback.h"
#include "str.h"

BEGIN_ICPF_NAMESPACE

// property types
/// Property type mask
#define PTM_TYPE		0x00ff
/// Signed 32-bit type property
#define PT_INT			0x0001
/// Unsigned 32-bit type property
#define PT_UINT			0x0002
/// Signed 64-bit type property
#define PT_LONGLONG		0x0003
/// Unsigned 64-bit type property
#define PT_ULONGLONG	0x0004
/// Bool type property
#define PT_BOOL			0x0005
/// String type property
#define PT_STRING		0x0006

// some flags
/// Property flags mask
#define PFM_FLAGS		0xff00
/// Standard property flag
#define PF_NULL			0x0000
/// The string specifies a pathname flag
#define PF_PATH			0x1000
/// Flag that force checking if the property has already been registered (when registering a property)
#define PF_CHECK		0x2000
/// This flag indicates that the property has been encrypted with a password (only string values)
#define PF_ENCRYPTED	0x4000
/// The property is currently in decrypted state (but should be encrypted when saving)
#define PF_DECODED		0x8000

class config;

extern config *__g_cfg;

#ifdef _WIN32
ICPFTEMPL_EXTERN template class LIBICPF_API callback2<void, ulong_t, ptr_t>;
#endif

/** \brief Structure contain information about one property.
 *
 *  Struct is used to store information about any property type (name, value,
 *  default value, allowed range, ...).
 */
struct _PROP
{
	int_t iType;			///< Type of the property (PT_*, PF_*)
	char_t *pszName;		///< Property name (as displayed in the .conf file)
	bool bModified;		///< States if the property value has been modified
	union prop			/// Union with different types of properties
	{
		struct INTP	/// Long-type property
		{
			int_t iVal;		///< Long value
			int_t iLo;		///< Minimum allowed value for the int_t property
			int_t iHi;		///< Maximum allowed value for the int_t property
		} i;

		struct UINTP	/// Unsigned int_t-type property
		{
			uint_t uiVal;	///< Unsigned int_t value
			uint_t uiLo;		///< Minimum allowed value for the uint_t property
			uint_t uiHi;		///< Maximum allowed value for the uint_t property
		} ui;

		struct LLVAL	/// Long int_t-type property
		{
			longlong_t llVal;	///< Long int_t value
			longlong_t llLo;		///< Minimum allowed value for the longlong_t property
			longlong_t llHi;		///< Maximum allowed value for the longlong_t property
		} ll;

		struct ULLVAL	/// Unsigned longlong_t-type property
		{
			ulonglong_t ullVal;	///< Unsigned longlong_t value
			ulonglong_t ullLo;	///< Minimum allowed value for the ulonglong_t property
			ulonglong_t ullHi;	///< Maximum allowed value for the ulonglong_t property
		} ull;

		bool bVal;			///< A bool-type value

		char_t* pszVal;		///< A string-type value
	} val;

};

/** \brief Property group handling class
 *
 *  Class is being used to manipulate the property groups (in connection with config::begin_group() and
 *  config::end_group().
 */
class LIBICPF_API prop_group
{
public:
	explicit prop_group(ulong_t ulID);	///< Standard constructor
	~prop_group();						///< Standard destructor

	void add(ulong_t ulProp);		///< Adds a new property id to the list
	bool is_set(ulong_t ulProp);	///< Checks if a property id is set inside this list
	ulong_t count() const;			///< Returns a count of properties in a list
	ulong_t get_at(ulong_t ulIndex);	///< Returns a property id at a given index
	ulong_t get_groupid() const;		///< Retrieves the group id

protected:
	void* m_pProperties;				///< Internal member. Pointer to a storage structure with an int_t.
//	std::vector<int_t> m_vProperties;	///< List of properties in a group
	ulong_t m_ulGroupID;				///< The group ID
};

/** \brief Configuration management class.
 *
 *  Class allows user to read and write configuration file in standard unix
 *  format (comments, empty lines and key=value strings). Class is fully thread-safe.
 *  Access to the properties is done by registering one and then getting or setting
 *  a value using the property identifier.
 */
class LIBICPF_API config
{
public:
/** \name Construction/destruction */
/**@{*/
	explicit config(bool bGlobal);	///< Standard constructor
	~config();	///< Standard destructor
/**@}*/
	
/** \name Reading and writing to the file */
/**@{*/
	int_t read(const char_t *pszFile);	///< Opens the file with the properties
	int_t write(const char_t* pszFile);	///< Saves the registered properties to the file
/**@}*/
	
/** \name Class lock/unlock functions */
/**@{*/
	/// Locks the config class for one thread
	void lock() { m_lock.lock(); };
	/// Unlocks the class
	void unlock() { m_lock.unlock(); };
/**@}*/
	
	// property type management
/** Property types */
/**@{*/
	int_t get_proptype(ulong_t ulProp);			///< Retrieves the property type
/**@}*/

	// registering the properties
/** \name Properties registration functions */
/**@{*/
	/// Registers int_t-type property
	ulong_t register_int(const char_t* pszName, int_t iDef, int_t iLo, int_t iHi, int_t iFlags=PF_NULL | PF_CHECK);
	/// Registers uint_t-type property
	ulong_t register_uint(const char_t* pszName, uint_t uiDef, uint_t uiLo, uint_t uiHi, int_t iFlags=PF_NULL | PF_CHECK);
	/// Registers longlong_t-type property
	ulong_t register_longlong(const char_t* pszName, longlong_t llDef, longlong_t llLo, longlong_t llHi, int_t iFlags=PF_NULL | PF_CHECK);
	/// Registers ulonglong_t-type property
	ulong_t register_ulonglong(const char_t* pszName, ulonglong_t ullDef, ulonglong_t ullLo, ulonglong_t ullHi, int_t iFlags=PF_NULL | PF_CHECK);
	/// Registers bool-type property
	ulong_t register_bool(const char_t* pszName, bool bDef, int_t iFlags=PF_NULL | PF_CHECK);
	/// Registers string-type property
	ulong_t register_string(const char_t* pszName, const char_t* pszDef, int_t iFlags=PF_NULL | PF_CHECK);
/**@}*/
	
	
	// getting property data
/** \name Getting and setting values */
/**@{*/
	int_t get_int(ulong_t ulProp);						///< Gets the value of int_t-type property
	uint_t get_uint(ulong_t ulProp);					///< Gets the value of uint_t-type property
	longlong_t get_longlong(ulong_t ulProp);			///< Gets the value of longlong_t-type property
	ulonglong_t get_ulonglong(ulong_t ulProp);			///< Gets the value of ulonglong_t-type property
	bool get_bool(ulong_t ulProp);						///< Gets the value of bool-type property
	void get_string(ulong_t ulProp, char_t* psz, size_t tMaxLen);	///< Gets the value of string-type property
	char_t* get_string(ulong_t ulProp);							///< Gets the value of ulonglong_t-type property (faster and more dangerous)

	// setting property data
	void set_int(ulong_t ulProp, int_t iVal, prop_group* pGroup=NULL);					///< Sets the value of int_t-type property
	void set_uint(ulong_t ulProp, uint_t uiVal, prop_group* pGroup=NULL);				///< Sets the value of uint_t-type property
	void set_longlong(ulong_t ulProp, longlong_t llVal, prop_group* pGroup=NULL);		///< Sets the value of longlong_t-type property
	void set_ulonglong(ulong_t ulProp, ulonglong_t ullVal, prop_group* pGroup=NULL);	///< Sets the value of ulonglong_t-type property
	void set_bool(ulong_t ulProp, bool bVal, prop_group* pGroup=NULL);					///< Sets the value of bool-type property
	void set_string(ulong_t ulProp, const char_t* pszVal, prop_group* pGroup=NULL);		///< Sets the value of string-type property
/**@}*/

	// group support
/** \name Property group support */
/**@{*/
	prop_group* begin_group(ulong_t ulID) const;					///< Begins a property group (currently handles multiple property changes when setting property values)
	void end_group(prop_group* pGroup);			///< Ends a property group
/**@}*/
	
#ifdef USE_ENCRYPTION
/** \name Encryption related */
/**@{*/
	void set_password(const char_t* pszPass);				///< Sets a password to encrypt/decrypt the properties
/**@}*/
#endif

	static config* get_config();					///< Retrieves the pointer to the global config class

protected:
	char_t* trim(char_t* pszString) const;									///< Gets rid of whitespace characters from a string
	void process_line(const char_t* pszName, const char_t* pszValue);	///< Sets a property value if registered
	void prepare_line(const _PROP* prop, string* pres) const;					///< Prepares a line of text with property key and value to write to a file
	ulong_t is_registered(const char_t* pszName);							///< Checks if a property with a given key has been registered
	ulong_t is_unreg(const char_t* pszName);								///< Chacks if the path is contained in unreg container

#ifdef USE_ENCRYPTION
	void encrypt_property(_PROP* prop) const;									///< Encrypts a string property
	void decrypt_property(_PROP* prop) const;									///< Decrypts a string property
#endif

protected:
	mutex m_lock;							///< Lock for the multi-threaded access to the properties
	void* m_pProps;							///< Properties' storage
	void* m_pUnreg;							///< Properties read from file, but not registered.

	bool m_bModified;						///< Global modification flag - states if any property is in modified state

#ifdef USE_ENCRYPTION
	string m_strPassword;					///< Password to encrypt/decrypt properties with
#endif

public:
	callback2<void, ulong_t, ptr_t> m_clbPropertyChanged;	///< Callback (callback2) which is executed when any property has changed
															// First param is count of properties changed (-1 if all changed), second one the prop_group* (or NULL if none changed)
};

END_ICPF_NAMESPACE

#endif
