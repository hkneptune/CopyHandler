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
#ifndef __TEXCEPTION_H__
#define __TEXCEPTION_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

// throws core exception object
#define THROW_CORE_EXCEPTION(error_code)\
	throw TCoreException(error_code, __FILEW__, __LINE__, __FUNCTIONW__)

class LIBCHCORE_API TCoreException
{
public:
	TCoreException(EGeneralErrors eErrorCode);
	TCoreException(EGeneralErrors eErrorCode, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);

	// error information
	EGeneralErrors GetErrorCode() const { return m_eErrorCode; }

	// location info
	const wchar_t* GetSourceFile() const { return m_pszFile; }
	size_t GetSourceLineNumber() const { return m_strLineNumber; }
	const wchar_t* GetFunctionName() const { return m_pszFunction; }

private:
	TCoreException() {}

protected:
	// what happened?
	EGeneralErrors m_eErrorCode;

	// where it happened?
	const wchar_t* m_pszFile;
	const wchar_t* m_pszFunction;
	size_t m_strLineNumber;
};

END_CHCORE_NAMESPACE

#endif
