#ifndef __CFGXML_H__
#define __CFGXML_H__

#include "gen_types.h"
#include "libicpf.h"
#include "config_base.h"

BEGIN_ICPF_NAMESPACE

/** Class provides the necessary base handlers for config class.
 *  It handles the xml data streams contained in the files, providing
 *  a way to set and retrieve data contained in the xml document.
 */
class xml_cfg : public config_base
{
public:
/** \name Construction/destruction/operators */
/**@{*/
	xml_cfg();							///< Standard constructor
	xml_cfg(const xml_cfg& rSrc);		///< Copy construtor
	virtual ~xml_cfg();					///< Standard destructor
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
	virtual const tchar_t* find_next(ptr_t pFindHandle);
	/// Closes the search operation
	virtual void find_close(ptr_t pFindHandle);

	/// Sets a value for a given key (either adds to or replaces the previous value)
	virtual void set_value(const tchar_t* pszName, const tchar_t* pszValue, bool bAdd);
/**@}*/

private:
	/// Find helper - recursively searches for a specific key node
	ptr_t find(ptr_t pNodePtr, const tchar_t* pszName);
	/// Set value helper - searches for a specific node and sets the value
	void set_value(ptr_t pNodePtr, const tchar_t* pszName, const tchar_t* pszValue, bool bAdd);

protected:
	ptr_t m_hStorage;		///< Handle to the internal xml storage
};

END_ICPF_NAMESPACE

#endif
