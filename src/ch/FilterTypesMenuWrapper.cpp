#include "stdafx.h"
#include "FilterTypesMenuWrapper.h"
#include "ch.h"
#include "resource.h"
#include "../libstring/TStringArray.h"
#include <regex>
#include "../libchcore/TCoreException.h"

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

bool FilterTypesMenuWrapper::ValidateFilter(const chcore::TStringPatternArray& arrPattern)
{
	CStringA strMsg;
	try
	{
		arrPattern.MatchesAny(chcore::PathFromString(L""));
	}
	catch(const chcore::TCoreException& e)
	{
		const size_t BufferSize = 8192;
		wchar_t szData[BufferSize] = {};

		e.GetErrorInfo(szData, BufferSize);
		strMsg = szData;
	}

	if(!strMsg.IsEmpty())
	{
		CString strFmt;
		strFmt.Format(L"%S", (PCSTR)strMsg);

		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_INVALID_FILTER_STRING));
		fmt.SetParam(_T("%err"), strFmt);
		AfxMessageBox(fmt.ToString());
		return false;
	}

	return true;
}
