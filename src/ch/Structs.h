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
#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include "DestPath.h"
//#include "LogFile.h"
#include "../libicpf/log.h"

/////////////////////////////////////////////////////////////////////////////
// priority

int PriorityToIndex(int nPriority);
int IndexToPriority(int nIndex);
int IndexToPriorityClass(int iIndex);
int PriorityClassToIndex(int iPriority);

///////////////////////////////////////////////////////////////////////////
// Exceptions

#define E_KILL_REQUEST		0x00
#define E_ERROR				0x01
#define E_CANCEL			0x02
#define E_PAUSE				0x03


///////////////////////////////////////////////////////////////////////////
// CLIPBOARDMONITORDATA
class CTaskArray;
class CTask;

struct CLIPBOARDMONITORDATA
{
	HWND m_hwnd;	// hwnd to window
	CTaskArray *m_pTasks;

	volatile bool bKill, bKilled;
};

///////////////////////////////////////////////////////////////////////////
// CProcessingException

class CProcessingException
{
public:
	CProcessingException(int iType, CTask* pTask) { m_iType=iType; m_pTask=pTask; m_dwError=0; };
	CProcessingException(int iType, CTask* pTask, UINT uiFmtID, DWORD dwError, ...);
	CProcessingException(int iType, CTask* pTask, DWORD dwError, const tchar_t* pszDesc);
	void Cleanup();

// Implementation
public:
	int m_iType;	// kill request, error, ...
	CTask* m_pTask;

	CString m_strErrorDesc;
	DWORD m_dwError;
};

#endif