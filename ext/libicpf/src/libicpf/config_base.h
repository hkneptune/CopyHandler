#ifndef __CONFIG_BASE_H__
#define __CONFIG_BASE_H__

#include "gen_types.h"
#include "libicpf.h"

BEGIN_ICPF_NAMESPACE

struct LIBICPF_API PROPINFO
{
	const tchar_t* pszName;		///< Property name
	const tchar_t* pszValue;	///< String value of the property (only for attribute-level property)
	bool bGroup;				///< Group-level property (true) or attribute (false)
};

/** Base config class. Manages the data that can be directly
 *  read or written to the storage medium (xml file, ini file,
 *  registry, ...).
 */
class LIBICPF_API config_base
{
public:
	/// Actions used when setting value
	enum actions
	{
		action_add,
		action_replace
	};

public:
/** \name File operations */
/**@{*/
	/// Reads the xml document from the specified file
	virtual void read(const tchar_t* pszPath) = 0;
	/// Saves the internal data to a specified file as the xml document
	virtual void save(const tchar_t* pszPath) = 0;
/**@}*/

/** \name Key and value handling */
/**@{*/
	/// Searches for a specified key (given all the path to a specific string)
	virtual ptr_t find(const tchar_t* pszName) = 0;
	/// Searches for the next string
	virtual bool find_next(ptr_t pFindHandle, PROPINFO& pi) = 0;
	/// Closes the search operation
	virtual void find_close(ptr_t pFindHandle) = 0;

	/// Sets a value for a given key
	virtual void set_value(const tchar_t* pszName, const tchar_t* pszValue, actions a=action_add) = 0;
	/// Clear values for a given property name
	virtual void clear(const tchar_t* pszName) = 0;
/**@}*/
};

END_ICPF_NAMESPACE

#endif
