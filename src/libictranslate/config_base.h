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
#ifndef __CONFIG_BASE_H__
#define __CONFIG_BASE_H__

struct PROPINFO
{
	const wchar_t* pszName;		///< Property name
	const wchar_t* pszValue;	///< String value of the property (only for attribute-level property)
	bool bGroup;				///< Group-level property (true) or attribute (false)
};

/** Base config class. Manages the data that can be directly
 *  read or written to the storage medium (xml file, ini file,
 *  registry, ...).
 */
class config_base
{
public:
	/// Actions used when setting value
	enum actions
	{
		action_add,
		action_replace
	};

public:
	virtual ~config_base() {}

/** \name File operations */
/**@{*/
	/// Reads the xml document from the specified file
	virtual void read(const wchar_t* pszPath) = 0;
	/// Processes the data from a given buffer
	virtual void read_from_buffer(const wchar_t* pszBuffer, size_t stLen) = 0;
	/// Saves the internal data to a specified file as the xml document
	virtual void save(const wchar_t* pszPath) = 0;
/**@}*/

/** \name Key and value handling */
/**@{*/
	/// Searches for a specified key (given all the path to a specific string)
	virtual void* find(const wchar_t* pszName) = 0;
	/// Searches for the next string
	virtual bool find_next(void* pFindHandle, PROPINFO& pi) = 0;
	/// Closes the search operation
	virtual void find_close(void* pFindHandle) = 0;

	/// Sets a value for a given key
	virtual void set_value(const wchar_t* pszName, const wchar_t* pszValue, actions a=action_add) = 0;
	/// Clear values for a given property name
	virtual void clear(const wchar_t* pszName) = 0;
/**@}*/
};

#endif
