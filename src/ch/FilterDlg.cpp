// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#include "ch.h"
#include "FilterDlg.h"
#include "resource.h"
#include "../libstring/TStringArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

CFilterDlg::CFilterDlg()
	:CLanguageDialog(IDD_FILTER_DIALOG),
	m_iArchive(FALSE),
	m_bAttributes(FALSE),
	m_bDate1(FALSE),
	m_bDate2(FALSE),
	m_iDirectory(FALSE),
	m_bFilter(FALSE),
	m_iHidden(FALSE),
	m_iReadOnly(FALSE),
	m_bSize(FALSE),
	m_uiSize1(0),
	m_bSize2(FALSE),
	m_uiSize2(0),
	m_iSystem(FALSE),
	m_bExclude(FALSE)
{
}

void CFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILTEREXCLUDE_COMBO, m_ctlExcludeMask);
	DDX_Control(pDX, IDC_SIZE2_SPIN, m_ctlSpin2);
	DDX_Control(pDX, IDC_SIZE1_SPIN, m_ctlSpin1);
	DDX_Control(pDX, IDC_TIME2_DATETIMEPICKER, m_ctlTime2);
	DDX_Control(pDX, IDC_TIME1_DATETIMEPICKER, m_ctlTime1);
	DDX_Control(pDX, IDC_SIZETYPE2_COMBO, m_ctlSizeType2);
	DDX_Control(pDX, IDC_SIZETYPE1_COMBO, m_ctlSizeType1);
	DDX_Control(pDX, IDC_SIZE2MULTI_COMBO, m_ctlSize2Multi);
	DDX_Control(pDX, IDC_SIZE1MULTI_COMBO, m_ctlSize1Multi);
	DDX_Control(pDX, IDC_FILTER_COMBO, m_ctlIncludeMask);
	DDX_Control(pDX, IDC_DATETYPE_COMBO, m_ctlDateType);
	DDX_Control(pDX, IDC_DATE2TYPE_COMBO, m_ctlDateType2);
	DDX_Control(pDX, IDC_DATE2_DATETIMEPICKER, m_ctlDate2);
	DDX_Control(pDX, IDC_DATE1TYPE_COMBO, m_ctlDateType1);
	DDX_Control(pDX, IDC_DATE1_DATETIMEPICKER, m_ctlDate1);
	DDX_Control(pDX, IDC_INCLUDE_MASK_BUTTON, m_btnIncludeMask);
	DDX_Control(pDX, IDC_EXCLUDE_MASK_BUTTON, m_btnExcludeMask);
	DDX_Check(pDX, IDC_ARCHIVE_CHECK, m_iArchive);
	DDX_Check(pDX, IDC_ATTRIBUTES_CHECK, m_bAttributes);
	DDX_Check(pDX, IDC_DATE_CHECK, m_bDate1);
	DDX_Check(pDX, IDC_DATE2_CHECK, m_bDate2);
	DDX_Check(pDX, IDC_DIRECTORY_CHECK, m_iDirectory);
	DDX_Check(pDX, IDC_FILTER_CHECK, m_bFilter);
	DDX_Check(pDX, IDC_HIDDEN_CHECK, m_iHidden);
	DDX_Check(pDX, IDC_READONLY_CHECK, m_iReadOnly);
	DDX_Check(pDX, IDC_SIZE_CHECK, m_bSize);
	DDX_Text(pDX, IDC_SIZE1_EDIT, m_uiSize1);
	DDX_Check(pDX, IDC_SIZE2_CHECK, m_bSize2);
	DDX_Text(pDX, IDC_SIZE2_EDIT, m_uiSize2);
	DDX_Check(pDX, IDC_SYSTEM_CHECK, m_iSystem);
	DDX_Check(pDX, IDC_EXCLUDEMASK_CHECK, m_bExclude);
}


BEGIN_MESSAGE_MAP(CFilterDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CFilterDlg)
	ON_BN_CLICKED(IDC_ATTRIBUTES_CHECK, OnAttributesCheck)
	ON_BN_CLICKED(IDC_DATE_CHECK, OnDateCheck)
	ON_BN_CLICKED(IDC_DATE2_CHECK, OnDate2Check)
	ON_BN_CLICKED(IDC_FILTER_CHECK, OnFilterCheck)
	ON_BN_CLICKED(IDC_SIZE_CHECK, OnSizeCheck)
	ON_BN_CLICKED(IDC_SIZE2_CHECK, OnSize2Check)
	ON_BN_CLICKED(IDC_EXCLUDEMASK_CHECK, OnExcludemaskCheck)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_BUTTON, OnIncludeMaskButton)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_BUTTON, OnExcludeMaskButton)
	ON_BN_CLICKED(IDC_EXCLUDEMASK_CHECK, OnExcludemaskCheck)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME1_DATETIMEPICKER, OnDatetimechangeTime1Datetimepicker)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE1_DATETIMEPICKER, OnDatetimechangeDate1Datetimepicker)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg message handlers

BOOL CFilterDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	// fill the combos with data
	const TCHAR *pszData;
	for (int i=0;i<3;i++)
	{
		pszData=GetResManager().LoadString(IDS_BYTE_STRING+i);
		m_ctlSize1Multi.AddString(pszData);
		m_ctlSize2Multi.AddString(pszData);
	}

	// strings <, <=, ...
	for (int i=0;i<5;i++)
	{
		pszData=GetResManager().LoadString(IDS_LT_STRING+i);
		m_ctlSizeType1.AddString(pszData);
		m_ctlSizeType2.AddString(pszData);
		m_ctlDateType1.AddString(pszData);
		m_ctlDateType2.AddString(pszData);
	}

	for (int i=0;i<3;i++)
	{
		m_ctlDateType.AddString(GetResManager().LoadString(IDS_DATECREATED_STRING+i));
	}

	// copy data from TFileFilter to a dialog - mask
	m_bFilter = m_ffFilter.GetUseMask();

	m_ctlIncludeMask.SetCurSel(m_ctlIncludeMask.AddString(m_ffFilter.GetCombinedMask().c_str()));
	for (int i=0;i<m_astrAddMask.GetSize();i++)
	{
		m_ctlIncludeMask.AddString(m_astrAddMask.GetAt(i));
	}

	m_bExclude = m_ffFilter.GetUseExcludeMask();
	m_ctlExcludeMask.SetCurSel(m_ctlExcludeMask.AddString(m_ffFilter.GetCombinedExcludeMask().c_str()));
	for (int i=0;i<m_astrAddExcludeMask.GetSize();i++)
		m_ctlExcludeMask.AddString(m_astrAddExcludeMask.GetAt(i));

	// size
	m_bSize = m_ffFilter.GetUseSize1();
	m_bSize2 = m_ffFilter.GetUseSize2();

	m_ctlSizeType1.SetCurSel(m_ffFilter.GetSizeType1());
	m_ctlSizeType2.SetCurSel(m_ffFilter.GetSizeType2());
	
	SetSize1(m_ffFilter.GetSize1());
	SetSize2(m_ffFilter.GetSize2());

	m_ctlSpin1.SetRange32(0, INT_MAX);
	m_ctlSpin2.SetRange32(0, INT_MAX);

	// date
	m_bDate1 = m_ffFilter.GetUseDateTime1();
	m_bDate2 = m_ffFilter.GetUseDateTime2();

	m_ctlDateType.SetCurSel(m_ffFilter.GetDateType());

	m_ctlDateType1.SetCurSel(m_ffFilter.GetDateCmpType1());
	m_ctlDateType2.SetCurSel(m_ffFilter.GetDateCmpType2());

	SYSTEMTIME st;
	m_ffFilter.GetDateTime1().GetAsSystemTime(st);
	m_ctlDate1.SendMessage(DTM_SETSYSTEMTIME, m_ffFilter.GetUseDate1() ? (WPARAM)GDT_VALID : (WPARAM)GDT_NONE, (LPARAM)&st);
	m_ffFilter.GetDateTime1().GetAsSystemTime(st);
	m_ctlTime1.SendMessage(DTM_SETSYSTEMTIME, m_ffFilter.GetUseTime1() ? (WPARAM)GDT_VALID : (WPARAM)GDT_NONE, (LPARAM)&st);

	m_ffFilter.GetDateTime2().GetAsSystemTime(st);
	m_ctlDate2.SendMessage(DTM_SETSYSTEMTIME, m_ffFilter.GetUseDate2() ? (WPARAM)GDT_VALID : (WPARAM)GDT_NONE, (LPARAM)&st);
	m_ffFilter.GetDateTime2().GetAsSystemTime(st);
	m_ctlTime2.SendMessage(DTM_SETSYSTEMTIME, m_ffFilter.GetUseTime2() ? (WPARAM)GDT_VALID : (WPARAM)GDT_NONE, (LPARAM)&st);

	// attributes
	m_bAttributes=m_ffFilter.GetUseAttributes();
	m_iArchive=m_ffFilter.GetArchive();
	m_iReadOnly=m_ffFilter.GetReadOnly();
	m_iHidden=m_ffFilter.GetHidden();
	m_iSystem=m_ffFilter.GetSystem();
	m_iDirectory=m_ffFilter.GetDirectory();
	
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

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void CFilterDlg::OnLanguageChanged()
{
	// empty combos
	int iPos[4];
	iPos[0]=m_ctlSize1Multi.GetCurSel();
	iPos[1]=m_ctlSize2Multi.GetCurSel();
	m_ctlSize1Multi.ResetContent();
	m_ctlSize2Multi.ResetContent();

	// fill the combos with data
	const TCHAR *pszData;
	for (int i=0;i<3;i++)
	{
		pszData=GetResManager().LoadString(IDS_BYTE_STRING+i);
		m_ctlSize1Multi.AddString(pszData);
		m_ctlSize2Multi.AddString(pszData);
	}

	// selection
	m_ctlSize1Multi.SetCurSel(iPos[0]);
	m_ctlSize2Multi.SetCurSel(iPos[1]);

	iPos[0]=m_ctlSizeType1.GetCurSel();
	iPos[1]=m_ctlSizeType2.GetCurSel();
	iPos[2]=m_ctlDateType1.GetCurSel();
	iPos[3]=m_ctlDateType2.GetCurSel();

	m_ctlSizeType1.ResetContent();
	m_ctlSizeType2.ResetContent();
	m_ctlDateType1.ResetContent();
	m_ctlDateType2.ResetContent();

	// strings <, <=, ...
	for (int i=0;i<5;i++)
	{
		pszData=GetResManager().LoadString(IDS_LT_STRING+i);
		m_ctlSizeType1.AddString(pszData);
		m_ctlSizeType2.AddString(pszData);
		m_ctlDateType1.AddString(pszData);
		m_ctlDateType2.AddString(pszData);
	}

	m_ctlSizeType1.SetCurSel(iPos[0]);
	m_ctlSizeType2.SetCurSel(iPos[1]);
	m_ctlDateType1.SetCurSel(iPos[2]);
	m_ctlDateType2.SetCurSel(iPos[3]);

	iPos[0]=m_ctlDateType.GetCurSel();
	m_ctlDateType.ResetContent();
	for (int i=0;i<3;i++)
	{
		m_ctlDateType.AddString(GetResManager().LoadString(IDS_DATECREATED_STRING+i));
	}
	m_ctlDateType.SetCurSel(iPos[0]);
}

void CFilterDlg::SetSize1(unsigned __int64 ullSize)
{
	if ((ullSize % 1048576) == 0 && ullSize != 0)
	{
		m_uiSize1=static_cast<UINT>(ullSize/1048576);
		m_ctlSize1Multi.SetCurSel(2);
	}
	else if ((ullSize % 1024) == 0 && ullSize != 0)
	{
		m_uiSize1=static_cast<UINT>(ullSize/1024);
		m_ctlSize1Multi.SetCurSel(1);
	}
	else
	{
		m_uiSize1=static_cast<unsigned int>(ullSize);
		m_ctlSize1Multi.SetCurSel(0);
	}
}

void CFilterDlg::SetSize2(unsigned __int64 ullSize)
{
	if ((ullSize % 1048576) == 0 && ullSize != 0)
	{
		m_uiSize2=static_cast<UINT>(ullSize/1048576);
		m_ctlSize2Multi.SetCurSel(2);
	}
	else if ((ullSize % 1024) == 0 && ullSize != 0)
	{
		m_uiSize2=static_cast<UINT>(ullSize/1024);
		m_ctlSize2Multi.SetCurSel(1);
	}
	else
	{
		m_uiSize2=static_cast<unsigned int>(ullSize);
		m_ctlSize2Multi.SetCurSel(0);
	}
}

void CFilterDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bFilter);
	m_btnIncludeMask.EnableWindow(m_bFilter);

	m_ctlExcludeMask.EnableWindow(m_bExclude);
	m_btnExcludeMask.EnableWindow(m_bExclude);

	// size
	m_ctlSizeType1.EnableWindow(m_bSize);
	m_ctlSizeType2.EnableWindow(m_bSize && m_bSize2);
	GetDlgItem(IDC_SIZE1_EDIT)->EnableWindow(m_bSize);
	GetDlgItem(IDC_SIZE2_EDIT)->EnableWindow(m_bSize && m_bSize2);
	GetDlgItem(IDC_SIZE1_SPIN)->EnableWindow(m_bSize);
	GetDlgItem(IDC_SIZE2_SPIN)->EnableWindow(m_bSize && m_bSize2);
	GetDlgItem(IDC_SIZE2_CHECK)->EnableWindow(m_bSize);
	m_ctlSize1Multi.EnableWindow(m_bSize);
	m_ctlSize2Multi.EnableWindow(m_bSize && m_bSize2);

	// date
	CTime tmTemp;
	bool bSecond=((m_ctlDate1.GetTime(tmTemp) == GDT_VALID) || (m_ctlTime1.GetTime(tmTemp) == GDT_VALID));
	m_ctlDateType.EnableWindow(m_bDate1);
	GetDlgItem(IDC_DATE2_CHECK)->EnableWindow(m_bDate1 && bSecond);
	m_ctlDateType1.EnableWindow(m_bDate1);
	m_ctlDateType2.EnableWindow(m_bDate1 && m_bDate2 && bSecond);
	m_ctlDate1.EnableWindow(m_bDate1);
	m_ctlDate2.EnableWindow(m_bDate1 && m_bDate2 && bSecond);
	m_ctlTime1.EnableWindow(m_bDate1);
	m_ctlTime2.EnableWindow(m_bDate1 && m_bDate2 && bSecond);

	// attrib
	GetDlgItem(IDC_ARCHIVE_CHECK)->EnableWindow(m_bAttributes);
	GetDlgItem(IDC_READONLY_CHECK)->EnableWindow(m_bAttributes);
	GetDlgItem(IDC_HIDDEN_CHECK)->EnableWindow(m_bAttributes);
	GetDlgItem(IDC_SYSTEM_CHECK)->EnableWindow(m_bAttributes);
	GetDlgItem(IDC_DIRECTORY_CHECK)->EnableWindow(m_bAttributes);
}

void CFilterDlg::OnOK() 
{
	UpdateData(TRUE);
	
	// TFileFilter --> dialogu - mask
	CString strText;
	m_ctlIncludeMask.GetWindowText(strText);
	m_ffFilter.SetUseMask(((m_bFilter != 0) && !strText.IsEmpty()));
	m_ffFilter.SetCombinedMask((PCTSTR)strText);

	m_ctlExcludeMask.GetWindowText(strText);
	m_ffFilter.SetUseExcludeMask((m_bExclude != 0) && !strText.IsEmpty());
	m_ffFilter.SetCombinedExcludeMask((PCTSTR)strText);

	// size
	m_ffFilter.SetUseSize1(m_bSize != 0);
	m_ffFilter.SetUseSize2(m_bSize2 != 0);

	m_ffFilter.SetSizeType1((chengine::ECompareType)m_ctlSizeType1.GetCurSel());
	m_ffFilter.SetSizeType2((chengine::ECompareType)m_ctlSizeType2.GetCurSel());

	m_ffFilter.SetSize1(static_cast<unsigned __int64>(m_uiSize1)*static_cast<unsigned __int64>(GetMultiplier(m_ctlSize1Multi.GetCurSel())));
	m_ffFilter.SetSize2(static_cast<unsigned __int64>(m_uiSize2)*static_cast<unsigned __int64>(GetMultiplier(m_ctlSize2Multi.GetCurSel())));

	// date
	m_ffFilter.SetDateType((chengine::TFileFilter::EDateType)m_ctlDateType.GetCurSel());

	m_ffFilter.SetDateCmpType1((chengine::ECompareType)m_ctlDateType1.GetCurSel());
	m_ffFilter.SetDateCmpType2((chengine::ECompareType)m_ctlDateType2.GetCurSel());

	CTime tDate;
	CTime tTime;
	m_ffFilter.SetUseDate1(m_ctlDate1.GetTime(tDate) == GDT_VALID);
	m_ffFilter.SetUseTime1(m_ctlTime1.GetTime(tTime) == GDT_VALID);
	m_ffFilter.SetDateTime1(chengine::TDateTime(tDate.GetYear(), tDate.GetMonth(), tDate.GetDay(), tTime.GetHour(), tTime.GetMinute(), tTime.GetSecond()));
	
	m_ffFilter.SetUseDate2(m_ctlDate2.GetTime(tDate) == GDT_VALID);
	m_ffFilter.SetUseTime2(m_ctlTime2.GetTime(tTime) == GDT_VALID);
	m_ffFilter.SetDateTime2(chengine::TDateTime(tDate.GetYear(), tDate.GetMonth(), tDate.GetDay(), tTime.GetHour(), tTime.GetMinute(), tTime.GetSecond()));

	m_ffFilter.SetUseDateTime1((m_bDate1 != 0) && (m_ffFilter.GetUseDate1() || m_ffFilter.GetUseTime1()));
	m_ffFilter.SetUseDateTime2((m_bDate2 != 0) && (m_ffFilter.GetUseDate2() || m_ffFilter.GetUseTime2()));

	// attributes
	m_ffFilter.SetUseAttributes(m_bAttributes != 0);
	m_ffFilter.SetArchive(m_iArchive);
	m_ffFilter.SetReadOnly(m_iReadOnly);
	m_ffFilter.SetHidden(m_iHidden);
	m_ffFilter.SetSystem(m_iSystem);
	m_ffFilter.SetDirectory(m_iDirectory);

	CLanguageDialog::OnOK();
}

BOOL CFilterDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(HIWORD(wParam) == 0)
	{
		if(LOWORD(wParam) >= ID_POPUP_FILTER_FILE_WILDCARD && LOWORD(wParam) <= ID_POPUP_FILTER_SEPARATOR_CHAR)
		{
			CComboBox& rCombo = m_bTracksIncludeButton ? m_ctlIncludeMask : m_ctlExcludeMask;

			auto iterFnd = m_mapFilterEntries.find(LOWORD(wParam));
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
	}
	return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
}

int CFilterDlg::GetMultiplier(int iIndex)
{
	switch (iIndex)
	{
	case 0:
		return 1;
	case 1:
		return 1024;
	case 2:
		return 1048576;
	default:
		ASSERT(true);	// bad index
		return 1;
	}
}

void CFilterDlg::OnAttributesCheck() 
{
	EnableControls();
}

void CFilterDlg::OnDateCheck() 
{
	EnableControls();
}

void CFilterDlg::OnDate2Check() 
{
	EnableControls();
}

void CFilterDlg::OnFilterCheck() 
{
	EnableControls();
}

void CFilterDlg::OnSizeCheck() 
{
	EnableControls();
}

void CFilterDlg::OnSize2Check() 
{
	EnableControls();
}

void CFilterDlg::OnExcludemaskCheck() 
{
	EnableControls();
}

void CFilterDlg::OnIncludeMaskButton()
{
	m_bTracksIncludeButton = true;

	// set point in which to set menu
	CRect rect;
	GetDlgItem(IDC_INCLUDE_MASK_BUTTON)->GetWindowRect(&rect);

	m_menuFilterType.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.right + 1, rect.top, this);
}

void CFilterDlg::OnExcludeMaskButton()
{
	m_bTracksIncludeButton = false;

	// set point in which to set menu
	CRect rect;
	GetDlgItem(IDC_EXCLUDE_MASK_BUTTON)->GetWindowRect(&rect);

	m_menuFilterType.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.right + 1, rect.top, this);
}

void CFilterDlg::OnDatetimechangeTime1Datetimepicker(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	EnableControls();
	
	*pResult = 0;
}

void CFilterDlg::OnDatetimechangeDate1Datetimepicker(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	EnableControls();
	
	*pResult = 0;
}
