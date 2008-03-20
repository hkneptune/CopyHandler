// ICTranslateDlg.h : header file
//

#pragma once
#include "../libictranslate/ResourceManager.h"
#include "afxcmn.h"
#include "afxwin.h"

// CICTranslateDlg dialog
class CICTranslateDlg : public CDialog
{
// Construction
public:
	CICTranslateDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ICTRANSLATE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	static void EnumLngStrings(uint_t uiID, const ictranslate::CTranslationItem* pTranslationItem, ptr_t pData);
	void UpdateCustomListImages();

// Implementation
protected:
	HICON m_hIcon;
	ictranslate::CLangData m_ldBase;
	ictranslate::CLangData m_ldCustom;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CListCtrl m_ctlBaseLanguageList;
public:
	CListCtrl m_ctlCustomLanguageList;
	CEdit m_ctlSrcText;
	CEdit m_ctlDstText;
	afx_msg void OnFileOpenBaseTranslation();
	afx_msg void OnFileOpenYourTranslation();
	afx_msg void OnItemChangedSrcDataList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemChangedDstDataList(NMHDR *pNMHDR, LRESULT *pResult);
};
