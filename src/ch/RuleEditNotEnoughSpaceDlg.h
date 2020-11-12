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
#pragma once

#include "../libchengine/FeedbackNotEnoughSpaceRule.h"

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg dialog

class RuleEditNotEnoughSpaceDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	RuleEditNotEnoughSpaceDlg(const chengine::FeedbackNotEnoughSpaceRule& rRule);

	chengine::FeedbackNotEnoughSpaceRule GetRule() const { return m_rule; }

protected:
	void OnLanguageChanged() override;
	void EnableControls();

	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bUseIncludeMask = FALSE;
	CComboBox m_ctlIncludeMask;
	BOOL m_bUseExcludeMask = FALSE;
	CComboBox m_ctlExcludeMask;

	CComboBox m_ctlResponse;

	chengine::FeedbackNotEnoughSpaceRule m_rule;

	CStringArray m_astrAddMask;
	CStringArray m_astrAddExcludeMask;
};
