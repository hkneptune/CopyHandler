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
#include "stdafx.h"
#include "resource.h"
#include "BufferSizeDlg.h"
#include "ch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBufferSizeDlg dialog

CBufferSizeDlg::CBufferSizeDlg(chengine::TBufferSizes* pInitialBufferSizes, chengine::TBufferSizes::EBufferType eSelectedBuffer) :
	ictranslate::CLanguageDialog(IDD_BUFFERSIZE_DIALOG),
	m_eSelectedBuffer(eSelectedBuffer)
{
	if (pInitialBufferSizes)
		m_bsSizes = *pInitialBufferSizes;
}

void CBufferSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TWODISKSMULTIPLIER_COMBO, m_ctlTwoDisksMulti);
	DDX_Control(pDX, IDC_ONEDISKMULTIPLIER_COMBO, m_ctlOneDiskMulti);
	DDX_Control(pDX, IDC_LANMULTIPLIER_COMBO, m_ctlLANMulti);
	DDX_Control(pDX, IDC_DEFAULTMULTIPLIER_COMBO, m_ctlDefaultMulti);
	DDX_Control(pDX, IDC_CDROMMULTIPLIER_COMBO, m_ctlCDROMMulti);
	DDX_Control(pDX, IDC_BUFFERCOUNT_SPIN, m_ctlBufferCountSpin);
	DDX_Text(pDX, IDC_DEFAULTSIZE_EDIT, m_uiDefaultSize);
	DDX_Text(pDX, IDC_LANSIZE_EDIT, m_uiLANSize);
	DDX_Text(pDX, IDC_CDROMSIZE_EDIT, m_uiCDROMSize);
	DDX_Text(pDX, IDC_ONEDISKSIZE_EDIT, m_uiOneDiskSize);
	DDX_Text(pDX, IDC_TWODISKSSIZE_EDIT, m_uiTwoDisksSize);
	DDX_Check(pDX, IDC_ONLYDEFAULT_CHECK, m_bOnlyDefaultCheck);
	DDX_Text(pDX, IDC_BUFFERCOUNT_EDIT, m_uiBufferCount);
	DDX_Text(pDX, IDC_MAXREADAHEAD_EDIT, m_uiMaxReadAhead);
	DDX_Text(pDX, IDC_MAXCONCURRENTREADS_EDIT, m_uiMaxConcurrentReads);
	DDX_Text(pDX, IDC_MAXCONCURRENTWRITES_EDIT, m_uiMaxConcurrentWrites);
}

BEGIN_MESSAGE_MAP(CBufferSizeDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_ONLYDEFAULT_CHECK, OnOnlydefaultCheck)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBufferSizeDlg message handlers

BOOL CBufferSizeDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	// set all the combos
	m_ctlDefaultMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlDefaultMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlDefaultMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));

	m_ctlOneDiskMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlOneDiskMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlOneDiskMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));

	m_ctlTwoDisksMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlTwoDisksMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlTwoDisksMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	
	m_ctlCDROMMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlCDROMMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlCDROMMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	
	m_ctlLANMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlLANMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlLANMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));

	// fill edit controls and set multipliers
	SetDefaultSize(m_bsSizes.GetDefaultSize());
	SetOneDiskSize(m_bsSizes.GetOneDiskSize());
	SetTwoDisksSize(m_bsSizes.GetTwoDisksSize());
	SetCDSize(m_bsSizes.GetCDSize());
	SetLANSize(m_bsSizes.GetLANSize());
	m_uiBufferCount = m_bsSizes.GetBufferCount();
	m_bOnlyDefaultCheck=m_bsSizes.IsOnlyDefault();
	m_uiMaxReadAhead = m_bsSizes.GetMaxReadAheadBuffers();
	m_uiMaxConcurrentReads = m_bsSizes.GetMaxConcurrentReads();
	m_uiMaxConcurrentWrites = m_bsSizes.GetMaxConcurrentWrites();

	// buffer count handling
	m_ctlBufferCountSpin.SetRange(1, 1000);

	EnableControls(!m_bsSizes.IsOnlyDefault());

	UpdateData(FALSE);

	// set focus to the requested control
	switch (m_eSelectedBuffer)
	{
	case chengine::TBufferSizes::eBuffer_Default:
		GetDlgItem(IDC_DEFAULTSIZE_EDIT)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_DEFAULTSIZE_EDIT))->SetSel(0, -1);
		break;
	case chengine::TBufferSizes::eBuffer_OneDisk:
		GetDlgItem(IDC_ONEDISKSIZE_EDIT)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_ONEDISKSIZE_EDIT))->SetSel(0, -1);
		break;
	case chengine::TBufferSizes::eBuffer_TwoDisks:
		GetDlgItem(IDC_TWODISKSSIZE_EDIT)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_TWODISKSSIZE_EDIT))->SetSel(0, -1);
		break;
	case chengine::TBufferSizes::eBuffer_CD:
		GetDlgItem(IDC_CDROMSIZE_EDIT)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_CDROMSIZE_EDIT))->SetSel(0, -1);
		break;
	case chengine::TBufferSizes::eBuffer_LAN:
		GetDlgItem(IDC_LANSIZE_EDIT)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_LANSIZE_EDIT))->SetSel(0, -1);
		break;
	case chengine::TBufferSizes::eBuffer_Last:
	default:
		break;
	}

	return FALSE;
}

void CBufferSizeDlg::OnLanguageChanged()
{
	UpdateData(TRUE);

	// set all the combos
	int iSel=m_ctlDefaultMulti.GetCurSel();
	m_ctlDefaultMulti.ResetContent();
	m_ctlDefaultMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlDefaultMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlDefaultMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	m_ctlDefaultMulti.SetCurSel(iSel);

	iSel=m_ctlOneDiskMulti.GetCurSel();
	m_ctlOneDiskMulti.ResetContent();
	m_ctlOneDiskMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlOneDiskMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlOneDiskMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	m_ctlOneDiskMulti.SetCurSel(iSel);

	iSel=m_ctlTwoDisksMulti.GetCurSel();
	m_ctlTwoDisksMulti.ResetContent();
	m_ctlTwoDisksMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlTwoDisksMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlTwoDisksMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	m_ctlTwoDisksMulti.SetCurSel(iSel);
	
	iSel=m_ctlCDROMMulti.GetCurSel();
	m_ctlCDROMMulti.ResetContent();
	m_ctlCDROMMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlCDROMMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlCDROMMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	m_ctlCDROMMulti.SetCurSel(iSel);
	
	iSel=m_ctlLANMulti.GetCurSel();
	m_ctlLANMulti.ResetContent();
	m_ctlLANMulti.AddString(GetResManager().LoadString(IDS_BYTE_STRING));
	m_ctlLANMulti.AddString(GetResManager().LoadString(IDS_KBYTE_STRING));
	m_ctlLANMulti.AddString(GetResManager().LoadString(IDS_MBYTE_STRING));
	m_ctlLANMulti.SetCurSel(iSel);

	UpdateData(FALSE);
}

UINT CBufferSizeDlg::IndexToValue(int iIndex)
{
	switch (iIndex)
	{
	case 0:
		return 1;
	case 1:
		return 1024;
	case 2:
		return 1048576;
	default:
		ASSERT(true);	// bad index
		return 1;
	}
}

void CBufferSizeDlg::OnOK() 
{
	if (!UpdateData(TRUE))
		return;
	
	// no buffer could be 0
	if (m_uiDefaultSize == 0 || m_uiOneDiskSize == 0 || m_uiTwoDisksSize == 0 || m_uiCDROMSize == 0 || m_uiLANSize == 0)
	{
		MsgBox(IDS_BUFFERSIZEZERO_STRING);
		return;
	}

	// assign values
	m_bsSizes.SetOnlyDefault(m_bOnlyDefaultCheck != 0);
	m_bsSizes.SetDefaultSize(m_uiDefaultSize*IndexToValue(m_ctlDefaultMulti.GetCurSel()));
	m_bsSizes.SetOneDiskSize(m_uiOneDiskSize*IndexToValue(m_ctlOneDiskMulti.GetCurSel()));
	m_bsSizes.SetTwoDisksSize(m_uiTwoDisksSize*IndexToValue(m_ctlTwoDisksMulti.GetCurSel()));
	m_bsSizes.SetCDSize(m_uiCDROMSize*IndexToValue(m_ctlCDROMMulti.GetCurSel()));
	m_bsSizes.SetLANSize(m_uiLANSize*IndexToValue(m_ctlLANMulti.GetCurSel()));
	m_bsSizes.SetBufferCount(m_uiBufferCount);
	m_bsSizes.SetMaxReadAheadBuffers(m_uiMaxReadAhead);
	m_bsSizes.SetMaxConcurrentReads(m_uiMaxConcurrentReads);
	m_bsSizes.SetMaxConcurrentWrites(m_uiMaxConcurrentWrites);

	CLanguageDialog::OnOK();
}

void CBufferSizeDlg::SetDefaultSize(UINT uiSize)
{
	if ((uiSize % 1048576) == 0)
	{
		m_uiDefaultSize=static_cast<UINT>(uiSize/1048576);
		m_ctlDefaultMulti.SetCurSel(2);
	}
	else if ((uiSize % 1024) == 0)
	{
		m_uiDefaultSize=static_cast<UINT>(uiSize/1024);
		m_ctlDefaultMulti.SetCurSel(1);
	}
	else
	{
		m_uiDefaultSize=uiSize;
		m_ctlDefaultMulti.SetCurSel(0);
	}
}

void CBufferSizeDlg::SetOneDiskSize(UINT uiSize)
{
	if ((uiSize % 1048576) == 0)
	{
		m_uiOneDiskSize=static_cast<UINT>(uiSize/1048576);
		m_ctlOneDiskMulti.SetCurSel(2);
	}
	else if ((uiSize % 1024) == 0)
	{
		m_uiOneDiskSize=static_cast<UINT>(uiSize/1024);
		m_ctlOneDiskMulti.SetCurSel(1);
	}
	else
	{
		m_uiOneDiskSize=uiSize;
		m_ctlOneDiskMulti.SetCurSel(0);
	}
}

void CBufferSizeDlg::SetTwoDisksSize(UINT uiSize)
{
	if ((uiSize % 1048576) == 0)
	{
		m_uiTwoDisksSize=static_cast<UINT>(uiSize/1048576);
		m_ctlTwoDisksMulti.SetCurSel(2);
	}
	else if ((uiSize % 1024) == 0)
	{
		m_uiTwoDisksSize=static_cast<UINT>(uiSize/1024);
		m_ctlTwoDisksMulti.SetCurSel(1);
	}
	else
	{
		m_uiTwoDisksSize=uiSize;
		m_ctlTwoDisksMulti.SetCurSel(0);
	}
}

void CBufferSizeDlg::SetCDSize(UINT uiSize)
{
	if ((uiSize % 1048576) == 0)
	{
		m_uiCDROMSize=static_cast<UINT>(uiSize/1048576);
		m_ctlCDROMMulti.SetCurSel(2);
	}
	else if ((uiSize % 1024) == 0)
	{
		m_uiCDROMSize=static_cast<UINT>(uiSize/1024);
		m_ctlCDROMMulti.SetCurSel(1);
	}
	else
	{
		m_uiCDROMSize=uiSize;
		m_ctlCDROMMulti.SetCurSel(0);
	}
}

void CBufferSizeDlg::SetLANSize(UINT uiSize)
{
	if ((uiSize % 1048576) == 0)
	{
		m_uiLANSize=static_cast<UINT>(uiSize/1048576);
		m_ctlLANMulti.SetCurSel(2);
	}
	else if ((uiSize % 1024) == 0)
	{
		m_uiLANSize=static_cast<UINT>(uiSize/1024);
		m_ctlLANMulti.SetCurSel(1);
	}
	else
	{
		m_uiLANSize=uiSize;
		m_ctlLANMulti.SetCurSel(0);
	}
}

void CBufferSizeDlg::EnableControls(bool bEnable)
{
	GetDlgItem(IDC_ONEDISKSIZE_EDIT)->EnableWindow(bEnable);
	m_ctlOneDiskMulti.EnableWindow(bEnable);
	GetDlgItem(IDC_TWODISKSSIZE_EDIT)->EnableWindow(bEnable);
	m_ctlTwoDisksMulti.EnableWindow(bEnable);
	GetDlgItem(IDC_CDROMSIZE_EDIT)->EnableWindow(bEnable);
	m_ctlCDROMMulti.EnableWindow(bEnable);
	GetDlgItem(IDC_LANSIZE_EDIT)->EnableWindow(bEnable);
	m_ctlLANMulti.EnableWindow(bEnable);
}

void CBufferSizeDlg::OnOnlydefaultCheck() 
{
	UpdateData(TRUE);
	EnableControls(m_bOnlyDefaultCheck == 0);
}
