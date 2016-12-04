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
#ifndef __BUFFERSIZEDLG_H__
#define __BUFFERSIZEDLG_H__

#include "..\libchcore\TBufferSizes.h"

/////////////////////////////////////////////////////////////////////////////
// CBufferSizeDlg dialog

class CBufferSizeDlg : public ictranslate::CLanguageDialog
{
public:
	CBufferSizeDlg(chcore::TBufferSizes* pInitialBufferSizes, chcore::TBufferSizes::EBufferType eSelectedBuffer = chcore::TBufferSizes::eBuffer_Default);

	const chcore::TBufferSizes& GetBufferSizes() const { return m_bsSizes; }

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnLanguageChanged() override;
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnOnlydefaultCheck();

	void SetLANSize(UINT uiSize);
	void SetCDSize(UINT uiSize);
	void SetTwoDisksSize(UINT uiSize);
	void SetOneDiskSize(UINT uiSize);
	void SetDefaultSize(UINT uiSize);
	UINT IndexToValue(int iIndex);

	void EnableControls(bool bEnable=true);

	DECLARE_MESSAGE_MAP()

private:
	CComboBox	m_ctlTwoDisksMulti;
	CComboBox	m_ctlOneDiskMulti;
	CComboBox	m_ctlLANMulti;
	CComboBox	m_ctlDefaultMulti;
	CComboBox	m_ctlCDROMMulti;
	CSpinButtonCtrl m_ctlBufferCountSpin;

	UINT m_uiDefaultSize = 0;
	UINT m_uiLANSize = 0;
	UINT m_uiCDROMSize = 0;
	UINT m_uiOneDiskSize = 0;
	UINT m_uiTwoDisksSize = 0;
	UINT m_uiBufferCount = 0;
	BOOL m_bOnlyDefaultCheck = TRUE;

	chcore::TBufferSizes::EBufferType m_eSelectedBuffer;
	chcore::TBufferSizes m_bsSizes;
};

#endif
