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
#ifndef __SHELLEXTCONTROL_H_
#define __SHELLEXTCONTROL_H_

#include "resource.h"       // main symbols
#include "guids.h"
#include "../liblogger/TLogger.h"
#include "../libchcore/TIpcMutex.h"
#include "../libchcore/TSharedMemory.h"
#include "IShellExtControl.h"

class CShellExtControl : public IShellExtControl
{
public:
	CShellExtControl();
	~CShellExtControl();

	STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;

	STDMETHOD(GetVersion)(LONG* plVersion, BSTR* pbstrVersion);
	STDMETHOD(SetFlags)(LONG lFlags, LONG lMask);
	STDMETHOD(GetFlags)(LONG* plFlags);

private:
	HRESULT Initialize();

private:
	volatile ULONG m_ulRefCnt = 0;

	HANDLE m_hMemory = nullptr;
	chcore::TIpcMutex m_mutex;

	struct SHELLEXT_DATA
	{
		long m_lFlags = 0;
	} *m_pShellExtData = nullptr;

	logger::TLoggerPtr m_spLog;
	chcore::TSharedMemory m_shmConfiguration;
	bool m_bInitialized = false;
};

#endif //__SHELLEXTCONTROL_H_
