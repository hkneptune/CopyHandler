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
#ifndef __CUSTOMCOPYDLG_H__
#define __CUSTOMCOPYDLG_H__

#include "../libchcore/TFileFilter.h"
#include "../libchcore/TTaskDefinition.h"
#include "CDragDropListCtrl.h"
#include "CDragDropComboEx.h"

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg dialog

class CCustomCopyDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CCustomCopyDlg();   // standard constructor
	explicit CCustomCopyDlg(const chcore::TTaskDefinition& rTaskDefinition);

// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

// Implementation
protected:
	void OnLanguageChanged() override;

	void UpdateFilesListCtrlHeaderWidth();

	void UpdateComboIcon();
	void EnableControls();
	void AddFilter(const chcore::TFileFilter& rFilter, int iPos=-1);
	void AddPath(CString strPath);

	void SetBuffersizesString();

	bool HasBasicTaskData();
	void UpdateInternalTaskDefinition();

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void OnAddDirectoryButton();
	afx_msg void OnAddFilesButton();
	afx_msg void OnRemoveButton();
	afx_msg void OnBrowseButton();
	void OnOK() override;
	afx_msg void OnChangebufferButton();
	afx_msg void OnAddfilterButton();
	afx_msg void OnRemovefilterButton();
	afx_msg void OnDestroy();
	afx_msg void OnFiltersCheck();
	afx_msg void OnStandardCheck();
	afx_msg void OnAdvancedCheck();
	afx_msg void OnDblclkFiltersList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkBuffersizesList();
	afx_msg void OnEditchangeDestpathComboboxex();
	afx_msg void OnImportButton();
	afx_msg void OnIgnorefoldersCheck();
	afx_msg void OnForcedirectoriesCheck();
	afx_msg void OnExportButtonClicked();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

public:
	chcore::TTaskDefinition m_tTaskDefinition;
	std::vector<CString> m_vRecent;						// recently selected paths

	CDragDropComboEx	m_ctlDstPath;
	CListCtrl	m_ctlFilters;
	CListBox	m_ctlBufferSizes;
	CComboBox	m_ctlOperation;
	CComboBox	m_ctlPriority;
	CDragDropListCtrl	m_ctlFiles;
	CImageList m_ilImages;

	bool m_bActualisation = false;	// is this dialog processing the combo text changing ?
	BOOL	m_bOnlyCreate = FALSE;
	BOOL	m_bIgnoreFolders = FALSE;
	BOOL	m_bForceDirectories = FALSE;
	BOOL	m_bFilters = FALSE;
	BOOL	m_bAdvanced = FALSE;
};

#endif
