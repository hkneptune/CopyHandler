/***************************************************************************
 *   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   (version 2) as published by the Free Software Foundation;             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __FEEDBACKREPLACEDLG_H__
#define __FEEDBACKREPLACEDLG_H__

namespace chengine
{
	class TFileInfo;
}

class CFeedbackReplaceDlg : public ictranslate::CLanguageDialog
{
	DECLARE_DYNAMIC(CFeedbackReplaceDlg)

public:
	CFeedbackReplaceDlg(const chengine::TFileInfo& spSrcFile, const chengine::TFileInfo& spDstFile, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFeedbackReplaceDlg();

	BOOL OnInitDialog() override;

	bool IsApplyToAllItemsChecked() const;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnCancel() override;

	void RefreshFilesInfo();
	void RefreshImages();

	DECLARE_MESSAGE_MAP()

private:
	CStatic m_ctlSrcIcon;
	CStatic m_ctlDstIcon;

	CEdit m_ctlSrcName;
	CEdit m_ctlSrcPath;
	CEdit m_ctlSrcSize;
	CEdit m_ctlSrcDate;

	CEdit m_ctlDstName;
	CEdit m_ctlDstPath;
	CEdit m_ctlDstSize;
	CEdit m_ctlDstDate;

	CMFCButton m_btnReplace;
	CMFCButton m_btnRename;
	CMFCButton m_btnResume;
	CMFCButton m_btnSkip;
	CMFCButton m_btnPause;
	CMFCButton m_btnCancel;

	CMenu m_menuMassReplace;
	CMenu m_menuMassRename;
	CMenu m_menuMassResume;
	CMenu m_menuMassSkip;

	CMFCMenuButton m_btnMassReplace;
	CMFCMenuButton m_btnMassRename;
	CMFCMenuButton m_btnMassResume;
	CMFCMenuButton m_btnMassSkip;

	BOOL m_bAllItems;

protected:
	const chengine::TFileInfo& m_rSrcFile;
	const chengine::TFileInfo& m_rDstFile;

public:
	afx_msg void OnBnClickedReplaceButton();
	afx_msg void OnBnClickedCopyRestButton();
	afx_msg void OnBnClickedSkipButton();
	afx_msg void OnBnClickedPauseButton();
	afx_msg void OnBnClickedCancelButton();

	afx_msg void OnBnMassReplace();
	afx_msg void OnBnMassRename();
	afx_msg void OnBnMassResume();
	afx_msg void OnBnMassSkip();
};

#endif
