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
#include "stdafx.h"
#include "ClassFactory.h"
#include "Logger.h"

extern std::atomic<long> g_DllRefCount;

ClassFactory::ClassFactory() :
	m_spLog(GetLogger(L"ClassFactory"))
{
	++g_DllRefCount;
	LOG_DEBUG(m_spLog) << L"Constructing ClassFactory";
}

ClassFactory::~ClassFactory()
{
	--g_DllRefCount;
}

STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	if (!ppReturn)
		return E_POINTER;

	*ppReturn = nullptr;

	if (IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	else if (IsEqualIID(riid, IID_IClassFactory))
		*ppReturn = (IClassFactory*)this;
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef()
{
	return InterlockedIncrement(&m_ulRefCnt);
}

STDMETHODIMP_(ULONG) ClassFactory::Release()
{
	ULONG ulNewValue = InterlockedDecrement(&m_ulRefCnt);
	if(ulNewValue)
		return ulNewValue;

	delete this;

	return 0UL;
}

STDMETHODIMP ClassFactory::LockServer(BOOL)
{
	return E_NOTIMPL;
}
