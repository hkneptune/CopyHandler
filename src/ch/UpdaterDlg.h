#pragma once
#include "afxwin.h"
#include "UpdateChecker.h"

// CUpdaterDlg dialog

class CUpdaterDlg : public ictranslate::CLanguageDialog
{
	DECLARE_DYNAMIC(CUpdaterDlg)

public:
	CUpdaterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUpdaterDlg();

	// Dialog Data
	enum { IDD = IDD_UPDATER_DIALOG };

	virtual BOOL OnInitDialog();
	void StartChecking();

	afx_msg void OnBnClickedOpenWebpageButton();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	CStatic m_ctlText;
	CUpdateChecker m_ucChecker;

};
