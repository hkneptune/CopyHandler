#ifndef __CFGINI_H__
#define __CFGINI_H__

#include "gen_types.h"
#include "libicpf.h"
#include "config_base.h"

BEGIN_ICPF_NAMESPACE

/** Class provides the necessary base handlers for config class.
*  It handles the ini data streams contained in the files, providing
*  a way to set and retrieve data contained in the ini document.
*/
class ini_cfg : public config_base
{
public:
	/** \name Construction/destruction/operators */
	/**@{*/
	ini_cfg();							///< Standard constructor
	ini_cfg(const ini_cfg& rSrc);		///< Copy constructor
	virtual ~ini_cfg();					///< Standard destructor
	/**@}*/

	/** \name File operations */
	/**@{*/
	/// Reads the xml document from the specified file
	virtual void read(const tchar_t* pszPath);
	/// Saves the internal data to a specified file as the xml document
	virtual void save(const tchar_t* pszPath);
	/**@}*/

	/** \name Key and value handling */
	/**@{*/
	/// Searches for a specified key (given all the path to a specific string)
	virtual ptr_t find(const tchar_t* pszName);
	/// Searches for the next string
	virtual bool find_next(ptr_t pFindHandle, PROPINFO& pi);
	/// Closes the search operation
	virtual void find_close(ptr_t pFindHandle);

	/// Sets a value for a given key
	virtual void set_value(const tchar_t* pszName, const tchar_t* pszValue, actions a=action_add);
	/// Clear values for a given property name
	virtual void clear(const tchar_t* pszName);
	/// Clears all entries
	virtual void clear();
	/**@}*/

private:
	/// Parses a single line of the ini file
	void parse_line(const tchar_t* pszLine);

	/// Parses the name of the property
	bool parse_property_name(const tchar_t* pszName, tstring_t& rstrSection, tstring_t& rstrName);
protected:
	ptr_t m_hMainNode;		///< Handle to the internal ini storage
	tstring_t m_strCurrentSection;	///< Current section of the config file
};

END_ICPF_NAMESPACE

#endif
