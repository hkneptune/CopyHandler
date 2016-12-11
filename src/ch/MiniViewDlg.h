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
#ifndef __MINIVIEWDLG_H__
#define __MINIVIEWDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CMiniViewDlg dialog
#include "ProgressListBox.h"
#include <array>

namespace chengine {
	class TTaskManager;
}

namespace chcore
{
	class TTaskManager;
}

#define BTN_COUNT 5

#define MSG_DRAWBUTTON	1
#define MSG_ONCLICK		2

#define WM_MINIVIEWDBLCLK		WM_USER+14

class CMiniViewDlg : public ictranslate::CLanguageDialog
{
// internal struct
public:
	struct _BTNDATA_
	{
		void (*pfnCallbackFunc)(CMiniViewDlg*, UINT, _BTNDATA_*, CDC*) = nullptr;		// callback - click
		int iPosition = 0;		// button pos counting from right
		bool bPressed = false;		// is it pressed ?
		bool bEnabled = false;		// is it enabled ?

		CRect rcButton;		// filled in OnNCPaint
	};

	std::array<_BTNDATA_, BTN_COUNT> m_bdButtons;

// Construction
public:
	CMiniViewDlg(chengine::TTaskManager* pArray, bool* pbHide, CWnd* pParent = nullptr);   // standard constructor

	void ShowWindow();
	void HideWindow();
	void ResizeDialog();
	friend void OnRestartBtn(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=nullptr);
	friend void OnCancelBtn(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=nullptr);
	friend void OnResume(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=nullptr);
	friend void OnPause(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=nullptr);
	friend void OnCloseBtn(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC);

	void RefreshStatus();
	void RecalcSize(int nHeight, bool bInitial);

	UINT GetLanguageUpdateOptions() override { return LDF_NODIALOGSIZE; }
	void OnLanguageChanged() override;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

	afx_msg HBRUSH OnCtlColor(CDC*, CWnd*, UINT);
	BOOL OnInitDialog() override;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelchangeProgressList();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSetfocusProgressList();
	afx_msg void OnSelcancelProgressList();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnDblclkProgressList();

	DECLARE_MESSAGE_MAP()

public:
	static bool m_bLock;

private:
	// from CMainWnd
	chengine::TTaskManager *m_pTasks;

	CBrush m_brBackground;
	int m_iLastHeight;
	bool m_bShown;

	// cache
	bool m_bActive;

	// lock
	bool *m_pbHide;		// is the big status dialog visible ?

	// in onmousemove points to last pressed button
	int m_iIndex;

	CProgressListBox	m_ctlStatus;
};

#endif
