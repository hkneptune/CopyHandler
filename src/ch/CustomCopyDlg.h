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
#pragma once

#include "CDragDropListCtrl.h"
#include "CDragDropComboEx.h"
#include "../libchengine/TTaskDefinition.h"
#include "../libchengine/FeedbackRules.h"

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg dialog

namespace chengine {
	class TFileFilter;
}

class CCustomCopyDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CCustomCopyDlg();   // standard constructor
	explicit CCustomCopyDlg(const chengine::TTaskDefinition& rTaskDefinition);

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnLanguageChanged() override;

	void UpdateFilesListCtrlHeaderWidth();
	void UpdateComboIcon();
	void UpdateFeedbackRulesEdit();

	void EnableControls();
	void AddFilter(const chengine::TFileFilter& rFilter, int iPos=-1);
	void AddPath(CString strPath);

	void SetBuffersizesString();

	bool HasBasicTaskData();
	void UpdateInternalTaskDefinition();

	BOOL OnInitDialog() override;
	afx_msg void OnAddDirectoryButton();
	afx_msg void OnAddFilesButton();
	afx_msg void OnRemoveButton();
	afx_msg void OnBrowseButton();
	afx_msg void OnOK() override;
	afx_msg void OnChangebufferButton();
	afx_msg void OnAddfilterButton();
	afx_msg void OnRemovefilterButton();
	afx_msg void OnDestroy();
	afx_msg void OnDblclkFiltersList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkBuffersizesList();
	afx_msg void OnEditchangeDestpathComboboxex();
	afx_msg void OnImportButton();
	afx_msg void OnIgnorefoldersCheck();
	afx_msg void OnForcedirectoriesCheck();
	afx_msg void OnExcludeEmptyDirectories();
	afx_msg void OnExportButtonClicked();
	afx_msg void OnBnCustomRules();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

public:
	chengine::TTaskDefinition m_tTaskDefinition;
	chengine::FeedbackRules m_rules;
	std::vector<CString> m_vRecent;						// recently selected paths

	CDragDropComboEx	m_ctlDstPath;
	CListCtrl	m_ctlFilters;
	CListBox	m_ctlBufferSizes;
	CComboBox	m_ctlOperation;
	CComboBox	m_ctlPriority;
	CDragDropListCtrl	m_ctlFiles;
	CImageList m_ilImages;
	CEdit m_ctlFeedbackRules;

	bool m_bActualisation = false;	// is this dialog processing the combo text changing ?
	BOOL	m_bOnlyCreate = FALSE;
	BOOL	m_bIgnoreFolders = FALSE;
	BOOL	m_bForceDirectories = FALSE;
	BOOL	m_bExcludeEmptyDirectories = FALSE;
};
