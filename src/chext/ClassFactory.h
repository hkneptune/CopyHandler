// ============================================================================
//  Copyright (C) 2001-2019 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#pragma once

#include "../liblogger/TLogger.h"

class ClassFactory : public IClassFactory
{
public:
	ClassFactory();
	virtual ~ClassFactory();

	//IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID*) override;
	STDMETHODIMP_(DWORD) AddRef() override;
	STDMETHODIMP_(DWORD) Release() override;

	//IClassFactory methods
	STDMETHODIMP LockServer(BOOL) override;

private:
	DWORD m_ulRefCnt = 0;
	logger::TLoggerPtr m_spLog;
};
