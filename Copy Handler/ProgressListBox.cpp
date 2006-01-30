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

#include "stdafx.h"
#include "ProgressListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define USE_SMOOTH_PROGRESS

/////////////////////////////////////////////////////////////////////////////
// CProgressListBox

CProgressListBox::CProgressListBox()
{
	m_bShowCaptions=true;
	m_bSmoothProgress=false;
}

CProgressListBox::~CProgressListBox()
{
	for (int i=0;i<m_items.GetSize();i++)
		delete m_items.GetAt(i);

	m_items.RemoveAll();
}


BEGIN_MESSAGE_MAP(CProgressListBox, CListBox)
	//{{AFX_MSG_MAP(CProgressListBox)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_CONTROL_REFLECT(LBN_KILLFOCUS, OnKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressListBox message handlers

void CProgressListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (lpDrawItemStruct->itemID == -1)
		return;
	_PROGRESSITEM_* pItem=m_items.GetAt(lpDrawItemStruct->itemID);

	// device context
	CDC* pDC=CDC::FromHandle(lpDrawItemStruct->hDC);
	pDC->SetBkMode(TRANSPARENT);

	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		// fill with color, because in other way the trash appears
		pDC->FillSolidRect(&lpDrawItemStruct->rcItem, GetSysColor(COLOR_3DFACE));
		CPoint apt[3]={ CPoint(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top+(lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top)/4),
						CPoint(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top+3*((lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top)/4)),
						CPoint(lpDrawItemStruct->rcItem.left+7, lpDrawItemStruct->rcItem.top+(lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top)/2) };
		pDC->Polygon(apt, 3);
		lpDrawItemStruct->rcItem.left+=10;
	}

	// draw text
	if (m_bShowCaptions)
	{
		CRect rcText(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top+2,
			lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.bottom-(9+2));
		pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		pDC->DrawText(pItem->m_strText, &rcText,
			DT_PATH_ELLIPSIS | DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

	// frame sizes
	int iEdgeWidth=1/*GetSystemMetrics(SM_CXEDGE)*/;
	int iEdgeHeight=1/*GetSystemMetrics(SM_CYEDGE)*/;

	// progress like drawing
	int iBoxWidth=static_cast<int>(static_cast<double>(((9+2)-2*iEdgeWidth))*(2.0/3.0))+1;
	CRect rcProgress(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom-(9+2), 
			lpDrawItemStruct->rcItem.left+2*iEdgeWidth+((lpDrawItemStruct->rcItem.right-lpDrawItemStruct->rcItem.left-2*iEdgeWidth)/iBoxWidth)*iBoxWidth,
			lpDrawItemStruct->rcItem.bottom);
	
	// edge
	pDC->Draw3dRect(&rcProgress, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));

	if (!m_bSmoothProgress)
	{
		// boxes within edge
		double dCount=static_cast<int>(static_cast<double>(((rcProgress.Width()-2*iEdgeHeight)/iBoxWidth))
			*(static_cast<double>(pItem->m_uiPos)/static_cast<double>(pItem->m_uiRange)));
		int iBoxCount=((dCount-static_cast<int>(dCount)) > 0.2) ? static_cast<int>(dCount)+1 : static_cast<int>(dCount);
		
		for (int i=0;i<iBoxCount;i++)
			pDC->FillSolidRect(lpDrawItemStruct->rcItem.left+i*iBoxWidth+iEdgeWidth+1,
			lpDrawItemStruct->rcItem.bottom-(9+2)+iEdgeHeight+1,
			iBoxWidth-2, rcProgress.Height()-2*iEdgeHeight-2, pItem->m_crColor);
	}
	else
	{
		pDC->FillSolidRect(lpDrawItemStruct->rcItem.left+iEdgeWidth+1, lpDrawItemStruct->rcItem.bottom-(9+2)+iEdgeHeight+1,
			static_cast<int>((rcProgress.Width()-2*iEdgeHeight-3)*(static_cast<double>(pItem->m_uiPos)/static_cast<double>(pItem->m_uiRange))),
			rcProgress.Height()-2*iEdgeHeight-2, pItem->m_crColor);
	}
}

bool CProgressListBox::GetShowCaptions()
{
	return m_bShowCaptions;
}

void CProgressListBox::SetShowCaptions(bool bShow)
{
	if (bShow != m_bShowCaptions)
	{
		m_bShowCaptions=bShow;
		if (bShow)
		{
			CClientDC dc(this);
			dc.SelectObject(GetFont());
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);
			int iHeight=MulDiv(tm.tmHeight+tm.tmExternalLeading, dc.GetDeviceCaps(LOGPIXELSY), tm.tmDigitizedAspectY);

			SetItemHeight(0, 9+2+2+iHeight);
		}
		else
			SetItemHeight(0, 9+2+2);

		RecalcHeight();
	}
}

void CProgressListBox::RecalcHeight()
{
	// new height
	int iCtlHeight=m_items.GetSize()*GetItemHeight(0);

	// change control size
	CRect rcCtl;
	GetClientRect(&rcCtl);
	this->SetWindowPos(NULL, 0, 0, rcCtl.Width(), iCtlHeight, SWP_NOZORDER | SWP_NOMOVE);
}

void CProgressListBox::Init()
{
	// set new height of an item
	CClientDC dc(this);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int iHeight=MulDiv(tm.tmHeight+tm.tmExternalLeading, dc.GetDeviceCaps(LOGPIXELSY), tm.tmDigitizedAspectY);
	SetItemHeight(0, m_bShowCaptions ? iHeight+2+9+2 : 2+9+2);
}

_PROGRESSITEM_* CProgressListBox::GetItemAddress(int iIndex)
{
	if (m_items.GetSize() > iIndex)
		return m_items.GetAt(iIndex);
	else
	{
		_PROGRESSITEM_* pItem=new _PROGRESSITEM_;
		pItem->m_uiRange=100;
		m_items.Add(pItem);
		return pItem;
	}
}

void CProgressListBox::UpdateItems(int nLimit, bool bUpdateSize)
{
	// delete items from array
	while (m_items.GetSize() > nLimit)
	{
		delete m_items.GetAt(nLimit);
		m_items.RemoveAt(nLimit);
	}

	// change count of elements in a listbox
	if (GetCount() != m_items.GetSize())
	{
		while (GetCount() < m_items.GetSize())
			AddString("");
		
		while (GetCount() > m_items.GetSize())
			DeleteString(m_items.GetSize());
	}

	if (bUpdateSize)
	{
		Invalidate();
		RecalcHeight();
	}
}

void CProgressListBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rcClip;
	dc.GetClipBox(&rcClip);

	CMemDC memDC(&dc, &rcClip);
	memDC.FillSolidRect(&rcClip, GetSysColor(COLOR_3DFACE));

	DefWindowProc(WM_PAINT, reinterpret_cast<WPARAM>(memDC.m_hDC), 0);
}

BOOL CProgressListBox::OnEraseBkgnd(CDC*) 
{
	return FALSE/*CListBox::OnEraseBkgnd(pDC)*/;
}

int CProgressListBox::SetCurSel(int nSelect)
{
	int nResult=static_cast<CListBox*>(this)->SetCurSel(nSelect);
	if (nSelect == -1)
		GetParent()->SendMessage(WM_COMMAND, (LBN_SELCANCEL << 16) | GetDlgCtrlID(), reinterpret_cast<LPARAM>(this->m_hWnd));

	return nResult;
}

void CProgressListBox::OnKillfocus() 
{
	SetCurSel(-1);
}

void CProgressListBox::SetSmoothProgress(bool bSmoothProgress)
{
	m_bSmoothProgress=bSmoothProgress;
}
