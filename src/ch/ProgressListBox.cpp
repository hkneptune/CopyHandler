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
#include "stdafx.h"
#include "ProgressListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace chengine;

CProgressListBox::CProgressListBox()
{
}

CProgressListBox::~CProgressListBox()
{
	for(std::vector<_PROGRESSITEM_*>::iterator iter = m_vItems.begin(); iter != m_vItems.end(); ++iter)
	{
		delete *iter;
	}
}


BEGIN_MESSAGE_MAP(CProgressListBox, CListBox)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_CONTROL_REFLECT(LBN_KILLFOCUS, OnKillfocus)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressListBox message handlers

void CProgressListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (lpDrawItemStruct->itemID == -1 || lpDrawItemStruct->itemID >= m_vItems.size())
		return;
	_PROGRESSITEM_* pItem=m_vItems.at(lpDrawItemStruct->itemID);

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
	if(m_bShowCaptions)
	{
		CRect rcText(
			lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top + m_iTopMargin,
			lpDrawItemStruct->rcItem.right,
			lpDrawItemStruct->rcItem.bottom - (m_iProgressHeight + m_iMidMargin + m_iBottomMargin));
		pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		pDC->DrawText(pItem->m_strText, &rcText,
			DT_PATH_ELLIPSIS | DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

	// frame sizes
	int iEdgeWidth = 1/*GetSystemMetrics(SM_CXEDGE)*/;
	int iEdgeHeight = 1/*GetSystemMetrics(SM_CYEDGE)*/;

	// progress like drawing
	int iBoxWidth = static_cast<int>(static_cast<double>(((9 + 2) - 2 * iEdgeWidth)) * (2.0 / 3.0)) + 1;
	CRect rcProgress(
		lpDrawItemStruct->rcItem.left,
		lpDrawItemStruct->rcItem.bottom - m_iProgressHeight - m_iBottomMargin,
		lpDrawItemStruct->rcItem.left + 2 * iEdgeWidth + ((lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left - 2 * iEdgeWidth) / iBoxWidth) * iBoxWidth,
		lpDrawItemStruct->rcItem.bottom - m_iBottomMargin);

	// edge
	pDC->Draw3dRect(&rcProgress, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));

	if(!m_bSmoothProgress)
	{
		// boxes within edge
		double dCount = static_cast<int>(static_cast<double>(((rcProgress.Width() - 2 * iEdgeHeight) / iBoxWidth))
			* (static_cast<double>(pItem->m_uiPos) / static_cast<double>(pItem->m_uiRange)));
		int iBoxCount = ((dCount - static_cast<int>(dCount)) > 0.2) ? static_cast<int>(dCount) + 1 : static_cast<int>(dCount);

		for(int i = 0; i < iBoxCount; i++)
		{
			pDC->FillSolidRect(
				lpDrawItemStruct->rcItem.left + i * iBoxWidth + iEdgeWidth + m_iProgressContentXMargin / 2,
				lpDrawItemStruct->rcItem.bottom - m_iProgressHeight - m_iBottomMargin + iEdgeHeight + m_iProgressContentYMargin / 2,
				iBoxWidth - m_iProgressContentXMargin,
				rcProgress.Height() - 2 * iEdgeHeight - m_iProgressContentYMargin,
				pItem->m_crColor);
		}
	}
	else
	{
		pDC->FillSolidRect(
			lpDrawItemStruct->rcItem.left + iEdgeWidth + m_iProgressContentXMargin / 2,
			lpDrawItemStruct->rcItem.bottom - m_iProgressHeight -m_iBottomMargin + iEdgeHeight + m_iProgressContentYMargin / 2,
			static_cast<int>((rcProgress.Width() - 2 * iEdgeHeight - 3) * (static_cast<double>(pItem->m_uiPos) / static_cast<double>(pItem->m_uiRange))),
			rcProgress.Height() - 2 * iEdgeHeight - m_iProgressContentYMargin,
			pItem->m_crColor);
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
		m_bShowCaptions = bShow;
		Init();

		RecalcHeight();
	}
}

void CProgressListBox::RecalcHeight()
{
	// new height
	int iCtlHeight = boost::numeric_cast<int>(m_vItems.size()) * GetItemHeight(0);

	// change control size
	CRect rcCtl;
	GetClientRect(&rcCtl);
	this->SetWindowPos(nullptr, 0, 0, rcCtl.Width(), iCtlHeight, SWP_NOZORDER | SWP_NOMOVE);
}

chengine::taskid_t CProgressListBox::GetSelectedTaskId() const
{
	int iSel = GetCurSel();
	if(iSel == LB_ERR || (size_t)iSel >= m_vItems.size())
		return NoTaskID;

	return m_vItems.at(iSel)->m_tTaskID;
}

void CProgressListBox::Init()
{
	CClientDC dc(this);
	dc.SelectObject(GetFont());
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int iHeight = MulDiv(tm.tmHeight + tm.tmExternalLeading, dc.GetDeviceCaps(LOGPIXELSY), tm.tmDigitizedAspectY);

	m_iProgressHeight = std::max(9, int(iHeight * 0.6));
	m_iTopMargin = std::max(2, int(iHeight / 6));
	m_iMidMargin = std::max(2, int(iHeight / 8));
	m_iBottomMargin = std::max(2, int(iHeight / 6));;
	m_iProgressContentXMargin = m_iProgressContentYMargin = std::max(1, m_iProgressHeight / 9);

	if(m_bShowCaptions)
	{
		SetItemHeight(0, m_iProgressHeight + m_iTopMargin + m_iMidMargin + m_iBottomMargin + iHeight);
	}
	else
		SetItemHeight(0, m_iProgressHeight + m_iTopMargin + m_iMidMargin + m_iBottomMargin);
}

_PROGRESSITEM_* CProgressListBox::GetItemAddress(int iIndex)
{
	if(boost::numeric_cast<int>(m_vItems.size()) > iIndex)
		return m_vItems.at(iIndex);

	_PROGRESSITEM_* pItem=new _PROGRESSITEM_;
	pItem->m_uiRange=100;
	m_vItems.push_back(pItem);
	return pItem;
}

void CProgressListBox::UpdateItems(int nLimit, bool bUpdateSize)
{
	// delete items from array
	if(boost::numeric_cast<int>(m_vItems.size()) > nLimit)
	{
		std::vector<_PROGRESSITEM_*>::iterator iterStart = m_vItems.begin() + nLimit;
		for(std::vector<_PROGRESSITEM_*>::iterator iterPos = iterStart; iterPos != m_vItems.end(); ++iterPos)
		{
			delete *iterPos;
		}
		m_vItems.erase(iterStart, m_vItems.end());
	}
	// change count of elements in a listbox
	if(GetCount() != boost::numeric_cast<int>(m_vItems.size()))
	{
		while(GetCount() < boost::numeric_cast<int>(m_vItems.size()))
			AddString(_T(""));

		while(GetCount() > boost::numeric_cast<int>(m_vItems.size()))
			DeleteString(boost::numeric_cast<UINT>(m_vItems.size()));
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

	if(!rcClip.IsRectEmpty())
	{
		CMemDC memDC(dc, &rcClip);
		memDC.GetDC().FillSolidRect(&rcClip, GetSysColor(COLOR_3DFACE));

		DefWindowProc(WM_PAINT, reinterpret_cast<WPARAM>(memDC.GetDC().m_hDC), 0);
	}
}

BOOL CProgressListBox::OnEraseBkgnd(CDC*) 
{
	return FALSE/*CListBox::OnEraseBkgnd(pDC)*/;
}

int CProgressListBox::SetCurrentSelection(int nSelect)
{
	int nResult=SetCurSel(nSelect);
	if (nSelect == -1)
		GetParent()->SendMessage(WM_COMMAND, (LBN_SELCANCEL << 16) | GetDlgCtrlID(), reinterpret_cast<LPARAM>(this->m_hWnd));

	return nResult;
}

void CProgressListBox::OnKillfocus() 
{
	SetCurrentSelection(-1);
}

void CProgressListBox::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	BOOL bOutside = FALSE;
	UINT uiIndex = ItemFromPoint(point, bOutside);
	if(!bOutside && uiIndex < m_vItems.size())
		SetCurrentSelection(uiIndex);
}

void CProgressListBox::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	chengine::taskid_t taskId = GetSelectedTaskId();
	CWnd* pWnd = GetParent();
	if(pWnd)
	{
		ClientToScreen(&point);

		TASK_CLICK_NOTIFICATION notify = { point, taskId };
		pWnd->SendMessage(WM_TASK_RCLICK, 0, (LPARAM)&notify);
	}
}

void CProgressListBox::SetSmoothProgress(bool bSmoothProgress)
{
	m_bSmoothProgress=bSmoothProgress;
}
