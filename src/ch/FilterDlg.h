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

#include "../libchengine/TFileFilter.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

class CFilterDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CFilterDlg();   // standard constructor

protected:
	void OnLanguageChanged() override;
	int GetMultiplier(int iIndex);
	void EnableControls();
	void SetSize1(unsigned __int64 ullSize);
	void SetSize2(unsigned __int64 ullSize);

	void DoDataExchange(CDataExchange* pDX) override;

	BOOL OnInitDialog() override;
	void OnOK() override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

	afx_msg void OnAttributesCheck();
	afx_msg void OnDateCheck();
	afx_msg void OnDate2Check();
	afx_msg void OnFilterCheck();
	afx_msg void OnSizeCheck();
	afx_msg void OnSize2Check();
	afx_msg void OnExcludemaskCheck();
	afx_msg void OnIncludeMaskButton();
	afx_msg void OnExcludeMaskButton();
	afx_msg void OnDatetimechangeTime1Datetimepicker(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeDate1Datetimepicker(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

public:
	CStringArray m_astrAddMask;
	CStringArray m_astrAddExcludeMask;
	chengine::TFileFilter m_ffFilter;

private:
	CComboBox	m_ctlIncludeMask;
	CComboBox	m_ctlExcludeMask;
	CSpinButtonCtrl	m_ctlSpin2;
	CSpinButtonCtrl	m_ctlSpin1;
	CDateTimeCtrl	m_ctlTime2;
	CDateTimeCtrl	m_ctlTime1;
	CComboBox	m_ctlSizeType2;
	CComboBox	m_ctlSizeType1;
	CComboBox	m_ctlSize2Multi;
	CComboBox	m_ctlSize1Multi;
	CComboBox	m_ctlDateType;
	CComboBox	m_ctlDateType2;
	CDateTimeCtrl	m_ctlDate2;
	CComboBox	m_ctlDateType1;
	CDateTimeCtrl	m_ctlDate1;
	CMFCButton m_btnIncludeMask;
	CMFCButton m_btnExcludeMask;

	bool m_bTracksIncludeButton = false;
	CMenu m_menuFilterType;
	std::map<int, std::wstring> m_mapFilterEntries;

	int		m_iArchive;
	BOOL	m_bAttributes;
	BOOL	m_bDate1;
	BOOL	m_bDate2;
	int		m_iDirectory;
	BOOL	m_bFilter;
	int		m_iHidden;
	int		m_iReadOnly;
	BOOL	m_bSize;
	UINT	m_uiSize1;
	BOOL	m_bSize2;
	UINT	m_uiSize2;
	int		m_iSystem;
	BOOL	m_bExclude;
};
