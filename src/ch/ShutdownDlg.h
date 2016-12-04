/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
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
#ifndef __SHUTDOWNDLG_H__
#define __SHUTDOWNDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CShutdownDlg dialog

class CShutdownDlg : public ictranslate::CLanguageDialog
{
public:
	CShutdownDlg();   // standard constructor

	void SetOverallTime(int iTotalSeconds);

private:
	void FormatTimeString(int iTime, CString* pstrData);

	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

private:
	int m_iOverallTime = 0;
	int m_iTime = 0;		// time in seconds
	CProgressCtrl	m_ctlProgress;
	CString	m_strTime;
};

#endif
