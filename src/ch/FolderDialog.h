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
/*************************************************************************
	CFolderDialog dialog control

	Files: FolderDialog.h, FolderDialog.cpp
	Author: Ixen Gerthannes
	Usage:
		1. Construct variable of type CFolderDialog
		2. Fill m_bdData member with appropriate information
		3. Call DoModal()
	Functions:
		void GetPath(CString& rstrPath); - returns chosen path
		void GetPath(LPTSTR pszPath); - returns chosen path
*************************************************************************/
#ifndef __FOLDERDIALOG_H__
#define __FOLDERDIALOG_H__

#include "DirTreeCtrl.h"
#include "ThemedButton.h"
#include "shortcuts.h"
#include "../libictranslate/LanguageDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CFolderDialog dialog

class CFolderDialog : public ictranslate::CLanguageDialog
{
// Construction
public:
	explicit CFolderDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFolderDialog();

public:
	virtual INT_PTR DoModal();

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	virtual BOOL OnTooltipText(UINT uiID, TOOLTIPTEXT* pTip);

	// structure used for passing parameters
	struct BROWSEDATA
	{
		CString strCaption;
		CString strText;
		CString strInitialDir;
		std::vector<CString> cvRecent;
		std::vector<CString> cvShortcuts;

		int cx = 0;
		int cy = 0;		// pixels
		int iView = 2;		// type of view (large icons, small icons, ...)
		bool bExtended = false;	// with the shortcuts or not
		bool bIgnoreDialogs = true;	// if tree ctrl should show shell dialogs in style 'insert floppy'
	} m_bdData;

	// getting path - after dialog exits
	void GetPath(CString& rstrPath);
	void GetPath(LPTSTR pszPath);

	// function displays dialog with some parameters
	friend INT_PTR BrowseForFolder(BROWSEDATA* pData, LPTSTR pszPath);

// Implementation
protected:
	void InitImageList();
	void ApplyExpandState(bool bExpand);
	int CreateControls();
	void SetView(int iView);
	void UpdateComboIcon();
	void SetComboPath(LPCTSTR lpszPath);
	void ResizeControls(int cx, int cy);

	CString m_strTip;			// for tooltip storage
	CString m_strPath;			// for path after dialog exits

	bool m_bIgnoreUpdate;		// ignores nearest edit update (with path)
	bool m_bIgnoreTreeRefresh;	// ignores nearest refreshing of tree ctrl
	HIMAGELIST m_hImages, m_hLargeImages;	// two system image lists - large and small icons

	// button bitmaps
	CImageList m_ilList;		// image list with buttons' images

	// last position in which gripper has been drawn
	CRect m_rcGripper;

	// controls that'll be diaplyed in a dialog
	CStatic					m_ctlTitle;
	CDirTreeCtrl			m_ctlTree;
	CComboBoxEx				m_ctlPath;
	CListCtrl				m_ctlShortcuts;
	CThemedButton			m_ctlLargeIcons;
	CThemedButton			m_ctlSmallIcons;
	CThemedButton			m_ctlList;
	CThemedButton			m_ctlReport;
	CThemedButton			m_ctlNewFolder;
	CButton					m_ctlOk;
	CButton					m_ctlCancel;
	CThemedButton			m_ctlToggle;
	CThemedButton			m_ctlAddShortcut;
	CThemedButton			m_ctlRemoveShortcut;

	// Generated message map functions
	//{{AFX_MSG(CFolderDialog)
	afx_msg void OnEndLabelEditShortcutList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddShortcut();
	afx_msg void OnRemoveShortcut();
	afx_msg void OnToggleButton();
	afx_msg void OnSelchangedFolderTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetInfoTipFolderTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShortcutKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnNewfolderButton();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnPathChanging();
	afx_msg void OnIconsRadio();
	afx_msg void OnSmalliconsRadio();
	afx_msg void OnListRadio();
	afx_msg void OnReportRadio();
	afx_msg void OnItemchangedShortcutList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetShortcutInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
