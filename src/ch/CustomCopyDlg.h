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

#include "DataBuffer.h"
#include "FileFilter.h"
#include "../libchcore/TTaskDefinition.h"

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg dialog

class CCustomCopyDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CCustomCopyDlg();   // standard constructor
	CCustomCopyDlg(const chcore::TTaskDefinition& rTaskDefinition);

// Dialog Data
	enum { IDD = IDD_CUSTOM_COPY_DIALOG };

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual void OnLanguageChanged();
	void UpdateDialog();
	void UpdateComboIcon();
	void SetComboPath(LPCTSTR lpszText);
	void EnableControls();
	void AddFilter(const CFileFilter& rFilter, int iPos=-1);
	void AddPath(CString strPath);

	void SetBuffersizesString();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnAddDirectoryButton();
	afx_msg void OnAddFilesButton();
	afx_msg void OnRemoveButton();
	afx_msg void OnBrowseButton();
	virtual void OnOK();
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

	DECLARE_MESSAGE_MAP()

public:
	chcore::TTaskDefinition m_tTaskDefinition;
	std::vector<CString> m_vRecent;						// recently selected paths
	bool m_bActualisation;	// is this dialog processing the combo text changing ?

	CComboBoxEx	m_ctlDstPath;
	CListCtrl	m_ctlFilters;
	CListBox	m_ctlBufferSizes;
	CComboBox	m_ctlOperation;
	CComboBox	m_ctlPriority;
	CListCtrl	m_ctlFiles;
	BOOL	m_bOnlyCreate;
	BOOL	m_bIgnoreFolders;
	BOOL	m_bForceDirectories;
	BOOL	m_bFilters;
	BOOL	m_bAdvanced;
	CImageList m_ilImages;
};

#endif
