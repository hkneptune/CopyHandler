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

	void UpdateBaseLanguageList();
	void UpdateCustomLanguageList();

	void UpdateCustomListImages();
	void UpdateCustomListImage(int iItem, bool bUpdateText);

	static int CALLBACK ListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// Implementation
protected:
	HICON m_hIcon;
	CImageList m_ilImages;
	ictranslate::CLangData m_ldBase;
	ictranslate::CLangData m_ldCustom;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CListCtrl m_ctlBaseLanguageList;
	CListCtrl m_ctlCustomLanguageList;
public:
	CEdit m_ctlSrcText;
	CEdit m_ctlDstText;
	afx_msg void OnFileOpenBaseTranslation();
	afx_msg void OnFileOpenYourTranslation();
	afx_msg void OnItemChangedSrcDataList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemChangedDstDataList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCopyButton();
	afx_msg void OnBnClickedApply();
	CEdit m_ctlSrcFilename;
	CEdit m_ctlSrcAuthor;
	CEdit m_ctlSrcLanguageName;
	CEdit m_ctlSrcHelpFilename;
	CEdit m_ctlSrcFont;
	CButton m_ctlSrcRTL;
	CEdit m_ctlDstFilename;
	CEdit m_ctlDstAuthor;
	CEdit m_ctlDstLanguageName;
	CEdit m_ctlDstHelpFilename;
	CEdit m_ctlDstFont;
	CButton m_ctlDstRTL;
	afx_msg void OnBnClickedChooseFontButton();
	afx_msg void OnEditCleanupTranslation();
};
