// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
//  ixen@copyhandler.com
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
#include "TMsgBox.h"
#include "ch.h"

using namespace ictranslate;

IMPLEMENT_DYNAMIC(TMsgBox, CLanguageDialog)

BEGIN_MESSAGE_MAP(TMsgBox, CLanguageDialog)
	ON_BN_CLICKED(IDC_FIRST_BUTTON, &TMsgBox::OnFirstButtonClicked)
	ON_BN_CLICKED(IDC_SECOND_BUTTON, &TMsgBox::OnSecondButtonClicked)
	ON_BN_CLICKED(IDC_THIRD_BUTTON, &TMsgBox::OnThirdButtonClicked)
	ON_NOTIFY(EN_REQUESTRESIZE, IDC_MSG_RICHEDIT, OnRichEditResize)
	ON_NOTIFY(EN_REQUESTRESIZE, IDC_MEASURE_RICHEDIT, OnRichEditResize)
END_MESSAGE_MAP()

TMsgBox::TMsgBox(UINT uiMsgResourceID, EButtonConfig eButtons, EIconConfig eIcon, CWnd* pParent /*= NULL*/) :
	CLanguageDialog(IDD_MSGBOX_DIALOG, pParent),
	m_eButtons(eButtons),
	m_eIcon(eIcon),
	m_rcRichEdit(0,0,0,0)
{
	m_strMessageText = GetResManager().LoadString(uiMsgResourceID);
}

TMsgBox::TMsgBox(const CString& strMessage, EButtonConfig eButtons, EIconConfig eIcon, CWnd* pParent /*= NULL*/) :
	CLanguageDialog(IDD_MSGBOX_DIALOG, pParent),
	m_strMessageText(strMessage),
	m_eButtons(eButtons),
	m_eIcon(eIcon)
{
}

TMsgBox::~TMsgBox()
{
}

void TMsgBox::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MSG_RICHEDIT, m_ctlRichEdit);
	DDX_Control(pDX, IDC_FIRST_BUTTON, m_ctlButton1);
	DDX_Control(pDX, IDC_SECOND_BUTTON, m_ctlButton2);
	DDX_Control(pDX, IDC_THIRD_BUTTON, m_ctlButton3);
	DDX_Control(pDX, IDC_BASIC_CHECK, m_ctlCheck);
	DDX_Control(pDX, IDC_IMAGE_STATIC, m_ctlImage);
	DDX_Control(pDX, IDC_MEASURE_RICHEDIT, m_ctlMeasureRichEdit);
}

BOOL TMsgBox::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	m_ctlMeasureRichEdit.SetEventMask(m_ctlRichEdit.GetEventMask() | ENM_REQUESTRESIZE);

	AddResizableControl(IDC_IMAGE_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_MSG_RICHEDIT, 0.0, 0.0, 1.0, 1.0);
	AddResizableControl(IDC_FIRST_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_SECOND_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_THIRD_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_BASIC_CHECK, 0.0, 1.0, 1.0, 0.0);

	InitializeResizableControls();

	m_ctlRichEdit.GetWindowRect(&m_rcRichEdit);
	ScreenToClient(&m_rcRichEdit);

	// initialize controls' texts
	InitializeControls();

	return TRUE;
}

void TMsgBox::OnFirstButtonClicked()
{
	const int iUndefinedResult = IDCANCEL;

	switch(m_eButtons)
	{
	case eOk:
		EndDialog(iUndefinedResult);
		break;
	case eCancel:
		EndDialog(iUndefinedResult);
		break;
	case eOkCancel:
		EndDialog(iUndefinedResult);
		break;
	case eYesNo:
		EndDialog(iUndefinedResult);
		break;
	case eYesNoCancel:
		EndDialog(IDYES);
		break;

	default:
		{
			_ASSERTE(FALSE);		// unsupported option
			EndDialog(iUndefinedResult);
		}
	}
}

void TMsgBox::OnSecondButtonClicked()
{
	const int iUndefinedResult = IDCANCEL;

	// the middle button
	switch(m_eButtons)
	{
	case eOk:
		EndDialog(iUndefinedResult);
		break;
	case eCancel:
		EndDialog(iUndefinedResult);
		break;
	case eOkCancel:
		EndDialog(IDOK);
		break;
	case eYesNo:
		EndDialog(IDYES);
		break;
	case eYesNoCancel:
		EndDialog(IDNO);
		break;

	default:
		{
			_ASSERTE(FALSE);		// unsupported option
			EndDialog(iUndefinedResult);
		}
	}
}

void TMsgBox::OnThirdButtonClicked()
{
	const int iUndefinedResult = IDCANCEL;

	// the rightmost button
	switch(m_eButtons)
	{
	case eOk:
		EndDialog(IDOK);
		break;
	case eCancel:
		EndDialog(IDCANCEL);
		break;
	case eOkCancel:
		EndDialog(IDCANCEL);
		break;
	case eYesNo:
		EndDialog(IDNO);
		break;
	case eYesNoCancel:
		EndDialog(IDCANCEL);
		break;

	default:
		{
			_ASSERTE(FALSE);		// unsupported option
			EndDialog(iUndefinedResult);
		}
	}
}

void TMsgBox::SetCheckBoxMessage(UINT uiMsgResourceID)
{
	m_strCheckboxText = GetResManager().LoadString(uiMsgResourceID);
}

void TMsgBox::SetCheckBoxMessage(const CString& strCheckboxMessage)
{
	m_strCheckboxText = strCheckboxMessage;
}

void TMsgBox::InitializeControls()
{
	m_ctlRichEdit.SetWindowText(m_strMessageText);
	m_ctlMeasureRichEdit.SetWindowText(m_strMessageText);

	HICON hIcon = NULL;
	switch(m_eIcon)
	{
	case eIcon_Warning:
		hIcon = AfxGetApp()->LoadStandardIcon(IDI_WARNING);
		break;

	case eIcon_Error:
		hIcon = AfxGetApp()->LoadStandardIcon(IDI_ERROR);
		break;

	case eIcon_Info:
	default:
		hIcon = AfxGetApp()->LoadStandardIcon(IDI_INFORMATION);
		break;
	}

	m_ctlImage.SetIcon(hIcon);

	if(m_strCheckboxText.IsEmpty())
		m_ctlCheck.ShowWindow(SW_HIDE);
	else
		m_ctlCheck.SetWindowText(m_strCheckboxText);

	switch(m_eButtons)
	{
	case eOk:
		m_ctlButton1.ShowWindow(SW_HIDE);
		m_ctlButton2.ShowWindow(SW_HIDE);
		m_ctlButton3.SetWindowText(GetResManager().LoadString(IDS_OK_STRING));
		break;
	case eOkCancel:
		m_ctlButton1.ShowWindow(SW_HIDE);
		m_ctlButton2.SetWindowText(GetResManager().LoadString(IDS_OK_STRING));
		m_ctlButton3.SetWindowText(GetResManager().LoadString(IDS_CANCEL_STRING));
		break;
	case eYesNo:
		m_ctlButton1.ShowWindow(SW_HIDE);
		m_ctlButton2.SetWindowText(GetResManager().LoadString(IDS_YES_STRING));
		m_ctlButton3.SetWindowText(GetResManager().LoadString(IDS_NO_STRING));
		break;
	case eYesNoCancel:
		m_ctlButton1.SetWindowText(GetResManager().LoadString(IDS_YES_STRING));
		m_ctlButton2.SetWindowText(GetResManager().LoadString(IDS_NO_STRING));
		m_ctlButton3.SetWindowText(GetResManager().LoadString(IDS_CANCEL_STRING));
		break;

	case eCancel:
	default:
		m_ctlButton1.ShowWindow(SW_HIDE);
		m_ctlButton2.ShowWindow(SW_HIDE);
		m_ctlButton3.SetWindowText(GetResManager().LoadString(IDS_CANCEL_STRING));
		break;
	}
}

void TMsgBox::OnRichEditResize(NMHDR* pNMHDR, LRESULT* pResult)
{
	REQRESIZE* pResize = (REQRESIZE*)pNMHDR;

	if(pResize && !m_rcRichEdit.IsRectNull())
	{
		if(pNMHDR->idFrom == IDC_MEASURE_RICHEDIT)
		{
			// get current monitor's resolution (and an aspect ratio)
			CSize sizeMax = GetMaxSize();

			// new rich edit control width/height suggestion
			int iSuggestedWidth = pResize->rc.right - pResize->rc.left;
			int iSuggestedHeight = pResize->rc.bottom - pResize->rc.top;
			int iSuggestedArea = iSuggestedWidth * iSuggestedHeight;

			// calculate approximate new height/width of a control with monitor's aspect ratio
			// with total area similar to the suggested one
			int iCalcWidth = (int)sqrt((double)sizeMax.cx * (double)iSuggestedArea / (double)sizeMax.cy);
			int iCalcHeight = (int)sqrt((double)sizeMax.cy * (double)iSuggestedArea / (double)sizeMax.cx);

			// calculate control size difference to apply to the dialog size
			int iWidthDiff = iCalcWidth - m_rcRichEdit.Width();
			int iHeightDiff = iCalcHeight - m_rcRichEdit.Height();

			// and apply the diff
			CRect rcThis(0,0,0,0);
			GetWindowRect(&rcThis);

			int iNewWidth = rcThis.Width() + iWidthDiff;
			int iNewHeight = rcThis.Height() + iHeightDiff;

			// make sure we don't exceed the max size
			if(iNewHeight > sizeMax.cy)
				iNewHeight = sizeMax.cy;
			if(iNewWidth > sizeMax.cx)
				iNewWidth = sizeMax.cx;

			// move window
			SetWindowPos(NULL, 0, 0, iNewWidth, iNewHeight, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE);

			// update richedit size
			m_ctlRichEdit.GetWindowRect(&m_rcRichEdit);
			ScreenToClient(&m_rcRichEdit);

			// and request another resize (now on the real richedit)
			m_ctlRichEdit.SetEventMask(m_ctlRichEdit.GetEventMask() | ENM_REQUESTRESIZE);
			m_ctlRichEdit.RequestResize();

			m_ctlMeasureRichEdit.SetEventMask(m_ctlMeasureRichEdit.GetEventMask() & ~ENM_REQUESTRESIZE);
		}
		else
		{
			int iWidthDiff = pResize->rc.right - pResize->rc.left - m_rcRichEdit.Width();
			int iHeightDiff = pResize->rc.bottom - pResize->rc.top - m_rcRichEdit.Height();

			CRect rcThis(0,0,0,0);
			GetWindowRect(&rcThis);

			SetWindowPos(NULL, 0, 0, rcThis.Width() + iWidthDiff, rcThis.Height() + iHeightDiff, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE);

			m_ctlRichEdit.SetEventMask(m_ctlRichEdit.GetEventMask() & ~ENM_REQUESTRESIZE);
		}
	}

	if(pResult)
		*pResult = 0;
}

CSize TMsgBox::GetMaxSize()
{
	CSize sizeMax = CSize(800, 600);

	HMONITOR hMonitor = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if(hMonitor)
	{
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);

		if(GetMonitorInfo(hMonitor, &mi))
		{
			sizeMax.cx = (int)((mi.rcWork.right - mi.rcWork.left) * 0.8);
			sizeMax.cy = (int)((mi.rcWork.bottom - mi.rcWork.top) * 0.8);
		}
	}
	else
	{
		RECT rcArea = { 0, 0, 0, 0 };

		if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, 0))
		{
			sizeMax.cx = (int)((rcArea.right - rcArea.left) * 0.8);
			sizeMax.cy = (int)((rcArea.bottom - rcArea.top) * 0.8);
		}
	}

	return sizeMax;
}
