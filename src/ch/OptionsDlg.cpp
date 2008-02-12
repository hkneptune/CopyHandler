/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/

#include "stdafx.h"
#include "ch.h"
#include "resource.h"
#include "OptionsDlg.h"
#include "BufferSizeDlg.h"
#include "ShortcutsDlg.h"
#include "RecentDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool COptionsDlg::m_bLock=false;

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CHLanguageDialog(COptionsDlg::IDD, pParent, &m_bLock)
{
	//{{AFX_DATA_INIT(COptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CHLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	DDX_Control(pDX, IDC_PROPERTIES_LIST, m_ctlProperties);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsDlg, CHLanguageDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_BN_CLICKED(IDC_APPLY_BUTTON, OnApplyButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

// properties handling macros
#define PROP_SEPARATOR(text)\
	m_ctlProperties.AddString(text);

#define PROP_BOOL(text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_COMBO_LIST, IDS_BOOLTEXT_STRING, (value));

#define PROP_UINT(text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_TEXT, _itot((value), m_szBuffer, 10), 0);

#define PROP_COMBO(text, prop_text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_COMBO_LIST, prop_text, (value));

#define PROP_DIR(text, prop_text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_DIR, (value)+CString(GetResManager()->LoadString(prop_text)), 0);

#define PROP_PATH(text, prop_text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_PATH, (value)+CString(GetResManager()->LoadString(prop_text)), 0);

#define PROP_CUSTOM_UINT(text, value, callback, param)\
	m_ctlProperties.AddString(text, ID_PROPERTY_CUSTOM, CString(_itot(value, m_szBuffer, 10)), callback, this, param, 0);

#define SKIP_SEPARATOR(pos)\
	pos++;

BOOL COptionsDlg::OnInitDialog() 
{
	CHLanguageDialog::OnInitDialog();
	
	m_ctlProperties.Init();

	// copy shortcut and recent paths
	GetConfig()->GetStringArrayValue(PP_RECENTPATHS, &m_cvRecent);
	GetConfig()->GetStringArrayValue(PP_SHORTCUTS, &m_cvShortcuts);

	GetConfig()->GetStringValue(PP_PLANGDIR, m_szLangPath, _MAX_PATH);
	GetApp()->ExpandPath(m_szLangPath);

	GetResManager()->Scan(m_szLangPath, &m_vld);

	// some attributes
	m_ctlProperties.SetBkColor(RGB(255, 255, 255));
	m_ctlProperties.SetTextColor(RGB(80, 80, 80));
	m_ctlProperties.SetTextHighlightColor(RGB(80,80,80));
	m_ctlProperties.SetHighlightColor(RGB(200, 200, 200));
	m_ctlProperties.SetPropertyBkColor(RGB(255,255,255));
	m_ctlProperties.SetPropertyTextColor(RGB(0,0,0));
	m_ctlProperties.SetLineStyle(RGB(74,109,132), PS_SOLID);

	FillPropertyList();

	return TRUE;
}

void CustomPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex)
{
	COptionsDlg* pDlg=static_cast<COptionsDlg*>(lpParam);
	CBufferSizeDlg dlg;

	dlg.m_bsSizes.m_bOnlyDefault=pDlg->GetBoolProp(iIndex-iParam-1);
	dlg.m_bsSizes.m_uiDefaultSize=pDlg->GetUintProp(iIndex-iParam);
	dlg.m_bsSizes.m_uiOneDiskSize=pDlg->GetUintProp(iIndex-iParam+1);
	dlg.m_bsSizes.m_uiTwoDisksSize=pDlg->GetUintProp(iIndex-iParam+2);
	dlg.m_bsSizes.m_uiCDSize=pDlg->GetUintProp(iIndex-iParam+3);
	dlg.m_bsSizes.m_uiLANSize=pDlg->GetUintProp(iIndex-iParam+4);
	dlg.m_iActiveIndex=iParam;	// selected buffer for editing

	if (dlg.DoModal() == IDOK)
	{
		PROPERTYITEM* pItem;
		TCHAR xx[32];

		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam-1));
		pItem->nPropertySelected=(dlg.m_bsSizes.m_bOnlyDefault ? 1 : 0);
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam));
		pItem->csProperties.SetAt(0, _itot(dlg.m_bsSizes.m_uiDefaultSize, xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+1));
		pItem->csProperties.SetAt(0, _itot(dlg.m_bsSizes.m_uiOneDiskSize, xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+2));
		pItem->csProperties.SetAt(0, _itot(dlg.m_bsSizes.m_uiTwoDisksSize, xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+3));
		pItem->csProperties.SetAt(0, _itot(dlg.m_bsSizes.m_uiCDSize, xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+4));
		pItem->csProperties.SetAt(0, _itot(dlg.m_bsSizes.m_uiLANSize, xx, 10));
	}
}

void ShortcutsPropertyCallbackProc(LPVOID lpParam, int /*iParam*/, CPtrList* pList, int iIndex)
{
	COptionsDlg* pDlg=static_cast<COptionsDlg*>(lpParam);

	CShortcutsDlg dlg;
	dlg.m_cvShortcuts.assign(pDlg->m_cvShortcuts.begin(), pDlg->m_cvShortcuts.end(), true, true);
	dlg.m_pcvRecent=&pDlg->m_cvRecent;
	if (dlg.DoModal() == IDOK)
	{
		// restore shortcuts to pDlg->cvShortcuts
		pDlg->m_cvShortcuts.assign(dlg.m_cvShortcuts.begin(), dlg.m_cvShortcuts.end(), true, false);
		dlg.m_cvShortcuts.erase(dlg.m_cvShortcuts.begin(), dlg.m_cvShortcuts.end(), false);

		// property list
		TCHAR szBuf[32];
		PROPERTYITEM* pItem;
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex));
		pItem->csProperties.SetAt(0, _itot(pDlg->m_cvShortcuts.size(), szBuf, 10));
	}
}

void RecentPropertyCallbackProc(LPVOID lpParam, int /*iParam*/, CPtrList* pList, int iIndex)
{
	COptionsDlg* pDlg=static_cast<COptionsDlg*>(lpParam);

	CRecentDlg dlg;
	dlg.m_cvRecent.assign(pDlg->m_cvRecent.begin(), pDlg->m_cvRecent.end(), true, true);
	if (dlg.DoModal() == IDOK)
	{
		// restore
		pDlg->m_cvRecent.assign(dlg.m_cvRecent.begin(), dlg.m_cvRecent.end(), true, false);
		dlg.m_cvRecent.erase(dlg.m_cvRecent.begin(), dlg.m_cvRecent.end(), false);

		// property list
		TCHAR szBuf[32];
		PROPERTYITEM* pItem;
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex));
		pItem->csProperties.SetAt(0, _itot(pDlg->m_cvRecent.size(), szBuf, 10));
	}
}

void COptionsDlg::OnOK() 
{
	// kill focuses
	m_ctlProperties.HideControls();

	ApplyProperties();

	SendClosingNotify();
	CHLanguageDialog::OnOK();
}

void COptionsDlg::FillPropertyList()
{
	CString strPath;

	// load settings
	PROP_SEPARATOR(IDS_PROGRAM_STRING)
	PROP_BOOL(IDS_CLIPBOARDMONITORING_STRING, GetConfig()->GetBoolValue(PP_PCLIPBOARDMONITORING))
	PROP_UINT(IDS_CLIPBOARDINTERVAL_STRING, GetConfig()->GetIntValue(PP_PMONITORSCANINTERVAL))
	PROP_BOOL(IDS_AUTORUNPROGRAM_STRING, GetConfig()->GetBoolValue(PP_PRELOADAFTERRESTART))
	PROP_BOOL(IDS_AUTOSHUTDOWN_STRING, GetConfig()->GetBoolValue(PP_PSHUTDOWNAFTREFINISHED))
	PROP_UINT(IDS_SHUTDOWNTIME_STRING, GetConfig()->GetIntValue(PP_PTIMEBEFORESHUTDOWN))
	PROP_COMBO(IDS_FORCESHUTDOWN_STRING, IDS_FORCESHUTDOWNVALUES_STRING, GetConfig()->GetBoolValue(PP_PFORCESHUTDOWN))
	PROP_UINT(IDS_AUTOSAVEINTERVAL_STRING, GetConfig()->GetIntValue(PP_PAUTOSAVEINTERVAL))
	PROP_COMBO(IDS_CFGPRIORITYCLASS_STRING, IDS_CFGPRIORITYCLASSITEMS_STRING, PriorityClassToIndex(GetConfig()->GetIntValue(PP_PPROCESSPRIORITYCLASS)))
	
	GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	TRACE("Autosavedir=%s\n", strPath);
	PROP_DIR(IDS_TEMPFOLDER_STRING, IDS_TEMPFOLDERCHOOSE_STRING, strPath)

	GetConfig()->GetStringValue(PP_PPLUGINSDIR, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	PROP_DIR(IDS_PLUGSFOLDER_STRING, IDS_PLUGSFOLDERCHOOSE_STRING, strPath)

	GetConfig()->GetStringValue(PP_PHELPDIR, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	PROP_DIR(IDS_CFGHELPDIR_STRING, IDS_CFGHELPDIRCHOOSE_STRING, strPath)
	
	// lang
	CString strLangs;
	UINT uiIndex=0;
	for (vector<CLangData>::iterator it=m_vld.begin();it != m_vld.end();it++)
	{
		strLangs+=(*it).pszLngName;
		strLangs+=_T("!");
		if (_tcsicmp((*it).GetFilename(true), GetResManager()->m_ld.GetFilename(true)) == 0)
			uiIndex=it-m_vld.begin();
	}
	strLangs.TrimRight(_T('!'));

	PROP_COMBO(IDS_LANGUAGE_STRING, strLangs, uiIndex)

	GetConfig()->GetStringValue(PP_PLANGDIR, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	PROP_DIR(IDS_LANGUAGESFOLDER_STRING, IDS_LANGSFOLDERCHOOSE_STRING, strPath)

	/////////////////
	PROP_SEPARATOR(IDS_STATUSWINDOW_STRING);
	PROP_UINT(IDS_REFRESHSTATUSINTERVAL_STRING, GetConfig()->GetIntValue(PP_STATUSREFRESHINTERVAL))
	PROP_BOOL(IDS_STATUSSHOWDETAILS_STRING, GetConfig()->GetBoolValue(PP_STATUSSHOWDETAILS))
	PROP_BOOL(IDS_STATUSAUTOREMOVE_STRING, GetConfig()->GetBoolValue(PP_STATUSAUTOREMOVEFINISHED))

	PROP_SEPARATOR(IDS_MINIVIEW_STRING)
	PROP_BOOL(IDS_SHOWFILENAMES_STRING, GetConfig()->GetBoolValue(PP_MVSHOWFILENAMES))
	PROP_BOOL(IDS_SHOWSINGLETASKS_STRING, GetConfig()->GetBoolValue(PP_MVSHOWSINGLETASKS))
	PROP_UINT(IDS_MINIVIEWREFRESHINTERVAL_STRING, GetConfig()->GetIntValue(PP_MVREFRESHINTERVAL))
	PROP_BOOL(IDS_MINIVIEWSHOWAFTERSTART_STRING, GetConfig()->GetBoolValue(PP_MVAUTOSHOWWHENRUN))
	PROP_BOOL(IDS_MINIVIEWAUTOHIDE_STRING, GetConfig()->GetBoolValue(PP_MVAUTOHIDEWHENEMPTY))
	PROP_BOOL(IDS_MINIVIEWSMOOTHPROGRESS_STRING, GetConfig()->GetBoolValue(PP_MVUSESMOOTHPROGRESS))

	PROP_SEPARATOR(IDS_CFGFOLDERDIALOG_STRING)
	PROP_BOOL(IDS_CFGFDEXTVIEW_STRING, GetConfig()->GetBoolValue(PP_FDEXTENDEDVIEW))
	PROP_UINT(IDS_CFGFDWIDTH_STRING, GetConfig()->GetIntValue(PP_FDWIDTH))
	PROP_UINT(IDS_CFGFDHEIGHT_STRING, GetConfig()->GetIntValue(PP_FDHEIGHT))
	PROP_COMBO(IDS_CFGFDSHORTCUTS_STRING, IDS_CFGFDSHORTCUTSSTYLES_STRING, GetConfig()->GetIntValue(PP_FDSHORTCUTLISTSTYLE))
	PROP_BOOL(IDS_CFGFDIGNOREDIALOGS_STRING, GetConfig()->GetBoolValue(PP_FDIGNORESHELLDIALOGS))

	PROP_SEPARATOR(IDS_CFGSHELL_STRING)
	PROP_BOOL(IDS_CFGSHCOPY_STRING, GetConfig()->GetBoolValue(PP_SHSHOWCOPY))
	PROP_BOOL(IDS_CFGSHMOVE_STRING, GetConfig()->GetBoolValue(PP_SHSHOWMOVE))
	PROP_BOOL(IDS_CFGSHCMSPECIAL_STRING, GetConfig()->GetBoolValue(PP_SHSHOWCOPYMOVE))
	PROP_BOOL(IDS_CFGSHPASTE_STRING, GetConfig()->GetBoolValue(PP_SHSHOWPASTE))
	PROP_BOOL(IDS_CFGSHPASTESPECIAL_STRING, GetConfig()->GetBoolValue(PP_SHSHOWPASTESPECIAL))
	PROP_BOOL(IDS_CFGSHCOPYTO_STRING, GetConfig()->GetBoolValue(PP_SHSHOWCOPYTO))
	PROP_BOOL(IDS_CFGSHMOVETO_STRING, GetConfig()->GetBoolValue(PP_SHSHOWMOVETO))
	PROP_BOOL(IDS_CFGSHCMTOSPECIAL_STRING, GetConfig()->GetBoolValue(PP_SHSHOWCOPYMOVETO))
	PROP_BOOL(IDS_CFGSHSHOWFREESPACE_STRING, GetConfig()->GetBoolValue(PP_SHSHOWFREESPACE))
	PROP_BOOL(IDS_CFGSHSHOWICONS_STRING, GetConfig()->GetBoolValue(PP_SHSHOWSHELLICONS))
	PROP_BOOL(IDS_CFGSHOVERRIDEDRAG_STRING, GetConfig()->GetBoolValue(PP_SHUSEDRAGDROP))
	PROP_COMBO(IDS_CFGOVERRIDEDEFACTION_STRING, IDS_CFGACTIONS_STRING, GetConfig()->GetIntValue(PP_SHDEFAULTACTION));

	PROP_SEPARATOR(IDS_PROCESSINGTHREAD_STRING)
	PROP_BOOL(IDS_AUTOCOPYREST_STRING, GetConfig()->GetBoolValue(PP_CMUSEAUTOCOMPLETEFILES))
	PROP_BOOL(IDS_SETDESTATTRIB_STRING, GetConfig()->GetBoolValue(PP_CMSETDESTATTRIBUTES))
	PROP_BOOL(IDS_SETDESTTIME_STRING, GetConfig()->GetBoolValue(PP_CMSETDESTDATE))
	PROP_BOOL(IDS_PROTECTROFILES_STRING, GetConfig()->GetBoolValue(PP_CMPROTECTROFILES))
	PROP_UINT(IDS_LIMITOPERATIONS_STRING, GetConfig()->GetIntValue(PP_CMLIMITMAXOPERATIONS))
	PROP_BOOL(IDS_READSIZEBEFOREBLOCK_STRING, GetConfig()->GetBoolValue(PP_CMREADSIZEBEFOREBLOCKING))
	PROP_COMBO(IDS_SHOWVISUALFEEDBACK_STRING, IDS_FEEDBACKTYPE_STRING, GetConfig()->GetIntValue(PP_CMSHOWVISUALFEEDBACK))
	PROP_BOOL(IDS_USETIMEDDIALOGS_STRING, GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK))
	PROP_UINT(IDS_TIMEDDIALOGINTERVAL_STRING, GetConfig()->GetIntValue(PP_CMFEEDBACKTIME))
	PROP_BOOL(IDS_AUTORETRYONERROR_STRING, GetConfig()->GetBoolValue(PP_CMAUTORETRYONERROR))
	PROP_UINT(IDS_AUTORETRYINTERVAL_STRING, GetConfig()->GetIntValue(PP_CMAUTORETRYINTERVAL))
	PROP_COMBO(IDS_DEFAULTPRIORITY_STRING, MakeCompoundString(IDS_PRIORITY0_STRING, 7, _T("!")), PriorityToIndex(GetConfig()->GetIntValue(PP_CMDEFAULTPRIORITY)))
	PROP_BOOL(IDS_CFGDISABLEPRIORITYBOOST_STRING, GetConfig()->GetBoolValue(PP_CMDISABLEPRIORITYBOOST))
	PROP_BOOL(IDS_DELETEAFTERFINISHED_STRING, GetConfig()->GetBoolValue(PP_CMDELETEAFTERFINISHED))
	PROP_BOOL(IDS_CREATELOGFILES_STRING, GetConfig()->GetBoolValue(PP_CMCREATELOG))

	// Buffer
	PROP_SEPARATOR(IDS_OPTIONSBUFFER_STRING)
	PROP_BOOL(IDS_AUTODETECTBUFFERSIZE_STRING, GetConfig()->GetBoolValue(PP_BFUSEONLYDEFAULT))
	PROP_CUSTOM_UINT(IDS_DEFAULTBUFFERSIZE_STRING, GetConfig()->GetIntValue(PP_BFDEFAULT), &CustomPropertyCallbackProc, 0)
	PROP_CUSTOM_UINT(IDS_ONEDISKBUFFERSIZE_STRING, GetConfig()->GetIntValue(PP_BFONEDISK), &CustomPropertyCallbackProc, 1)
	PROP_CUSTOM_UINT(IDS_TWODISKSBUFFERSIZE_STRING, GetConfig()->GetIntValue(PP_BFTWODISKS), &CustomPropertyCallbackProc, 2)
	PROP_CUSTOM_UINT(IDS_CDBUFFERSIZE_STRING, GetConfig()->GetIntValue(PP_BFCD), &CustomPropertyCallbackProc, 3)
	PROP_CUSTOM_UINT(IDS_LANBUFFERSIZE_STRING, GetConfig()->GetIntValue(PP_BFLAN), &CustomPropertyCallbackProc, 4)
	PROP_BOOL(IDS_USENOBUFFERING_STRING, GetConfig()->GetBoolValue(PP_BFUSENOBUFFERING))
	PROP_UINT(IDS_LARGEFILESMINSIZE_STRING, GetConfig()->GetIntValue(PP_BFBOUNDARYLIMIT))


	// Sounds
	PROP_SEPARATOR(IDS_SOUNDS_STRING)
	PROP_BOOL(IDS_PLAYSOUNDS_STRING, GetConfig()->GetBoolValue(PP_SNDPLAYSOUNDS))
	GetConfig()->GetStringValue(PP_SNDERRORSOUNDPATH, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	PROP_PATH(IDS_SOUNDONERROR_STRING, IDS_SOUNDSWAVFILTER_STRING, strPath)
	GetConfig()->GetStringValue(PP_SNDFINISHEDSOUNDPATH, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	PROP_PATH(IDS_SOUNDONFINISH_STRING, IDS_SOUNDSWAVFILTER_STRING, strPath)

	PROP_SEPARATOR(IDS_CFGSHORTCUTS_STRING)
	PROP_CUSTOM_UINT(IDS_CFGSCCOUNT_STRING, m_cvShortcuts.size(), &ShortcutsPropertyCallbackProc, 0)

	PROP_SEPARATOR(IDS_CFGRECENT_STRING)
	PROP_CUSTOM_UINT(IDS_CFGRPCOUNT_STRING, m_cvRecent.size(), &RecentPropertyCallbackProc, 0)

 /*	PROP_SEPARATOR(IDS_CFGLOGFILE_STRING)
	PROP_BOOL(IDS_CFGENABLELOGGING_STRING, GetConfig()->GetBoolValue(PP_LOGENABLELOGGING))
	PROP_BOOL(IDS_CFGLIMITATION_STRING, GetConfig()->GetBoolValue(PP_LOGLIMITATION))
	PROP_UINT(IDS_CFGMAXLIMIT_STRING, GetConfig()->GetIntValue(PP_LOGMAXLIMIT))
	PROP_BOOL(IDS_CFGLOGPRECISELIMITING_STRING, GetConfig()->GetBoolValue(PP_LOGPRECISELIMITING))
	PROP_UINT(IDS_CFGTRUNCBUFFERSIZE_STRING, GetConfig()->GetIntValue(PP_LOGTRUNCBUFFERSIZE))*/
}

void COptionsDlg::ApplyProperties()
{
	// counter
	int iPosition=0;

	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_PCLIPBOARDMONITORING, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_PMONITORSCANINTERVAL, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_PRELOADAFTERRESTART, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_PSHUTDOWNAFTREFINISHED, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_PTIMEBEFORESHUTDOWN, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_PFORCESHUTDOWN, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_PAUTOSAVEINTERVAL, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_PPROCESSPRIORITYCLASS, IndexToPriorityClass(GetIndexProp(iPosition++)));
	GetConfig()->SetStringValue(PP_PAUTOSAVEDIRECTORY, GetStringProp(iPosition++));
	GetConfig()->SetStringValue(PP_PPLUGINSDIR, GetStringProp(iPosition++));
	GetConfig()->SetStringValue(PP_PHELPDIR, GetStringProp(iPosition++));
	// language
	PCTSTR pszSrc=m_vld.at(GetIndexProp(iPosition++)).GetFilename(true);
	if (_tcsnicmp(pszSrc, GetApp()->GetProgramPath(), _tcslen(GetApp()->GetProgramPath())) == 0)
	{
		// replace the first part of path with <PROGRAM>
		TCHAR szData[_MAX_PATH];
		_sntprintf(szData, _MAX_PATH, _T("<PROGRAM>%s"), pszSrc+_tcslen(GetApp()->GetProgramPath()));
		GetConfig()->SetStringValue(PP_PLANGUAGE, szData);
	}
	else
		GetConfig()->SetStringValue(PP_PLANGUAGE, pszSrc);
	GetConfig()->SetStringValue(PP_PLANGDIR, GetStringProp(iPosition++));

	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetIntValue(PP_STATUSREFRESHINTERVAL, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_STATUSSHOWDETAILS, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_STATUSAUTOREMOVEFINISHED, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_MVSHOWFILENAMES, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_MVSHOWSINGLETASKS, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_MVREFRESHINTERVAL, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_MVAUTOSHOWWHENRUN, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_MVAUTOHIDEWHENEMPTY, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_MVUSESMOOTHPROGRESS, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_FDEXTENDEDVIEW, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_FDWIDTH, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_FDHEIGHT, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_FDSHORTCUTLISTSTYLE, GetIndexProp(iPosition++));
	GetConfig()->SetBoolValue(PP_FDIGNORESHELLDIALOGS, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_SHSHOWCOPY, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWMOVE, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWCOPYMOVE, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWPASTE, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWPASTESPECIAL, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWCOPYTO, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWMOVETO, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWCOPYMOVETO, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWFREESPACE, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHSHOWSHELLICONS, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_SHUSEDRAGDROP, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_SHDEFAULTACTION, GetIndexProp(iPosition++));

	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_CMUSEAUTOCOMPLETEFILES, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMSETDESTATTRIBUTES, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMSETDESTDATE, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMPROTECTROFILES, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_CMLIMITMAXOPERATIONS, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMREADSIZEBEFOREBLOCKING, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_CMSHOWVISUALFEEDBACK, GetIndexProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMUSETIMEDFEEDBACK, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_CMFEEDBACKTIME, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMAUTORETRYONERROR, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_CMAUTORETRYINTERVAL, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_CMDEFAULTPRIORITY, IndexToPriority(GetIndexProp(iPosition++)));
	GetConfig()->SetBoolValue(PP_CMDISABLEPRIORITYBOOST, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMDELETEAFTERFINISHED, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_CMCREATELOG, GetBoolProp(iPosition++));

	// Buffer
	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_BFUSEONLYDEFAULT, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_BFDEFAULT, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_BFONEDISK, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_BFTWODISKS, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_BFCD, GetUintProp(iPosition++));
	GetConfig()->SetIntValue(PP_BFLAN, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_BFUSENOBUFFERING, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_BFBOUNDARYLIMIT, GetUintProp(iPosition++));

	// log file
/*	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_LOGENABLELOGGING, GetBoolProp(iPosition++));
	GetConfig()->SetBoolValue(PP_LOGLIMITATION, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_LOGMAXLIMIT, GetUintProp(iPosition++));
	GetConfig()->SetBoolValue(PP_LOGPRECISELIMITING, GetBoolProp(iPosition++));
	GetConfig()->SetIntValue(PP_LOGTRUNCBUFFERSIZE, GetUintProp(iPosition++));*/

	// Sounds
	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetBoolValue(PP_SNDPLAYSOUNDS, GetBoolProp(iPosition++));
	GetConfig()->SetStringValue(PP_SNDERRORSOUNDPATH, GetStringProp(iPosition++));
	GetConfig()->SetStringValue(PP_SNDFINISHEDSOUNDPATH, GetStringProp(iPosition++));

	// shortcuts & recent paths
	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetStringArrayValue(PP_SHORTCUTS, &m_cvShortcuts);
	
	SKIP_SEPARATOR(iPosition)
	GetConfig()->SetStringArrayValue(PP_RECENTPATHS, &m_cvRecent);

	GetConfig()->Save();
}

void COptionsDlg::OnCancel() 
{
	SendClosingNotify();
	CHLanguageDialog::OnCancel();
}

void COptionsDlg::SendClosingNotify()
{
	GetParent()->PostMessage(WM_CONFIGNOTIFY);
}

CString COptionsDlg::MakeCompoundString(UINT uiBase, int iCount, LPCTSTR lpszSeparator)
{
	_tcscpy(m_szBuffer, GetResManager()->LoadString(uiBase+0));
	for (int i=1;i<iCount;i++)
	{
		_tcscat(m_szBuffer, lpszSeparator);
		_tcscat(m_szBuffer, GetResManager()->LoadString(uiBase+i));
	}

	return CString((PCTSTR)m_szBuffer);
}

bool COptionsDlg::GetBoolProp(int iPosition)
{
	m_ctlProperties.GetProperty(iPosition, &m_iSel);
	return m_iSel != 0;
}

UINT COptionsDlg::GetUintProp(int iPosition)
{
	m_ctlProperties.GetProperty(iPosition, &m_strTemp);
	return _ttoi(m_strTemp);
}

CString COptionsDlg::GetStringProp(int iPosition)
{
	m_ctlProperties.GetProperty(iPosition, &m_strTemp);
	return m_strTemp;
}

int COptionsDlg::GetIndexProp(int iPosition)
{
	m_ctlProperties.GetProperty(iPosition, &m_iSel);
	return m_iSel;
}

void COptionsDlg::OnApplyButton() 
{
	// kill focuses
	m_ctlProperties.HideControls();

	ApplyProperties();
}

void COptionsDlg::OnLanguageChanged(WORD /*wOld*/, WORD /*wNew*/)
{
	m_ctlProperties.Reinit();

	// set attributes
	m_ctlProperties.SetBkColor(RGB(255, 255, 255));
	m_ctlProperties.SetTextColor(RGB(80, 80, 80));
	m_ctlProperties.SetTextHighlightColor(RGB(80,80,80));
	m_ctlProperties.SetHighlightColor(RGB(200, 200, 200));
	m_ctlProperties.SetPropertyBkColor(RGB(255,255,255));
	m_ctlProperties.SetPropertyTextColor(RGB(0,0,0));
	m_ctlProperties.SetLineStyle(RGB(74,109,132), PS_SOLID);

	FillPropertyList();
}
