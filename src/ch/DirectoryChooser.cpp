// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#include "DirectoryChooser.h"
#include "ch.h"
#include "FolderDialog.h"
#include "..\libchcore\TPathContainer.h"
#include "..\libchcore\TPath.h"
#include "CfgProperties.h"
#include "resource.h"

namespace DirectoryChooser
{
INT_PTR ChooseDirectory(chcore::EOperationType eOperation, const chcore::TPathContainer& rInputPaths, chcore::TSmartPath& rSelectedPath)
{
	rSelectedPath.Clear();

	chcore::TConfig& rConfig = GetConfig();

	// get dest folder
	CFolderDialog dlg;

	GetPropValue<PP_SHORTCUTS>(rConfig, dlg.m_bdData.cvShortcuts);
	GetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_bdData.cvRecent);

	dlg.m_bdData.bExtended=GetPropValue<PP_FDEXTENDEDVIEW>(rConfig);
	dlg.m_bdData.cx=GetPropValue<PP_FDWIDTH>(rConfig);
	dlg.m_bdData.cy=GetPropValue<PP_FDHEIGHT>(rConfig);
	dlg.m_bdData.iView=GetPropValue<PP_FDSHORTCUTLISTSTYLE>(rConfig);
	dlg.m_bdData.bIgnoreDialogs=GetPropValue<PP_FDIGNORESHELLDIALOGS>(rConfig);

	dlg.m_bdData.strInitialDir=(dlg.m_bdData.cvRecent.size() > 0) ? dlg.m_bdData.cvRecent.at(0) : _T("");

	if(eOperation == chcore::eOperation_Copy)
		dlg.m_bdData.strCaption = GetResManager().LoadString(IDS_TITLECOPY_STRING);
	else if(eOperation == chcore::eOperation_Move)
		dlg.m_bdData.strCaption = GetResManager().LoadString(IDS_TITLEMOVE_STRING);
	else
		dlg.m_bdData.strCaption = GetResManager().LoadString(IDS_TITLEUNKNOWNOPERATION_STRING);
	dlg.m_bdData.strText = GetResManager().LoadString(IDS_MAINBROWSETEXT_STRING);

	// set count of data to display
	size_t stClipboardSize = rInputPaths.GetCount();
	size_t stEntries = (stClipboardSize > 3) ? 2 : stClipboardSize;
	for(size_t stIndex = 0; stIndex < stEntries; stIndex++)
	{
		dlg.m_bdData.strText += rInputPaths.GetAt(stIndex).ToString();
		dlg.m_bdData.strText += _T("\n");
	}

	// add ...
	if (stEntries < stClipboardSize)
		dlg.m_bdData.strText+=_T("...");

	// show window
	INT_PTR iResult = dlg.DoModal();

	// set data to config
	SetPropValue<PP_SHORTCUTS>(rConfig, dlg.m_bdData.cvShortcuts);
	SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_bdData.cvRecent);

	SetPropValue<PP_FDEXTENDEDVIEW>(rConfig, dlg.m_bdData.bExtended);
	SetPropValue<PP_FDWIDTH>(rConfig, dlg.m_bdData.cx);
	SetPropValue<PP_FDHEIGHT>(rConfig, dlg.m_bdData.cy);
	SetPropValue<PP_FDSHORTCUTLISTSTYLE>(rConfig, dlg.m_bdData.iView);
	SetPropValue<PP_FDIGNORESHELLDIALOGS>(rConfig, dlg.m_bdData.bIgnoreDialogs);
	rConfig.Write();

	if(iResult == IDOK)
	{
		CString strPath;
		dlg.GetPath(strPath);

		rSelectedPath = chcore::PathFromString(strPath);
	}

	return iResult;
}
}
