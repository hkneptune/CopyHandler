/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#ifndef __PROGRESSLISTBOX_H__
#define __PROGRESSLISTBOX_H__

#include "memdc.h"
#include "afxtempl.h"
#include "structs.h"

/////////////////////////////////////////////////////////////////////////////
// CProgressListBox window
struct _PROGRESSITEM_
{
	CString m_strText;
	
	UINT m_uiPos;
	UINT m_uiRange;

	COLORREF m_crColor;

	CTask* m_pTask;
};

class CProgressListBox : public CListBox
{
// Construction
public:
	CProgressListBox();

// Attributes
public:

// Operations
public:
	CArray<_PROGRESSITEM_*, _PROGRESSITEM_*> m_items;

protected:
	bool m_bShowCaptions;
	bool m_bSmoothProgress;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetSmoothProgress(bool bSmoothProgress);
	int SetCurSel( int nSelect );
	void Init();

	void UpdateItems(int nLimit, bool bUpdateSize);		// updates items in listbox
	void RecalcHeight();	// sets size of a listbox by counting szie of the items

	_PROGRESSITEM_* GetItemAddress(int iIndex);

	void SetShowCaptions(bool bShow=true);
	bool GetShowCaptions();

	virtual ~CProgressListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProgressListBox)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg void OnKillfocus();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
