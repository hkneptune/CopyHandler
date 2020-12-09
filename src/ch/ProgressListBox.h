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

#include "../libchengine/TaskID.h"

constexpr int WM_TASK_RCLICK = WM_USER + 13;

/////////////////////////////////////////////////////////////////////////////
// CProgressListBox window
struct _PROGRESSITEM_
{
	CString m_strText;

	UINT m_uiPos;
	UINT m_uiRange;

	COLORREF m_crColor;

	chengine::taskid_t m_tTaskID;
};

struct TASK_CLICK_NOTIFICATION
{
	CPoint point;
	chengine::taskid_t taskId = chengine::NoTaskID;
};

class CProgressListBox : public CListBox
{
public:
	CProgressListBox();
	virtual ~CProgressListBox();

public:
	void SetSmoothProgress(bool bSmoothProgress);
	int SetCurrentSelection(int nSelect);
	chengine::taskid_t GetSelectedTaskId() const;

	_PROGRESSITEM_* GetItemAddress(int iIndex);
	std::vector<_PROGRESSITEM_*>& GetItems() { return m_vItems; }

	void UpdateItems(int nLimit, bool bUpdateSize);		// updates items in listbox

	void Init();

	void SetShowCaptions(bool bShow = true);
	bool GetShowCaptions();

protected:
	void RecalcHeight();	// sets size of a listbox by counting size of the items

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg void OnKillfocus();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

private:
	std::vector<_PROGRESSITEM_*> m_vItems;
	bool m_bShowCaptions = true;
	bool m_bSmoothProgress = false;

	int m_iTopMargin = 5;
	int m_iProgressHeight = 10;
	int m_iMidMargin = 2;
	int m_iBottomMargin = 0;
	int m_iProgressContentXMargin = 2;
	int m_iProgressContentYMargin = 2;
};
