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
#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__

#include "PropertyListCtrl.h"

#define WM_CONFIGNOTIFY		WM_USER+13

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

class COptionsDlg : public ictranslate::CLanguageDialog
{
public:
	explicit COptionsDlg(CWnd* pParent = nullptr);   // standard constructor

	void OnLanguageChanged() override;

	void SendClosingNotify();

	friend void CustomPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex);
	friend void ShortcutsPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex);
	friend void RecentPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex);

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

private:
	void FillPropertyList();
	void ApplyProperties();

	int GetIndexProp(int iPosition);
	CString GetStringProp(int iPosition);
	UINT GetUintProp(int iPosition);
	bool GetBoolProp(int iPosition);
	CString MakeCompoundString(UINT uiBase, int iCount, LPCTSTR lpszSeparator);

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	afx_msg void OnApplyButton();

	DECLARE_MESSAGE_MAP()

private:
	logger::TLoggerPtr m_spLog;
	static bool m_bLock;				// locker

	std::vector<CString> m_cvRecent;
	std::vector<CString> m_cvShortcuts;

	// for languages
	vector<ictranslate::CLangData> m_vld;

	CPropertyListCtrl m_ctlProperties;
};

#endif
