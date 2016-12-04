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
#ifndef __CFGINI_H__
#define __CFGINI_H__

#include "config_base.h"
#include <string>

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
	ini_cfg(const ini_cfg& rSrc) = delete;		///< Copy constructor
	virtual ~ini_cfg();					///< Standard destructor
	/**@}*/

	ini_cfg& operator=(const ini_cfg& rSrc) = delete;

	/** \name File operations */
	/**@{*/
	/// Reads the xml document from the specified file
	void read(const wchar_t* pszPath) override;
	/// Processes the data from a given buffer
	void read_from_buffer(const wchar_t* pszBuffer, size_t stLen) override;
	/// Saves the internal data to a specified file as the xml document
	void save(const wchar_t* pszPath) override;
	/**@}*/

	/** \name Key and value handling */
	/**@{*/
	/// Searches for a specified key (given all the path to a specific string)
	void* find(const wchar_t* pszName) override;
	/// Searches for the next string
	bool find_next(void* pFindHandle, PROPINFO& pi) override;
	/// Closes the search operation
	void find_close(void* pFindHandle) override;

	/// Sets a value for a given key
	void set_value(const wchar_t* pszName, const wchar_t* pszValue, actions a=action_add) override;
	/// Clear values for a given property name
	void clear(const wchar_t* pszName) override;
	/// Clears all entries
	virtual void clear();
	/**@}*/

private:
	/// Parses a single line of the ini file
	void parse_line(const wchar_t* pszLine);

	/// Parses the name of the property
	bool parse_property_name(const wchar_t* pszName, std::wstring& rstrSection, std::wstring& rstrName);
protected:
	void* m_hMainNode;		///< Handle to the internal ini storage
	std::wstring m_strCurrentSection;	///< Current section of the config file
};

#endif
