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
#include "resource.h"
#include "RuleEditDlg.h"
#include "ch.h"
#include "RuleEditAlreadyExistsDlg.h"
#include "RuleEditErrorDlg.h"
#include "RuleEditNotEnoughSpaceDlg.h"
#include <type_traits>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace string;
using namespace chengine;

namespace
{
	int GetSelectedIndex(CListCtrl& ctlList)
	{
		POSITION pos = ctlList.GetFirstSelectedItemPosition();
		if(pos == nullptr)
			return -1;

		int iItem = ctlList.GetNextSelectedItem(pos);
		return iItem;
	}

	template<class RuleList>
	void OnUpButton(RuleEditDlg* pDlg, CListCtrl& ctlList, RuleList& ruleList, std::function<void(RuleEditDlg*)> reloadList)
	{
		int iItem = GetSelectedIndex(ctlList);
		if(iItem < 1)
			return;

		// store current item
		auto rRule = ruleList.GetAt(iItem);

		// remove current item
		ctlList.DeleteItem(iItem);
		ruleList.RemoveAt(iItem);

		// re-insert it at the right position
		ruleList.InsertAt(iItem - 1, rRule);
		reloadList(pDlg);

		ctlList.SetItemState(iItem - 1, LVIS_SELECTED, LVIS_SELECTED);
	}

	template<class RuleList>
	void OnDownButton(RuleEditDlg* pDlg, CListCtrl& ctlList, RuleList& ruleList, std::function<void(RuleEditDlg*)> reloadList)
	{
		int iItem = GetSelectedIndex(ctlList);
		if(iItem < 0 || iItem + 1 >= ctlList.GetItemCount())
			return;

		// store current item
		auto rRule = ruleList.GetAt(iItem);

		// remove current item
		ctlList.DeleteItem(iItem);
		ruleList.RemoveAt(iItem);

		// re-insert it at the right position
		ruleList.InsertAt(iItem + 1, rRule);
		reloadList(pDlg);

		ctlList.SetItemState(iItem + 1, LVIS_SELECTED, LVIS_SELECTED);
	}

	template<class RuleList>
	void OnRemoveButton(CListCtrl& ctlList, RuleList& ruleList)
	{
		int iItem = GetSelectedIndex(ctlList);
		if(iItem < 0)
			return;

		ctlList.DeleteItem(iItem);
		ruleList.RemoveAt(iItem);
	}
}

RuleEditDlg::RuleEditDlg(const FeedbackRules& rRules) :
	ictranslate::CLanguageDialog(IDD_RULE_EDIT_ALL_DIALOG),
	m_rules(rRules)
{
	m_rules.ResetModifications();
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
	ON_BN_CLICKED(IDC_ALREADY_EXISTS_UP_BUTTON, OnAlreadyExistsUpButton)
	ON_BN_CLICKED(IDC_ALREADY_EXISTS_DOWN_BUTTON, OnAlreadyExistsDownButton)

	ON_BN_CLICKED(IDC_FILE_ERROR_CHANGE_BUTTON, OnErrorChangeButton)
	ON_BN_CLICKED(IDC_FILE_ERROR_ADD_BUTTON, OnErrorAddButton)
	ON_BN_CLICKED(IDC_FILE_ERROR_REMOVE_BUTTON, OnErrorRemoveButton)
	ON_BN_CLICKED(IDC_FILE_ERROR_UP_BUTTON, OnErrorUpButton)
	ON_BN_CLICKED(IDC_FILE_ERROR_DOWN_BUTTON, OnErrorDownButton)

	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_CHANGE_BUTTON, OnNotEnoughSpaceChangeButton)
	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_ADD_BUTTON, OnNotEnoughSpaceAddButton)
	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_REMOVE_BUTTON, OnNotEnoughSpaceRemoveButton)
	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_UP_BUTTON, OnNotEnoughSpaceUpButton)
	ON_BN_CLICKED(IDC_NOT_ENOUGH_SPACE_DOWN_BUTTON, OnNotEnoughSpaceDownButton)

	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_ALREADY_EXISTS_RULES_LIST, OnAlreadyExistsItemChanged)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_ERROR_RULES_LIST, OnErrorItemChanged)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_NOT_ENOUGH_SPACE_RULES_LIST, OnNotEnoughSpaceItemChanged)

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
	AddResizableControl(IDC_ALREADY_EXISTS_UP_BUTTON, 0.0, 0.34, 0.0, 0.0);
	AddResizableControl(IDC_ALREADY_EXISTS_DOWN_BUTTON, 0.0, 0.34, 0.0, 0.0);

	AddResizableControl(IDC_FILE_ERROR_RULES_STATIC, 0.0, 0.34, 1.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_RULES_LIST, 0.0, 0.34, 1.0, 0.33);
	AddResizableControl(IDC_FILE_ERROR_CHANGE_BUTTON, 1.0, 0.67, 0.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_ADD_BUTTON, 1.0, 0.67, 0.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_REMOVE_BUTTON, 1.0, 0.67, 0.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_UP_BUTTON, 0.0, 0.67, 0.0, 0.0);
	AddResizableControl(IDC_FILE_ERROR_DOWN_BUTTON, 0.0, 0.67, 0.0, 0.0);

	AddResizableControl(IDC_NOT_ENOUGH_SPACE_STATIC, 0.0, 0.67, 1.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_RULES_LIST, 0.0, 0.67, 1.0, 0.33);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_CHANGE_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_ADD_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_REMOVE_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_UP_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_NOT_ENOUGH_SPACE_DOWN_BUTTON, 0.0, 1.0, 0.0, 0.0);

	AddResizableControl(IDOK, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDCANCEL, 1.0, 1.0, 0.0, 0.0);

	AddResizableControl(IDC_BOTTOM_BAR_STATIC, 0.0, 1.0, 1.0, 0.0);

	InitializeResizableControls();

	// styles
	m_ctlAlreadyExistsRulesList.SetExtendedStyle(m_ctlAlreadyExistsRulesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ctlErrorRulesList.SetExtendedStyle(m_ctlErrorRulesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ctlNotEnoughSpaceRulesList.SetExtendedStyle(m_ctlNotEnoughSpaceRulesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// init columns in lists
	InitAlreadyExistsColumns();
	InitErrorColumns();
	InitNotEnoughSpaceColumns();

	FillAlreadyExistsList();
	FillErrorList();
	FillNotEnoughSpaceList();

	EnableControls();

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
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem = 0;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(2, &lvc);

	// size
	lvc.iSubItem = 1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRSIZE_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(3, &lvc);

	// time
	lvc.iSubItem = 2;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRDATE_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlAlreadyExistsRulesList.InsertColumn(4, &lvc);

	// attributes
	lvc.iSubItem = 3;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRRESULT_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
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
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem = 0;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(2, &lvc);

	// operation type
	lvc.iSubItem = 1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDROPERATION_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(3, &lvc);

	// system error
	lvc.iSubItem = 2;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRSYSTEMERROR_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
	m_ctlErrorRulesList.InsertColumn(4, &lvc);

	// result
	lvc.iSubItem = 3;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRRESULT_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
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
	lvc.cx = static_cast<int>(0.4 * rc.Width());
	m_ctlNotEnoughSpaceRulesList.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem = 0;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.4 * rc.Width());
	m_ctlNotEnoughSpaceRulesList.InsertColumn(2, &lvc);

	// result
	lvc.iSubItem = 1;
	lvc.pszText = (PTSTR)GetResManager().LoadString(IDS_HDRRESULT_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.2 * rc.Width());
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
	TString strDstFile = GetResManager().LoadString(IDS_DESTINATION_FILE_STRING);

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

	EnableControls();
}

void RuleEditDlg::OnOK() 
{
	UpdateData(TRUE);

	CLanguageDialog::OnOK();
}

void RuleEditDlg::EnableControls()
{
	EnableAlreadyExistsControls();
	EnableErrorControls();
	EnableNotEnoughSpaceControls();
}

void RuleEditDlg::EnableAlreadyExistsControls()
{
	int iCurrentSel = GetSelectedIndex(m_ctlAlreadyExistsRulesList);
	GetDlgItem(IDC_ALREADY_EXISTS_UP_BUTTON)->EnableWindow(iCurrentSel > 0);
	GetDlgItem(IDC_ALREADY_EXISTS_DOWN_BUTTON)->EnableWindow(iCurrentSel >= 0 && iCurrentSel + 1 < m_ctlAlreadyExistsRulesList.GetItemCount());
	GetDlgItem(IDC_ALREADY_EXISTS_CHANGE_BUTTON)->EnableWindow(iCurrentSel >= 0);
	GetDlgItem(IDC_ALREADY_EXISTS_REMOVE_BUTTON)->EnableWindow(iCurrentSel >= 0);
}

void RuleEditDlg::EnableErrorControls()
{
	int iCurrentSel = GetSelectedIndex(m_ctlErrorRulesList);
	GetDlgItem(IDC_FILE_ERROR_UP_BUTTON)->EnableWindow(iCurrentSel > 0);
	GetDlgItem(IDC_FILE_ERROR_DOWN_BUTTON)->EnableWindow(iCurrentSel >= 0 && iCurrentSel + 1 < m_ctlErrorRulesList.GetItemCount());
	GetDlgItem(IDC_FILE_ERROR_CHANGE_BUTTON)->EnableWindow(iCurrentSel >= 0);
	GetDlgItem(IDC_FILE_ERROR_REMOVE_BUTTON)->EnableWindow(iCurrentSel >= 0);
}

void RuleEditDlg::EnableNotEnoughSpaceControls()
{
	int iCurrentSel = GetSelectedIndex(m_ctlNotEnoughSpaceRulesList);
	GetDlgItem(IDC_NOT_ENOUGH_SPACE_UP_BUTTON)->EnableWindow(iCurrentSel > 0);
	GetDlgItem(IDC_NOT_ENOUGH_SPACE_DOWN_BUTTON)->EnableWindow(iCurrentSel >= 0 && iCurrentSel + 1 < m_ctlNotEnoughSpaceRulesList.GetItemCount());
	GetDlgItem(IDC_NOT_ENOUGH_SPACE_CHANGE_BUTTON)->EnableWindow(iCurrentSel >= 0);
	GetDlgItem(IDC_NOT_ENOUGH_SPACE_REMOVE_BUTTON)->EnableWindow(iCurrentSel >= 0);
}

void RuleEditDlg::OnAlreadyExistsItemChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	EnableAlreadyExistsControls();
}

void RuleEditDlg::OnErrorItemChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	EnableErrorControls();
}

void RuleEditDlg::OnNotEnoughSpaceItemChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	EnableNotEnoughSpaceControls();
}

void RuleEditDlg::OnDblclkAlreadyExistsList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iItem = GetSelectedIndex(m_ctlAlreadyExistsRulesList);
	if(iItem < 0)
		OnAlreadyExistsAddButton();
	else
		OnAlreadyExistsChangeButton();
	*pResult = 0;
}

void RuleEditDlg::OnAlreadyExistsChangeButton()
{
	int iItem = GetSelectedIndex(m_ctlAlreadyExistsRulesList);
	if(iItem < 0)
		return;

	FeedbackAlreadyExistsRuleList& rRules = m_rules.GetAlreadyExistsRules();
	auto& rRule = rRules.GetAt(iItem);

	RuleEditAlreadyExistsDlg dlg(rRule);
	if(dlg.DoModal() == IDOK)
	{
		// delete old element
		m_ctlAlreadyExistsRulesList.DeleteItem(iItem);

		rRule = dlg.GetRule();
		AddAlreadyExistsRule(rRule, iItem);
	}

	EnableAlreadyExistsControls();
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
	EnableAlreadyExistsControls();
}

void RuleEditDlg::OnAlreadyExistsRemoveButton()
{
	OnRemoveButton(m_ctlAlreadyExistsRulesList, m_rules.GetAlreadyExistsRules());
	EnableAlreadyExistsControls();
}

void RuleEditDlg::OnAlreadyExistsUpButton()
{
	OnUpButton(this, m_ctlAlreadyExistsRulesList, m_rules.GetAlreadyExistsRules(), &RuleEditDlg::FillAlreadyExistsList);
	EnableAlreadyExistsControls();
}

void RuleEditDlg::OnAlreadyExistsDownButton()
{
	OnDownButton(this, m_ctlAlreadyExistsRulesList, m_rules.GetAlreadyExistsRules(), &RuleEditDlg::FillAlreadyExistsList);
	EnableAlreadyExistsControls();
}

void RuleEditDlg::OnDblclkErrorList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iItem = GetSelectedIndex(m_ctlAlreadyExistsRulesList);
	if(iItem < 0)
		OnErrorAddButton();
	else
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

		RuleEditErrorDlg dlg(rRule);
		if(dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlErrorRulesList.DeleteItem(iItem);

			rRule = dlg.GetRule();
			AddErrorRule(rRule, iItem);
		}
	}
	EnableErrorControls();
}

void RuleEditDlg::OnErrorAddButton()
{
	FeedbackErrorRule newRule;

	RuleEditErrorDlg dlg(newRule);
	if(dlg.DoModal() == IDOK)
	{
		const auto& rRule = dlg.GetRule();
		m_rules.GetErrorRules().Add(rRule);
		AddErrorRule(rRule, -1);
	}
	EnableErrorControls();
}

void RuleEditDlg::OnErrorRemoveButton()
{
	OnRemoveButton(m_ctlErrorRulesList, m_rules.GetErrorRules());
	EnableErrorControls();
}

void RuleEditDlg::OnErrorUpButton()
{
	OnUpButton(this, m_ctlErrorRulesList, m_rules.GetErrorRules(), &RuleEditDlg::FillErrorList);
	EnableErrorControls();
}

void RuleEditDlg::OnErrorDownButton()
{
	OnDownButton(this, m_ctlErrorRulesList, m_rules.GetErrorRules(), &RuleEditDlg::FillErrorList);
	EnableErrorControls();
}

void RuleEditDlg::OnDblclkNotEnoughSpaceList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iItem = GetSelectedIndex(m_ctlAlreadyExistsRulesList);
	if(iItem < 0)
		OnNotEnoughSpaceAddButton();
	else
		OnNotEnoughSpaceChangeButton();
	*pResult = 0;
}

void RuleEditDlg::OnNotEnoughSpaceChangeButton()
{
	POSITION pos = m_ctlNotEnoughSpaceRulesList.GetFirstSelectedItemPosition();
	if(pos != nullptr)
	{
		FeedbackNotEnoughSpaceRuleList& rRules = m_rules.GetNotEnoughSpaceRules();

		int iItem = m_ctlNotEnoughSpaceRulesList.GetNextSelectedItem(pos);
		if(iItem < 0)
			return;

		auto& rRule = rRules.GetAt(iItem);

		RuleEditNotEnoughSpaceDlg dlg(rRule);
		if(dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlNotEnoughSpaceRulesList.DeleteItem(iItem);

			rRule = dlg.GetRule();
			AddNotEnoughSpaceRule(rRule, iItem);
		}
	}
	EnableNotEnoughSpaceControls();
}

void RuleEditDlg::OnNotEnoughSpaceAddButton()
{
	FeedbackNotEnoughSpaceRule newRule;

	RuleEditNotEnoughSpaceDlg dlg(newRule);
	if(dlg.DoModal() == IDOK)
	{
		const auto& rRule = dlg.GetRule();
		m_rules.GetNotEnoughSpaceRules().Add(rRule);
		AddNotEnoughSpaceRule(rRule, -1);
	}
	EnableNotEnoughSpaceControls();
}

void RuleEditDlg::OnNotEnoughSpaceRemoveButton()
{
	OnRemoveButton(m_ctlNotEnoughSpaceRulesList, m_rules.GetNotEnoughSpaceRules());
	EnableNotEnoughSpaceControls();
}

void RuleEditDlg::OnNotEnoughSpaceUpButton()
{
	OnUpButton(this, m_ctlNotEnoughSpaceRulesList, m_rules.GetNotEnoughSpaceRules(), &RuleEditDlg::FillNotEnoughSpaceList);
	EnableNotEnoughSpaceControls();
}

void RuleEditDlg::OnNotEnoughSpaceDownButton()
{
	OnDownButton(this, m_ctlNotEnoughSpaceRulesList, m_rules.GetNotEnoughSpaceRules(), &RuleEditDlg::FillNotEnoughSpaceList);
	EnableNotEnoughSpaceControls();
}
