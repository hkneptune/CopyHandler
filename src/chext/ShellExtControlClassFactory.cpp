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
#include "ShellExtControlClassFactory.h"
#include "ShellExtControl.h"
#include <new.h>

STDMETHODIMP ShellExtControlClassFactory::CreateInstance(LPUNKNOWN pUnknown, REFIID riid, LPVOID *ppObject)
{
	if (!ppObject)
		return E_POINTER;

	*ppObject = nullptr;
	if (pUnknown != nullptr)
		return CLASS_E_NOAGGREGATION;

	auto pShellExtControl = new (std::nothrow) CShellExtControl();
	if (!pShellExtControl)
		return E_OUTOFMEMORY;

	const HRESULT hr = pShellExtControl->QueryInterface(riid, ppObject);
	if (FAILED(hr))
		delete pShellExtControl;
	return hr;
}
