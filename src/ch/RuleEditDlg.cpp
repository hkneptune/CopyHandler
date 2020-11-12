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
#include "stdafx.h"
#include "resource.h"
#include "RuleEditDlg.h"
#include "ch.h"
#include "RuleEditAlreadyExistsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace string;
using namespace chengine;

RuleEditDlg::RuleEditDlg(const FeedbackRules& rRules) :
	ictranslate::CLanguageDialog(IDD_RULE_EDIT_ALL_DIALOG),
	m_rules(rRules)
{
}

void RuleEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILE_ALREADY_EXISTS_RULES_LIST, m_ctlAlreadyExistsRulesList);
	DDX_Control(pDX, IDC_FILE_ERROR_RULES_LIST, m_ctlErrorRulesList);
	DDX_Control(pDX, IDC_NOT_ENOUGH_SPACE_RULES_LIST, m_ctlNotEnoughSpaceRulesList);
}

BEGIN_MESSAGE_MAP(RuleEditDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_ALREADY_EXISTS_CHANGE_BUTTON, OnAlreadyExistsChangeButton)
	ON_BN_CLICKED(IDC_ALREADY_EXISTS_ADD_BUTTON, OnAlreadyExistsAddButton)
	ON_BN_CLICKED(IDC_ALREADY_EXISTS_REMOVE_BUTTON, OnAlreadyExistsRemoveButton)

	ON_BN_CLICKED(IDC_FILE_ERROR_CHANGE_BUTTON, OnErrorChangeButton)
	ON_BN_CLICKED(IDC_FILE_ERROR_ADD_BUTTON, OnErrorAddButton)
	ON_BN_CLICKED(IDC_FILE_ERROR_REMOVE_BUTTON, OnErrorRemoveButton)

	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_CHANGE_BUTTON, OnNotEnoughSpaceChangeButton)
	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_ADD_BUTTON, OnNotEnoughSpaceAddButton)
	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_REMOVE_BUTTON, OnNotEnoughSpaceRemoveButton)

	ON_NOTIFY(NM_DBLCLK, IDC_FILE_ALREADY_EXISTS_RULES_LIST, OnDblclkAlreadyExistsList)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_ERROR_RULES_LIST, OnDblclkErrorList)
	ON_NOTIFY(NM_DBLCLK, IDC_NOT_ENOUGH_SPACE_RULES_LIST, OnDblclkNotEnoughSpaceList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RuleEditDlg message handlers
BOOL RuleEditDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	AddResizableControl(IDC_FILE_ALREADY_EXISTS_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_FILE_ALREADY_EXISTS_RULES_LIST, 0.0, 0.0, 1.0, 0.34);
	AddResizableControl(IDC_ALREADY_EXISTS_CHANGE_BUTTON, 1.0, 0.34, 0.0, 0.0);
	AddResizableControl(IDC_ALREADY_EXISTS_ADD_BUTTON, 1.0, 0.34, 0.0, 0.0);
	AddResizableControl(IDC_ALREADY_EXISTS_REMOVE_BUTTON, 1.0, 0.34, 0.0, 0.0);

	AddResizableControl(IDC_FILE_ERROR_RULES_STATIC, 0.0, 0.34, 1.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_RULES_LIST, 0.0, 0.34, 1.0, 0.33);
	AddResizableControl(IDC_FILE_ERROR_CHANGE_BUTTON, 1.0, 0.67, 0.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_ADD_BUTTON, 1.0, 0.67, 0.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_REMOVE_BUTTON, 1.0, 0.67, 0.0, 0.0);

	AddResizableControl(IDC_NOT_ENOUGH_SPACE_STATIC, 0.0, 0.67, 1.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_RULES_LIST, 0.0, 0.67, 1.0, 0.33);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_CHANGE_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_ADD_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_REMOVE_BUTTON, 1.0, 1.0, 0.0, 0.0);

	AddResizableControl(IDOK, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDCANCEL, 1.0, 1.0, 0.0, 0.0);

	InitializeResizableControls();

	// styles
	m_ctlAlreadyExistsRulesList.SetExtendedStyle(m_ctlAlreadyExistsRulesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ctlErrorRulesList.SetExtendedStyle(m_ctlErrorRulesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ctlNotEnoughSpaceRulesList.SetExtendedStyle(m_ctlNotEnoughSpaceRulesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// init columns in lists
	InitAlreadyExistsColumns();
	InitErrorColumns();
	InitNotEnoughSpaceColumns();

	UpdateData(FALSE);

	return TRUE;
}

void RuleEditDlg::InitAlreadyExistsColumns()
{
	CRect rc;
	m_ctlAlreadyExistsRulesList.GetWindowRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXEDGE) * 2;

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;

	// mask
	lvc.iSubItem = -1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem = 0;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(2, &lvc);

	// size
	lvc.iSubItem = 1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRSIZE_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.3 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(3, &lvc);

	// time
	lvc.iSubItem = 2;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRDATE_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.3 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(4, &lvc);

	// attributes
	lvc.iSubItem = 3;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRRESULT_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.1 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(5, &lvc);
}

void RuleEditDlg::InitErrorColumns()
{
	CRect rc;
	m_ctlErrorRulesList.GetWindowRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXEDGE) * 2;

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;

	// mask
	lvc.iSubItem = -1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem = 0;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(2, &lvc);

	// operation type
	lvc.iSubItem = 1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDROPERATION_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.3 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(3, &lvc);

	// system error
	lvc.iSubItem = 2;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRSYSTEMERROR_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.3 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(4, &lvc);

	// result
	lvc.iSubItem = 3;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRRESULT_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.1 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(5, &lvc);
}

void RuleEditDlg::InitNotEnoughSpaceColumns()
{
	CRect rc;
	m_ctlNotEnoughSpaceRulesList.GetWindowRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXEDGE) * 2;

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;

	// mask
	lvc.iSubItem = -1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15 * rc.Width());
	m_ctlNotEnoughSpaceRulesList.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem = 0;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15 * rc.Width());
	m_ctlNotEnoughSpaceRulesList.InsertColumn(2, &lvc);

	// result
	lvc.iSubItem = 1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRRESULT_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.1 * rc.Width());
	m_ctlNotEnoughSpaceRulesList.InsertColumn(5, &lvc);
}

void RuleEditDlg::FillAlreadyExistsList()
{
	const auto& rRules = m_rules.GetAlreadyExistsRules();

	m_ctlAlreadyExistsRulesList.DeleteAllItems();

	for(size_t stIndex = 0; stIndex < rRules.GetCount(); ++stIndex)
	{
		const auto& rRule = rRules.GetAt(stIndex);
		AddAlreadyExistsRule(rRule, boost::numeric_cast<int>(stIndex));
	}
}

void RuleEditDlg::FillErrorList()
{
	const auto& rRules = m_rules.GetErrorRules();

	m_ctlErrorRulesList.DeleteAllItems();

	for(size_t stIndex = 0; stIndex < rRules.GetCount(); ++stIndex)
	{
		const auto& rRule = rRules.GetAt(stIndex);
		AddErrorRule(rRule, boost::numeric_cast<int>(stIndex));
	}
}

void RuleEditDlg::FillNotEnoughSpaceList()
{
	const auto& rRules = m_rules.GetNotEnoughSpaceRules();

	m_ctlNotEnoughSpaceRulesList.DeleteAllItems();

	for(size_t stIndex = 0; stIndex < rRules.GetCount(); ++stIndex)
	{
		const auto& rRule = rRules.GetAt(stIndex);
		AddNotEnoughSpaceRule(rRule, boost::numeric_cast<int>(stIndex));
	}
}

void RuleEditDlg::AddAlreadyExistsRule(const FeedbackAlreadyExistsRule& rRule, int iPos)
{
	LVITEM lvi;
	CString strLoaded;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = (iPos == -1) ? m_ctlAlreadyExistsRulesList.GetItemCount() : iPos;

	/////////////////////
	lvi.iSubItem = 0;

	if(rRule.GetUseMask())
	{
		string::TString strData = rRule.GetCombinedMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlAlreadyExistsRulesList.InsertItem(&lvi);

	/////////////////////
	lvi.iSubItem = 1;

	if(rRule.GetUseExcludeMask())
	{
		string::TString strData = rRule.GetCombinedExcludeMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlAlreadyExistsRulesList.SetItem(&lvi);

	/////////////////
	lvi.iSubItem = 2;

	TString strSrcFile = GetResManager().LoadString(IDS_SOURCE_FILE_STRING);
	TString strDstFile = GetResManager().LoadString(IDS_SOURCE_FILE_STRING);

	if(rRule.GetUseSizeCompare())
	{
		TString strOperator = GetResManager().LoadString(IDS_LT_STRING + rRule.GetSizeCompareType());

		strLoaded.Format(_T("%s %s %s"), strSrcFile.c_str(), strOperator.c_str(), strDstFile.c_str());
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlAlreadyExistsRulesList.SetItem(&lvi);

	///////////////////
	lvi.iSubItem = 3;

	if(rRule.GetUseDateCompare())
	{
		TString strOperator = GetResManager().LoadString(IDS_LT_STRING + rRule.GetDateCompareType());

		strLoaded.Format(_T("%s %s %s"), strSrcFile.c_str(), strOperator.c_str(), strDstFile.c_str());
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlAlreadyExistsRulesList.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem = 4;
	strLoaded.Empty();

	strLoaded = GetResManager().LoadString(IDS_FEEDBACK_RESPONSE_UNKNOWN + rRule.GetResult());

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlAlreadyExistsRulesList.SetItem(&lvi);
}

void RuleEditDlg::AddErrorRule(const FeedbackErrorRule& rRule, int iPos)
{
	LVITEM lvi;
	CString strLoaded;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = (iPos == -1) ? m_ctlErrorRulesList.GetItemCount() : iPos;

	/////////////////////
	lvi.iSubItem = 0;

	if(rRule.GetUseMask())
	{
		string::TString strData = rRule.GetCombinedMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlErrorRulesList.InsertItem(&lvi);

	/////////////////////
	lvi.iSubItem = 1;

	if(rRule.GetUseExcludeMask())
	{
		string::TString strData = rRule.GetCombinedExcludeMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlErrorRulesList.SetItem(&lvi);

	/////////////////
	lvi.iSubItem = 2;

	TString strSrcFile = GetResManager().LoadString(IDS_SOURCE_FILE_STRING);
	TString strDstFile = GetResManager().LoadString(IDS_SOURCE_FILE_STRING);

	if(rRule.GetUseErrorType())
		strLoaded = GetResManager().LoadString(IDS_OPERATION_DELETEERROR + rRule.GetErrorType());
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlErrorRulesList.SetItem(&lvi);

	///////////////////
	lvi.iSubItem = 3;

	if(rRule.GetUseSystemErrorNo())
		strLoaded.Format(_T("%u"), rRule.GetSystemErrorNo());
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlErrorRulesList.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem = 4;
	strLoaded.Empty();

	strLoaded = GetResManager().LoadString(IDS_FEEDBACK_RESPONSE_UNKNOWN + rRule.GetResult());

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlErrorRulesList.SetItem(&lvi);
}

void RuleEditDlg::AddNotEnoughSpaceRule(const FeedbackNotEnoughSpaceRule& rRule, int iPos)
{
	LVITEM lvi;
	CString strLoaded;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = (iPos == -1) ? m_ctlNotEnoughSpaceRulesList.GetItemCount() : iPos;

	/////////////////////
	lvi.iSubItem = 0;

	if(rRule.GetUseMask())
	{
		string::TString strData = rRule.GetCombinedMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlNotEnoughSpaceRulesList.InsertItem(&lvi);

	/////////////////////
	lvi.iSubItem = 1;

	if(rRule.GetUseExcludeMask())
	{
		string::TString strData = rRule.GetCombinedExcludeMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = L"";

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlNotEnoughSpaceRulesList.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem = 2;
	strLoaded.Empty();

	strLoaded = GetResManager().LoadString(IDS_FEEDBACK_RESPONSE_UNKNOWN + rRule.GetResult());

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlNotEnoughSpaceRulesList.SetItem(&lvi);
}

void RuleEditDlg::OnLanguageChanged()
{
	UpdateData(TRUE);

	m_ctlAlreadyExistsRulesList.DeleteAllItems();
	m_ctlErrorRulesList.DeleteAllItems();
	m_ctlNotEnoughSpaceRulesList.DeleteAllItems();

	while(m_ctlAlreadyExistsRulesList.DeleteColumn(0));
	while(m_ctlErrorRulesList.DeleteColumn(0));
	while(m_ctlNotEnoughSpaceRulesList.DeleteColumn(0));

	InitAlreadyExistsColumns();
	InitErrorColumns();
	InitNotEnoughSpaceColumns();

	// refresh the entries in filters' list
	FillAlreadyExistsList();
	FillErrorList();
	FillNotEnoughSpaceList();
}

void RuleEditDlg::OnOK() 
{
	UpdateData(TRUE);

	CLanguageDialog::OnOK();
}

void RuleEditDlg::OnDblclkAlreadyExistsList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnAlreadyExistsChangeButton();
	*pResult = 0;
}

void RuleEditDlg::OnAlreadyExistsChangeButton()
{
	POSITION pos = m_ctlAlreadyExistsRulesList.GetFirstSelectedItemPosition();
	if(pos != nullptr)
	{
		FeedbackAlreadyExistsRuleList& rRules = m_rules.GetAlreadyExistsRules();

		int iItem = m_ctlAlreadyExistsRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		auto& rRule = rRules.GetAt(iItem);

		RuleEditAlreadyExistsDlg dlg(rRule);
		if(dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlAlreadyExistsRulesList.DeleteItem(iItem);

			rRule = dlg.GetRule();
			AddAlreadyExistsRule(rRule, iItem);
		}
	}
}

void RuleEditDlg::OnAlreadyExistsAddButton()
{
	FeedbackAlreadyExistsRule newRule;

	RuleEditAlreadyExistsDlg dlg(newRule);
	if(dlg.DoModal() == IDOK)
	{
		const auto& rRule = dlg.GetRule();
		m_rules.GetAlreadyExistsRules().Add(rRule);
		AddAlreadyExistsRule(rRule, -1);
	}
}

void RuleEditDlg::OnAlreadyExistsRemoveButton()
{
	FeedbackAlreadyExistsRuleList& rRules = m_rules.GetAlreadyExistsRules();

	while(true)
	{
		POSITION pos = m_ctlAlreadyExistsRulesList.GetFirstSelectedItemPosition();
		if(pos == nullptr)
			break;

		int iItem = m_ctlAlreadyExistsRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		m_ctlAlreadyExistsRulesList.DeleteItem(iItem);
		rRules.RemoveAt(iItem);
	}
}

void RuleEditDlg::OnDblclkErrorList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnErrorChangeButton();
	*pResult = 0;
}

void RuleEditDlg::OnErrorChangeButton()
{
	POSITION pos = m_ctlErrorRulesList.GetFirstSelectedItemPosition();
	if(pos != nullptr)
	{
		FeedbackErrorRuleList& rRules = m_rules.GetErrorRules();

		int iItem = m_ctlErrorRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		auto& rRule = rRules.GetAt(iItem);
		rRule;

/*
		RuleEditErrorDlg dlg(rRule);
		if(dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlErrorRulesList.DeleteItem(iItem);

			rRule = dlg.GetRule();
			AddErrorRule(rRule, iItem);
		}
*/
	}
}

void RuleEditDlg::OnErrorAddButton()
{
	FeedbackErrorRule newRule;

/*
	RuleEditErrorDlg dlg(newRule);
	if(dlg.DoModal() == IDOK)
	{
		const auto& rRule = dlg.GetRule();
		m_rules.GetErrorRules().Add(rRule);
		AddErrorRule(rRule, -1);
	}
*/
}

void RuleEditDlg::OnErrorRemoveButton()
{
	FeedbackErrorRuleList& rRules = m_rules.GetErrorRules();

	while(true)
	{
		POSITION pos = m_ctlErrorRulesList.GetFirstSelectedItemPosition();
		if(pos == nullptr)
			break;

		int iItem = m_ctlErrorRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		m_ctlErrorRulesList.DeleteItem(iItem);
		rRules.RemoveAt(iItem);
	}
}

void RuleEditDlg::OnDblclkNotEnoughSpaceList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnNotEnoughSpaceChangeButton();
	*pResult = 0;
}

void RuleEditDlg::OnNotEnoughSpaceChangeButton()
{
	POSITION pos = m_ctlNotEnoughSpaceRulesList.GetFirstSelectedItemPosition();
	if(pos != nullptr)
	{
		FeedbackErrorRuleList& rRules = m_rules.GetErrorRules();

		int iItem = m_ctlNotEnoughSpaceRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		auto& rRule = rRules.GetAt(iItem);
		rRule;
/*
		RuleEditNotEnoughSpaceDlg dlg(rRule);
		if(dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlNotEnoughSpaceRulesList.DeleteItem(iItem);

			rRule = dlg.GetRule();
			AddNotEnoughSpaceRule(rRule, iItem);
		}
*/
	}
}

void RuleEditDlg::OnNotEnoughSpaceAddButton()
{
	FeedbackErrorRule newRule;

/*
	RuleEditNotEnoughSpaceDlg dlg(newRule);
	if(dlg.DoModal() == IDOK)
	{
		const auto& rRule = dlg.GetRule();
		m_rules.GetNotEnoughSpaceRules().Add(rRule);
		AddNotEnoughSpaceRule(rRule, -1);
	}
*/
}

void RuleEditDlg::OnNotEnoughSpaceRemoveButton()
{
	FeedbackNotEnoughSpaceRuleList& rRules = m_rules.GetNotEnoughSpaceRules();

	while(true)
	{
		POSITION pos = m_ctlNotEnoughSpaceRulesList.GetFirstSelectedItemPosition();
		if(pos == nullptr)
			break;

		int iItem = m_ctlNotEnoughSpaceRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		m_ctlNotEnoughSpaceRulesList.DeleteItem(iItem);
		rRules.RemoveAt(iItem);
	}
}
