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
/*************************************************************************
	File: Exception.h
	Version: 1.0
	Author: Ixen Gerthannes (ixen@interia.pl)
	File description:
		Contain CException class - a base for any other exception
		types.
	Classes:
		CException
			- provides basic exception functionality.
			- when used with MFC class name is CExceptionEx (it's
				not based on MFC CException!).
*************************************************************************/
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "stdio.h"

#define THROW_EXCEPTIONEX(str_reason, app_code, last_error) throw new CExceptionEx(__FILE__, __LINE__, __FUNCTION__, str_reason, app_code, last_error)

// not too specific - use specialised classes based on this one (this also could be used)
class CExceptionEx
{
protected:
	enum PropType { dtString, dtPtrToString, dtDword, dtSysError };

	struct __EXCPROPINFO
	{
		TCHAR szName[64];				// name of the property (ie."Source file")
		PropType eType;					// type of the property (string, dword, bool, ...)
		void* pData;					// pointer to the value of the property
	};

public:
	CExceptionEx(PCTSTR pszSrcFile, DWORD dwLine, PCTSTR pszFunc, PCTSTR pszReason, DWORD dwReason, DWORD dwLastError=0)
	{
		// init the object with a given values
		_tcsncpy(m_szSourceFile, pszSrcFile, _MAX_PATH);
		m_szSourceFile[_MAX_PATH-1]=_T('\0');
		m_dwSourceLine=dwLine;
		_tcsncpy(m_szFunction, pszFunc, _MAX_PATH);
		m_szFunction[_MAX_PATH-1]=_T('\0');
		SetReason(pszReason);
		m_dwReason=dwReason;
		m_dwError=dwLastError;
	};
	CExceptionEx(PCTSTR pszSrcFile, DWORD dwLine, PCTSTR pszFunc, TCHAR* pszReason,  DWORD dwReason, DWORD dwLastError=0)
	{
		_tcsncpy(m_szSourceFile, pszSrcFile, _MAX_PATH);
		m_szSourceFile[_MAX_PATH-1]=_T('\0');
		m_dwSourceLine=dwLine;
		_tcsncpy(m_szFunction, pszFunc, _MAX_PATH);
		m_szFunction[_MAX_PATH-1]=_T('\0');
		m_pszReason=pszReason;
		m_dwReason=dwReason;
		m_dwError=dwLastError;
	};

	virtual ~CExceptionEx() { delete [] m_pszReason; };

	virtual int RegisterInfo(__EXCPROPINFO* pInfo)
	{
		// if the pInfo is null - return count of a needed props
		if (pInfo == NULL)
			return 6;	// +baseClass::RegisterInfo

		// call base class RegisterInfo

		// function has to register the info to be displayed (called from within GetInfo)
		RegisterProp(pInfo+0, _T("Source file"), dtString, &m_szSourceFile);
		RegisterProp(pInfo+1, _T("Line"), dtDword, &m_dwSourceLine);
		RegisterProp(pInfo+2, _T("Function"), dtString, &m_szFunction);
		RegisterProp(pInfo+3, _T("Reason"), dtPtrToString, &m_pszReason);
		RegisterProp(pInfo+4, _T("App error"), dtDword, &m_dwReason);
		RegisterProp(pInfo+5, _T("System error"), dtSysError, &m_dwError);

		return 6;
	};

public:
	// helpers
	static TCHAR* FormatReason(PCTSTR pszReason, ...)
	{
		const size_t stMaxReason = 1024;
		TCHAR szBuf[stMaxReason];

		va_list marker;
		va_start(marker, pszReason);
		_vsntprintf(szBuf, stMaxReason - 1, pszReason, marker);
		szBuf[stMaxReason - 1] = _T('\0');
		va_end(marker);

		TCHAR *pszData=new TCHAR[_tcslen(szBuf)+1];
		_tcscpy(pszData, szBuf);
		return pszData;
	};

	// formats max info about this exception
	virtual TCHAR* GetInfo(LPCTSTR pszDesc, TCHAR* pszStr, DWORD dwMaxLen)
	{
		const size_t stMaxData = 1024;

		// get the properties
		int iCount=RegisterInfo(NULL);
		__EXCPROPINFO *pepi=new __EXCPROPINFO[iCount];
		RegisterInfo(pepi);			// register all the properties

		// add the desc to the out
		if (pszDesc)
		{
			_tcsncpy(pszStr, pszDesc, dwMaxLen-1);
			pszStr[dwMaxLen-1]=_T('\0');
		}
		else
			pszStr[0]=_T('\0');

		size_t tIndex=_tcslen(pszStr);

		// format the info accordingly
		TCHAR szData[stMaxData];
		for (int i=0;i<iCount;i++)
		{
			// format this line
			switch(pepi[i].eType)
			{
			case dtString:
				{
					if (pszDesc)
						_sntprintf(szData, stMaxData - 1, _T("\r\n\t%s: %s"), pepi[i].szName, (TCHAR*)pepi[i].pData);
					else
						_sntprintf(szData, stMaxData - 1, _T("%s: %s\r\n"), pepi[i].szName, (TCHAR*)pepi[i].pData);
					break;
				}
			case dtPtrToString:
				{
					if (pszDesc)
						_sntprintf(szData, stMaxData - 1, _T("\r\n\t%s: %s"), pepi[i].szName, *((TCHAR**)pepi[i].pData));
					else
						_sntprintf(szData, stMaxData - 1, _T("%s: %s\r\n"), pepi[i].szName, *((TCHAR**)pepi[i].pData));
					break;
				}
			case dtDword:
				{
					if (pszDesc)
						_sntprintf(szData, stMaxData - 1, _T("\r\n\t%s: %lu"), pepi[i].szName, *((DWORD*)pepi[i].pData));
					else
						_sntprintf(szData, stMaxData - 1, _T("%s: %lu\r\n"), pepi[i].szName, *((DWORD*)pepi[i].pData));
					break;
				}
			case dtSysError:
				{
					// get info about the last error (always treated as a system error)
					TCHAR szSystem[1024];
					DWORD dwPos=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, *((DWORD*)pepi[i].pData), 0, szSystem, 1024, NULL);

					// get rid of \r\n at the end of szSystem
					while(--dwPos && (szSystem[dwPos] == 0x0a || szSystem[dwPos] == 0x0d))
						szSystem[dwPos]=_T('\0');

					if (pszDesc)
						_sntprintf(szData, stMaxData - 1, _T("\r\n\t%s: %lu (%s)"), pepi[i].szName, *((DWORD*)pepi[i].pData), szSystem);
					else
						_sntprintf(szData, stMaxData - 1, _T("%s: %lu (%s)\r\n"), pepi[i].szName, *((DWORD*)pepi[i].pData), szSystem);

					break;
				}
			}

			szData[stMaxData - 1] = _T('\0');

			// append the line
			size_t tLen=_tcslen(szData);
			if (tIndex+tLen < dwMaxLen-1)
				_tcscat(pszStr, szData);
		}

		delete [] pepi;
		return pszStr;
	};

protected:
	void SetReason(PCTSTR pszReason) { /*delete [] m_pszReason;*/ if (pszReason) { m_pszReason=new TCHAR[_tcslen(pszReason)+1]; _tcscpy(m_pszReason, pszReason); } else m_pszReason=NULL; };
	void RegisterProp(__EXCPROPINFO* pInfo, PCTSTR pszName, PropType type, void* pData)
	{
		_tcsncpy(pInfo->szName, pszName, 63);
		pInfo->szName[63]=_T('\0');
		pInfo->eType=type;
		pInfo->pData=pData;
	};

public:
	// exception information
	TCHAR m_szSourceFile[_MAX_PATH];		// source file from where the exception is being thrown
	DWORD m_dwSourceLine;					// line in the source file from where exception has been thrown
	TCHAR m_szFunction[_MAX_PATH];			// name of the function in which the exception occured
	TCHAR *m_pszReason;						// description of this error (in english - internal error code - description - human readable)
	DWORD m_dwReason;						// numerical value that states app-level error number
	DWORD m_dwError;						// in most cases GetLastError() when it has any sense
};

#endif