#include "stdafx.h"
#include "FilterTypesMenuWrapper.h"
#include "ch.h"
#include "resource.h"
#include "../libstring/TStringArray.h"

void FilterTypesMenuWrapper::Init()
{
	HMENU hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_FILTER_TYPE_MENU));
	m_menuFilterType.Attach(hMenu);

	CMenu* pPopup = m_menuFilterType.GetSubMenu(0);
	for(int iIndex = 0; iIndex < pPopup->GetMenuItemCount(); ++iIndex)
	{
		int iCmd = pPopup->GetMenuItemID(iIndex);
		if(iCmd > 0)
		{
			CString strText;
			pPopup->GetMenuString(iIndex, strText, MF_BYPOSITION);
			m_mapFilterEntries.insert({ iCmd, (PCTSTR)strText });
		}
	}
}

void FilterTypesMenuWrapper::StartTracking(bool bIncludeMask, CWnd& rParent, int iBtnID)
{
	m_bTracksIncludeButton = bIncludeMask;
	CRect rect;
	rParent.GetDlgItem(iBtnID)->GetWindowRect(&rect);

	m_menuFilterType.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.right + 1, rect.top, &rParent);
}

void FilterTypesMenuWrapper::OnCommand(int iCommandID, CComboBox& rCombo)
{
	auto iterFnd = m_mapFilterEntries.find(iCommandID);
	if(iterFnd != m_mapFilterEntries.end())
	{
		string::TString strEntry = iterFnd->second.c_str();
		string::TStringArray arrStrings;
		strEntry.Split(L"\t", arrStrings);

		if(arrStrings.GetCount() > 1)
		{
			CString strText;
			rCombo.GetWindowText(strText);

			CString strParsed = arrStrings.GetAt(0).c_str();
			if(!strText.IsEmpty() && strText.Right(1) != L";" && strParsed != L";")
				strText += L";";

			rCombo.SetWindowText(strText + strParsed);
		}
	}
}
