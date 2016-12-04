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
#ifndef __TMSGBOX_H__
#define __TMSGBOX_H__

#include "..\libictranslate\LanguageDialog.h"

class TMsgBox : public ictranslate::CLanguageDialog
{
public:
	enum EButtonConfig
	{
		eOk,
		eCancel,
		eOkCancel,
		eYesNo,
		eYesNoCancel
	};

	enum EIconConfig
	{
		eIcon_Info,
		eIcon_Warning,
		eIcon_Error
	};

public:
	TMsgBox(UINT uiMsgResourceID, EButtonConfig eButtons, EIconConfig eIcon, CWnd* pParent = nullptr);
	TMsgBox(const CString& strMessage, EButtonConfig eButtons, EIconConfig eIcon, CWnd* pParent = nullptr);
	virtual ~TMsgBox();

	void SetCheckBoxMessage(UINT uiMsgResourceID);
	void SetCheckBoxMessage(const CString& strCheckboxMessage);

	bool WasChecked() const;

	static INT_PTR MsgBox(UINT uiMsgResourceID, EButtonConfig eButtons, EIconConfig eIcon, UINT uiCheckboxResourceID = 0, bool* pbWasChecked = nullptr, CWnd* pParent = nullptr);
	static INT_PTR MsgBox(const CString& strMessage, EButtonConfig eButtons, EIconConfig eIcon, const CString& strCheckboxText = CString(), bool* pbWasChecked = nullptr, CWnd* pParent = nullptr);

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	void CalculateMinimumDlgSize();

	void InitRichEdit();

	void OnCancel() override;
	void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	void InitializeControls();

	void OnFirstButtonClicked();
	void OnSecondButtonClicked();
	void OnThirdButtonClicked();
	void OnRichEditResize(NMHDR* pNMHDR, LRESULT* pResult);

	CSize GetMaxSize();

private:
	CRichEditCtrl m_ctlRichEdit;
	CRichEditCtrl m_ctlMeasureRichEdit;
	CButton m_ctlButton1;
	CButton m_ctlButton2;
	CButton m_ctlButton3;
	CButton m_ctlCheck;
	CStatic m_ctlImage;

	CString m_strMessageText;

	EIconConfig m_eIcon;
	EButtonConfig m_eButtons;
	
	CString m_strCheckboxText;

	CRect m_rcRichEdit;
	CRect m_rcDialogMinSize;
	int m_iCheckBoxHeight;

	bool m_bCheckboxChecked;

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(TMsgBox)
};

#endif
