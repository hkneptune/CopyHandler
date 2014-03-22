/***************************************************************************
*   Copyright (C) 2001-2013 by J�zef Starosczyk                           *
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
#ifndef __TBASEEXCEPTION_H__
#define __TBASEEXCEPTION_H__

#include "libchcore.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TBaseException : public virtual std::exception
{
public:
	TBaseException(EGeneralErrors eErrorCode, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);
	TBaseException(EGeneralErrors eErrorCode, const char* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);

	virtual ~TBaseException();

	// error information
	EGeneralErrors GetErrorCode() const { return m_eErrorCode; }

	virtual void GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;
	virtual void GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;

private:
	TBaseException();

	// location info
	const wchar_t* GetSourceFile() const { return m_pszFile; }
	size_t GetSourceLineNumber() const { return m_stLineNumber; }
	const wchar_t* GetFunctionName() const { return m_pszFunction; }

protected:
	// what happened?
	EGeneralErrors m_eErrorCode;

	// where it happened?
	const wchar_t* m_pszMsg;
	bool m_bDeleteMsg;

	const wchar_t* m_pszFile;
	const wchar_t* m_pszFunction;
	size_t m_stLineNumber;
};

END_CHCORE_NAMESPACE

#endif