// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  ShellPathsHelpers.cpp
/// @date  2011/04/10
/// @brief Contains implementation of functions handling shell paths conversions.
// ============================================================================
#include "stdafx.h"
#include "ShellPathsHelpers.h"
#include <boost/shared_array.hpp>
#include "../libchcore/TPath.h"
#include "../libchcore/TPathContainer.h"

namespace ShellPathsHelpers {

HRESULT GetPathFromITEMIDLIST(LPCITEMIDLIST pidlFolder, chcore::TSmartPath& pathFolder)
{
	if(!pidlFolder)
		return E_INVALIDARG;

	size_t stMaxPathSize = 32768;
	boost::shared_array<wchar_t> szDstPathBuffer(new wchar_t[stMaxPathSize]);
	szDstPathBuffer.get()[0] = _T('\0');

	if(!SHGetPathFromIDList(pidlFolder, szDstPathBuffer.get()))
		return E_FAIL;

	pathFolder = chcore::PathFromString(szDstPathBuffer.get());

	return S_OK;
}

void GetPathsFromHDROP(HDROP hDrop, chcore::TPathContainer& tPathContainer)
{
	tPathContainer.Clear();

	// get clipboard data
	UINT uiFilesCount = DragQueryFile(hDrop, 0xffffffff, NULL, 0);

	const size_t stMaxPathLength = 32768;
	TCHAR szPath[stMaxPathLength];

	// get files and put it in a table
	for(UINT uiIndex = 0; uiIndex < uiFilesCount; ++uiIndex)
	{
		UINT uiSize = DragQueryFile(hDrop, uiIndex, szPath, stMaxPathLength);
		szPath[uiSize] = _T('\0');

		tPathContainer.Add(chcore::PathFromString(szPath));
	}
}

HRESULT GetPathsFromIDataObject(IDataObject* piDataObject, chcore::TPathContainer& tPathContainer)
{
	tPathContainer.Clear();

	_ASSERTE(piDataObject);
	if(!piDataObject)
		return E_INVALIDARG;

	// retrieve some informations from the data object
	STGMEDIUM medium;
	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	// retrieve the CF_HDROP-type data from data object
	HRESULT hResult = piDataObject->QueryGetData(&fe);
	if(hResult != S_OK)
		return S_FALSE;

	hResult = piDataObject->GetData(&fe, &medium);
	if(SUCCEEDED(hResult))
		GetPathsFromHDROP(static_cast<HDROP>(medium.hGlobal), tPathContainer);

	ReleaseStgMedium(&medium);

	return S_OK;
}

}
