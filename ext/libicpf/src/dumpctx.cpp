/***************************************************************************
 *   Copyright (C) 2004-2006 by Józef Starosczyk                           *
 *   ixen@draknet.sytes.net                                                *
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
/** \file dumpctx.cpp
 *  \brief Contain implementation of class dumpctx - a debug helper class.
 */
#include "dumpctx.h"
#include <stdio.h>
#include "log.h"
#include "macros.h"

BEGIN_ICPF_NAMESPACE

#ifdef _WIN32
	#define snprintf _snprintf
#endif

/** Constructor stores the passed data in the internal members.
 * \param[in] uiType - type of dump (one of the DCX_*)
 * \param[in] pParam - additional param - the type of theis param depends on the ulType
 */
dumpctx::dumpctx(uint_t uiType, ptr_t pParam) : 
	m_lock("dumpctx::m_lock")
{
	m_uiType=uiType;
	if (uiType == DCX_FILE)
	{
		size_t tLen=strlen((const char_t*)pParam);
		m_pParam=(ptr_t)new char_t[tLen+1];
		strcpy((char_t*)m_pParam, (const char_t*)pParam);
	}
	else
		m_pParam=pParam;
}

/** Destructor frees the internal data if needed
 */
dumpctx::~dumpctx()
{
	if (m_uiType == DCX_FILE)
		delete [] (char_t*)m_pParam;
}

/** Function opens the dump. It means initializing the internal string
 *  that will contain the dump result and locking the class (using mutex).
 *  \note Always call the close() member if you have opened the dump.
 */
void dumpctx::open(const char_t* pszObject)
{
	MLOCK(m_lock);
	m_strBuffer=pszObject;
	m_strBuffer+="\n";
}

/** Closes the dump. Depending on the type specified in the constructor the
 *  function outputs the dump to the specified location. Also the internal 
 *  buffer is cleared.
 */
void dumpctx::close()
{
	// perform a dump - depending on the type of a dest object
	switch(m_uiType)
	{
		case DCX_STD:
		{
			printf(m_strBuffer);
			break;
		}
		case DCX_FILE:
		{
			FILE* pFile=fopen((const char_t*)m_pParam, "a");
			if (pFile != NULL)
			{
				fprintf(pFile, m_strBuffer);
				fclose(pFile);
			}
			
			break;
		}
		case DCX_FILEHANDLE:
		{
			fprintf((FILE*)m_pParam, m_strBuffer);
			break;
		}
		case DCX_LOG:
		{
			((log_file*)m_pParam)->logd(m_strBuffer);
			break;
		}
	}
	
	// clean the internal buffer
	m_strBuffer.clear();
	
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given ansi string.
 *  Strings longer than MAX_DUMP characters will be truncated.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] pszValue - an ansi string - the value of a given member
 */
void dumpctx::dump(const char_t* pszName, const char_t* pszValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (string):\n\t" PTRFMT " (\"" STRFMT "\")\n", pszName, pszValue, pszValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given unicode string.
 *  Strings longer than MAX_DUMP characters will be truncated.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] pszValue - an unicode string - the value of a given member
 */
void dumpctx::dump(const char_t* pszName, const wchar_t* pszValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (wide string):\n\t" PTRFMT " (\"" WSTRFMT "\")\n", pszName, pszValue, pszValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given character.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] cValue - a character (signed char_t) value
 */
void dumpctx::dump(const char_t* pszName, const char_t cValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (char_t):\n\t'" CHARFMT "' (hex: " CXFMT " / dec: " CFMT ")\n", pszName, cValue, (short_t)cValue, (short_t)cValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given short_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] sValue - a short_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const short_t sValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (short_t):\n\t" SFMT " (hex: " SXFMT ")\n", pszName, sValue, sValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given int_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] iValue - a int_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const int_t iValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (int_t):\n\t" LFMT " (hex: " LXFMT ")\n", pszName, iValue, iValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given uchar_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] ucValue - an uchar_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const uchar_t ucValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (uchar_t):\n\t'" UCHARFMT "' (hex: " UCXFMT " / dec: " UCFMT ")\n", pszName, ucValue, (ushort_t)ucValue, (ushort_t)ucValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given ushort_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] usValue - an ushort_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const ushort_t usValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (ushort_t):\n\t" USFMT " (hex: " USXFMT ")\n", pszName, usValue, usValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the given uint_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] uiValue - an uint_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const uint_t uiValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (uint_t):\n\t" ULFMT " (hex: " ULXFMT ")\n", pszName, uiValue, uiValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the longlong_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] llValue - a longlong_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const longlong_t llValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (longlong_t):\n\t" LLFMT " (hex: " LLXFMT ")\n", pszName, llValue, llValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the ulonglong_t.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] ullValue - an ulonglong_t value to dump
 */
void dumpctx::dump(const char_t* pszName, const ulonglong_t ullValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (ulonglong_t):\n\t" ULLFMT " (hex: " ULLXFMT ")\n", pszName, ullValue, ullValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

/** Function dumps (stores in the internal string object) the untyped pointer.
 * \param[in] pszName - name of the member variable of the dumped object
 * \param[in] pValue - an untyped pointer value to dump
 */
void dumpctx::dump(const char_t* pszName, const ptr_t pValue)
{
	snprintf(m_szBuffer, MAX_DUMP, STRFMT " (ptr_t):\n\t" PTRFMT "\n", pszName, pValue);
	m_szBuffer[MAX_DUMP-1]='\0';
	MLOCK(m_lock);
	m_strBuffer+=m_szBuffer;
	MUNLOCK(m_lock);
}

END_ICPF_NAMESPACE
