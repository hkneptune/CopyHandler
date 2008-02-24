#pragma once
#include "afxwin.h"


// CCrashDlg dialog

class CCrashDlg : public CDialog
{
	DECLARE_DYNAMIC(CCrashDlg)

public:
	CCrashDlg(bool bResult, PCTSTR pszFilename, CWnd* pParent = NULL);   // standard constructor
	virtual ~CCrashDlg();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_CRASH_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	bool m_bResult;
	CString m_strFilename;
public:
	CStatic m_ctlVersion;
	CEdit m_ctlLocation;
	CButton m_ctlOKButton;
	CStatic m_ctlInfo;
	CStatic m_ctlVersionInfo;
	CStatic m_ctlLocationInfo;
};
