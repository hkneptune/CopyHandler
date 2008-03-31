#pragma once
#include "afxwin.h"
#include "UpdateChecker.h"

// CUpdaterDlg dialog

class CUpdaterDlg : public ictranslate::CLanguageDialog
{
	DECLARE_DYNAMIC(CUpdaterDlg)

public:
	CUpdaterDlg(CUpdateChecker::ECheckResult eResult, PCTSTR pszVersion, PCTSTR pszError, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUpdaterDlg();

// Dialog Data
	enum { IDD = IDD_UPDATER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CStatic m_ctlText;

	CUpdateChecker::ECheckResult m_eResult;
	CString m_strVersion;
	CString m_strError;
public:
	virtual BOOL OnInitDialog();
};
