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
#ifndef __FFLISTCTRL_H__
#define __FFLISTCTRL_H__

/////////////////////////////////////////////////////////////////////////////
// CFFListCtrl window
#define LVN_CHANGEDSELECTION	WM_USER+10

class CFFListCtrl : public CListCtrl
{
// Construction
public:
	CFFListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SendSelChangedToParent();
	void LimitItems(int iLimit);
	virtual ~CFFListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFFListCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg void OnPaint();
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
