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
#include "ch.h"
#include "resource.h"
#include "OptionsDlg.h"
#include "BufferSizeDlg.h"
#include "ShortcutsDlg.h"
#include "RecentDlg.h"
#include <assert.h>
#include "structs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool COptionsDlg::m_bLock=false;

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	:ictranslate::CLanguageDialog(COptionsDlg::IDD, pParent, &m_bLock)
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROPERTIES_LIST, m_ctlProperties);
}

BEGIN_MESSAGE_MAP(COptionsDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_APPLY_BUTTON, OnApplyButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

// properties handling macros
#define PROP_SEPARATOR(text)\
	m_ctlProperties.AddString(text)

#define PROP_BOOL(text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_COMBO_LIST, IDS_BOOLTEXT_STRING, (value))

#define PROP_UINT(text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_TEXT, _itot(boost::numeric_cast<int>((value)), m_szBuffer, 10), 0)

#define PROP_COMBO(text, prop_text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_COMBO_LIST, prop_text, boost::numeric_cast<int>((value)))

#define PROP_DIR(text, prop_text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_DIR, (value)+CString(GetResManager().LoadString(prop_text)), 0)

#define PROP_PATH(text, prop_text, value)\
	m_ctlProperties.AddString(text, ID_PROPERTY_PATH, (value)+CString(GetResManager().LoadString(prop_text)), 0)

#define PROP_CUSTOM_UINT(text, value, callback, param)\
	m_ctlProperties.AddString(text, ID_PROPERTY_CUSTOM, CString(_itot(boost::numeric_cast<int>((value)), m_szBuffer, 10)), callback, this, param, 0)

#define SKIP_SEPARATOR(pos)\
	pos++

BOOL COptionsDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	AddResizableControl(IDC_PROPERTIES_LIST, 0.0, 0.0, 1.0, 1.0);
	AddResizableControl(IDOK, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDCANCEL, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_APPLY_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDHELP, 1.0, 1.0, 0.0, 0.0);

	InitializeResizableControls();

	m_ctlProperties.Init();

	// copy shortcut and recent paths
	chcore::TConfig& rConfig = GetConfig();

	m_cvRecent.clear();
	GetPropValue<PP_RECENTPATHS>(rConfig, m_cvRecent);

	m_cvShortcuts.clear();
	GetPropValue<PP_SHORTCUTS>(rConfig, m_cvShortcuts);

	_tcscpy(m_szLangPath, _T("<PROGRAM>\\Langs\\"));
	GetApp().ExpandPath(m_szLangPath);

	GetResManager().Scan(m_szLangPath, &m_vld);

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

	chcore::TBufferSizes tBufferSizes(pDlg->GetBoolProp(iIndex - iParam - 1),
		pDlg->GetUintProp(iIndex - iParam + 5),
		pDlg->GetUintProp(iIndex - iParam),
		pDlg->GetUintProp(iIndex - iParam + 1),
		pDlg->GetUintProp(iIndex - iParam + 2),
		pDlg->GetUintProp(iIndex - iParam + 3),
		pDlg->GetUintProp(iIndex - iParam + 4));

	CBufferSizeDlg dlg(&tBufferSizes, (chcore::TBufferSizes::EBufferType)iParam);
	if (dlg.DoModal() == IDOK)
	{
		tBufferSizes = dlg.GetBufferSizes();

		PROPERTYITEM* pItem;
		TCHAR xx[32];

		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam-1));
		pItem->nPropertySelected=(tBufferSizes.IsOnlyDefault() ? 1 : 0);
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam));
		pItem->csProperties.SetAt(0, _itot(tBufferSizes.GetDefaultSize(), xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+1));
		pItem->csProperties.SetAt(0, _itot(tBufferSizes.GetOneDiskSize(), xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+2));
		pItem->csProperties.SetAt(0, _itot(tBufferSizes.GetTwoDisksSize(), xx, 10));
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex-iParam+3));
		pItem->csProperties.SetAt(0, _itot(tBufferSizes.GetCDSize(), xx, 10));
		pItem = (PROPERTYITEM*) pList->GetAt(pList->FindIndex(iIndex - iParam + 4));
		pItem->csProperties.SetAt(0, _itot(tBufferSizes.GetLANSize(), xx, 10));
		pItem = (PROPERTYITEM*) pList->GetAt(pList->FindIndex(iIndex - iParam + 5));
		pItem->csProperties.SetAt(0, _itot(tBufferSizes.GetBufferCount(), xx, 10));
	}
}

void ShortcutsPropertyCallbackProc(LPVOID lpParam, int /*iParam*/, CPtrList* pList, int iIndex)
{
	COptionsDlg* pDlg=static_cast<COptionsDlg*>(lpParam);

	CShortcutsDlg dlg;
	dlg.m_cvShortcuts = pDlg->m_cvShortcuts;
	dlg.m_pcvRecent=&pDlg->m_cvRecent;
	if (dlg.DoModal() == IDOK)
	{
		// restore shortcuts to pDlg->cvShortcuts
		pDlg->m_cvShortcuts = dlg.m_cvShortcuts;

		// property list
		TCHAR szBuf[32];
		PROPERTYITEM* pItem;
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex));
		pItem->csProperties.SetAt(0, _ui64tot(pDlg->m_cvShortcuts.size(), szBuf, 10));
	}
}

void RecentPropertyCallbackProc(LPVOID lpParam, int /*iParam*/, CPtrList* pList, int iIndex)
{
	COptionsDlg* pDlg=static_cast<COptionsDlg*>(lpParam);

	CRecentDlg dlg;
	dlg.m_cvRecent = pDlg->m_cvRecent;
	if (dlg.DoModal() == IDOK)
	{
		// restore
		pDlg->m_cvRecent = dlg.m_cvRecent;

		// property list
		TCHAR szBuf[32];
		PROPERTYITEM* pItem;
		pItem = (PROPERTYITEM*)pList->GetAt(pList->FindIndex(iIndex));
		pItem->csProperties.SetAt(0, _ui64tot(pDlg->m_cvRecent.size(), szBuf, 10));
	}
}

void COptionsDlg::OnOK() 
{
	// kill focuses
	m_ctlProperties.HideControls();

	ApplyProperties();

	SendClosingNotify();
	CLanguageDialog::OnOK();
}

void COptionsDlg::FillPropertyList()
{
	CString strPath;

	// load settings
	PROP_SEPARATOR(IDS_PROGRAM_STRING);
	PROP_BOOL(IDS_CLIPBOARDMONITORING_STRING, GetPropValue<PP_PCLIPBOARDMONITORING>(GetConfig()));
	PROP_UINT(IDS_CLIPBOARDINTERVAL_STRING, GetPropValue<PP_PMONITORSCANINTERVAL>(GetConfig()));
	PROP_BOOL(IDS_AUTORUNPROGRAM_STRING, GetPropValue<PP_PRELOADAFTERRESTART>(GetConfig()));
	PROP_COMBO(IDS_CFG_CHECK_FOR_UPDATES_FREQUENCY, IDS_UPDATE_FREQUENCIES, GetPropValue<PP_PCHECK_FOR_UPDATES_FREQUENCY>(GetConfig()));
	PROP_COMBO(IDS_CFG_UPDATECHANNEL, IDS_CFGUPDATECHANNELITEMS_STRING, GetPropValue<PP_PUPDATECHANNEL>(GetConfig()));
	PROP_BOOL(IDS_AUTOSHUTDOWN_STRING, GetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig()));
	PROP_UINT(IDS_SHUTDOWNTIME_STRING, GetPropValue<PP_PTIMEBEFORESHUTDOWN>(GetConfig()));
	PROP_COMBO(IDS_FORCESHUTDOWN_STRING, IDS_FORCESHUTDOWNVALUES_STRING, GetPropValue<PP_PFORCESHUTDOWN>(GetConfig()) ? 1 : 0);
	PROP_UINT(IDS_AUTOSAVEINTERVAL_STRING, GetPropValue<PP_PAUTOSAVEINTERVAL>(GetConfig()));
	PROP_COMBO(IDS_CFGPRIORITYCLASS_STRING, IDS_CFGPRIORITYCLASSITEMS_STRING, PriorityClassToIndex(boost::numeric_cast<int>(GetPropValue<PP_PPROCESSPRIORITYCLASS>(GetConfig()))));
	PROP_DIR(IDS_TEMPFOLDER_STRING, IDS_TEMPFOLDERCHOOSE_STRING, strPath);

	// lang
	CString strLangs;
	size_t stIndex=0;
	for (vector<ictranslate::CLangData>::iterator it=m_vld.begin();it != m_vld.end();++it)
	{
		strLangs+=(*it).GetLangName();
		strLangs+=_T("!");
		if (_tcsicmp((*it).GetFilename(true), GetResManager().m_ld.GetFilename(true)) == 0)
			stIndex = it - m_vld.begin();
	}
	strLangs.TrimRight(_T('!'));

	PROP_COMBO(IDS_LANGUAGE_STRING, strLangs, stIndex);

	/////////////////
	PROP_SEPARATOR(IDS_STATUSWINDOW_STRING);
	PROP_UINT(IDS_REFRESHSTATUSINTERVAL_STRING, GetPropValue<PP_STATUSREFRESHINTERVAL>(GetConfig()));
	PROP_BOOL(IDS_STATUSAUTOREMOVE_STRING, GetPropValue<PP_STATUSAUTOREMOVEFINISHED>(GetConfig()));

	PROP_SEPARATOR(IDS_MINIVIEW_STRING);
	PROP_BOOL(IDS_SHOWFILENAMES_STRING, GetPropValue<PP_MVSHOWFILENAMES>(GetConfig()));
	PROP_BOOL(IDS_SHOWSINGLETASKS_STRING, GetPropValue<PP_MVSHOWSINGLETASKS>(GetConfig()));
	PROP_UINT(IDS_MINIVIEWREFRESHINTERVAL_STRING, GetPropValue<PP_MVREFRESHINTERVAL>(GetConfig()));
	PROP_BOOL(IDS_MINIVIEWSHOWAFTERSTART_STRING, GetPropValue<PP_MVAUTOSHOWWHENRUN>(GetConfig()));
	PROP_BOOL(IDS_MINIVIEWAUTOHIDE_STRING, GetPropValue<PP_MVAUTOHIDEWHENEMPTY>(GetConfig()));
	PROP_BOOL(IDS_MINIVIEWSMOOTHPROGRESS_STRING, GetPropValue<PP_MVUSESMOOTHPROGRESS>(GetConfig()));

	PROP_SEPARATOR(IDS_CFGFOLDERDIALOG_STRING);
	PROP_BOOL(IDS_CFGFDEXTVIEW_STRING, GetPropValue<PP_FDEXTENDEDVIEW>(GetConfig()));
	PROP_UINT(IDS_CFGFDWIDTH_STRING, GetPropValue<PP_FDWIDTH>(GetConfig()));
	PROP_UINT(IDS_CFGFDHEIGHT_STRING, GetPropValue<PP_FDHEIGHT>(GetConfig()));
	PROP_COMBO(IDS_CFGFDSHORTCUTS_STRING, IDS_CFGFDSHORTCUTSSTYLES_STRING, GetPropValue<PP_FDSHORTCUTLISTSTYLE>(GetConfig()));
	PROP_BOOL(IDS_CFGFDIGNOREDIALOGS_STRING, GetPropValue<PP_FDIGNORESHELLDIALOGS>(GetConfig()));

	PROP_SEPARATOR(IDS_CFGSHELL_STRING);
	PROP_BOOL(IDS_CFGSHCOPY_STRING, GetPropValue<PP_SHSHOWCOPY>(GetConfig()));
	PROP_BOOL(IDS_CFGSHMOVE_STRING, GetPropValue<PP_SHSHOWMOVE>(GetConfig()));
	PROP_BOOL(IDS_CFGSHCMSPECIAL_STRING, GetPropValue<PP_SHSHOWCOPYMOVE>(GetConfig()));
	PROP_BOOL(IDS_CFGSHPASTE_STRING, GetPropValue<PP_SHSHOWPASTE>(GetConfig()));
	PROP_BOOL(IDS_CFGSHPASTESPECIAL_STRING, GetPropValue<PP_SHSHOWPASTESPECIAL>(GetConfig()));
	PROP_BOOL(IDS_CFGSHCOPYTO_STRING, GetPropValue<PP_SHSHOWCOPYTO>(GetConfig()));
	PROP_BOOL(IDS_CFGSHMOVETO_STRING, GetPropValue<PP_SHSHOWMOVETO>(GetConfig()));
	PROP_BOOL(IDS_CFGSHCMTOSPECIAL_STRING, GetPropValue<PP_SHSHOWCOPYMOVETO>(GetConfig()));
	PROP_BOOL(IDS_CFGSHSHOWFREESPACE_STRING, GetPropValue<PP_SHSHOWFREESPACE>(GetConfig()));
	PROP_BOOL(IDS_CFGSHSHOWICONS_STRING, GetPropValue<PP_SHSHOWSHELLICONS>(GetConfig()));
	PROP_BOOL(IDS_CFGSHINTERCEPTDRAG_STRING, GetPropValue<PP_SHINTERCEPTDRAGDROP>(GetConfig()));
	PROP_BOOL(IDS_CFGINTERCEPTKEYACTION_STRING, GetPropValue<PP_SHINTERCEPTKEYACTIONS>(GetConfig()));
	PROP_BOOL(IDS_CFGINTERCEPTCONTEXTMENU_STRING, GetPropValue<PP_SHINTERCEPTCTXMENUACTIONS>(GetConfig()));

	PROP_SEPARATOR(IDS_DIALOGS_SHOW_HIDE_STRING);
	PROP_COMBO(IDS_SHELLEXT_REGISTER_SHOWHIDE_STRING, MakeCompoundString(IDS_ALWAYS_SHOW_STRING, 3, _T("!")), GetPropValue<PP_HIDE_SHELLEXT_UNREGISTERED>(GetConfig()));
	PROP_COMBO(IDS_SHELLEXT_VERSIONMISMATCH_SHOWHIDE_STRING, MakeCompoundString(IDS_ALWAYS_SHOW_STRING, 3, _T("!")), GetPropValue<PP_HIDE_SHELLEXT_VERSIONMISMATCH>(GetConfig()));

	PROP_SEPARATOR(IDS_PROCESSINGTHREAD_STRING);
	PROP_BOOL(IDS_SETDESTATTRIB_STRING, GetPropValue<PP_CMSETDESTATTRIBUTES>(GetConfig()));
	PROP_BOOL(IDS_SETDESTTIME_STRING, GetPropValue<PP_CMSETDESTDATE>(GetConfig()));
	PROP_BOOL(IDS_PROTECTROFILES_STRING, GetPropValue<PP_CMPROTECTROFILES>(GetConfig()));
	PROP_UINT(IDS_LIMITOPERATIONS_STRING, GetPropValue<PP_CMLIMITMAXOPERATIONS>(GetConfig()));
	PROP_BOOL(IDS_READSIZEBEFOREBLOCK_STRING, GetPropValue<PP_CMREADSIZEBEFOREBLOCKING>(GetConfig()));
	PROP_COMBO(IDS_DEFAULTPRIORITY_STRING, MakeCompoundString(IDS_PRIORITY0_STRING, 7, _T("!")), PriorityToIndex(boost::numeric_cast<int>(GetPropValue<PP_CMDEFAULTPRIORITY>(GetConfig()))));
	PROP_BOOL(IDS_CFGDISABLEPRIORITYBOOST_STRING, GetPropValue<PP_CMDISABLEPRIORITYBOOST>(GetConfig()));
	PROP_BOOL(IDS_DELETEAFTERFINISHED_STRING, GetPropValue<PP_CMDELETEAFTERFINISHED>(GetConfig()));

	// Buffer
	PROP_SEPARATOR(IDS_OPTIONSBUFFER_STRING);
	PROP_BOOL(IDS_AUTODETECTBUFFERSIZE_STRING, GetPropValue<PP_BFUSEONLYDEFAULT>(GetConfig()));
	PROP_CUSTOM_UINT(IDS_DEFAULTBUFFERSIZE_STRING, GetPropValue<PP_BFDEFAULT>(GetConfig()), &CustomPropertyCallbackProc, 0);
	PROP_CUSTOM_UINT(IDS_ONEDISKBUFFERSIZE_STRING, GetPropValue<PP_BFONEDISK>(GetConfig()), &CustomPropertyCallbackProc, 1);
	PROP_CUSTOM_UINT(IDS_TWODISKSBUFFERSIZE_STRING, GetPropValue<PP_BFTWODISKS>(GetConfig()), &CustomPropertyCallbackProc, 2);
	PROP_CUSTOM_UINT(IDS_CDBUFFERSIZE_STRING, GetPropValue<PP_BFCD>(GetConfig()), &CustomPropertyCallbackProc, 3);
	PROP_CUSTOM_UINT(IDS_LANBUFFERSIZE_STRING, GetPropValue<PP_BFLAN>(GetConfig()), &CustomPropertyCallbackProc, 4);
	PROP_BOOL(IDS_USENOBUFFERING_STRING, GetPropValue<PP_BFUSENOBUFFERING>(GetConfig()));
	PROP_UINT(IDS_LARGEFILESMINSIZE_STRING, GetPropValue<PP_BFBOUNDARYLIMIT>(GetConfig()));
	PROP_UINT(IDS_BUFFER_QUEUE_DEPTH, GetPropValue<PP_BFQUEUEDEPTH>(GetConfig()));

	PROP_SEPARATOR(IDS_CFGLOGFILE_STRING);
	PROP_BOOL(IDS_CFGENABLELOGGING_STRING, GetPropValue<PP_LOGENABLELOGGING>(GetConfig()));
	PROP_UINT(IDS_CFGMAXLIMIT_STRING, GetPropValue<PP_LOGMAXSIZE>(GetConfig()));
	PROP_COMBO(IDS_CFGLOGLEVEL, IDS_CFGLOGLEVEL_VALUES, GetPropValue<PP_LOGLEVEL>(GetConfig()));

	// Sounds
	PROP_SEPARATOR(IDS_SOUNDS_STRING);
	PROP_BOOL(IDS_PLAYSOUNDS_STRING, GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()));
	PROP_PATH(IDS_SOUNDONERROR_STRING, IDS_SOUNDSWAVFILTER_STRING, GetPropValue<PP_SNDERRORSOUNDPATH>(GetConfig()));
	PROP_PATH(IDS_SOUNDONFINISH_STRING, IDS_SOUNDSWAVFILTER_STRING, GetPropValue<PP_SNDFINISHEDSOUNDPATH>(GetConfig()));

	PROP_SEPARATOR(IDS_CFGSHORTCUTS_STRING);
	PROP_CUSTOM_UINT(IDS_CFGSCCOUNT_STRING, m_cvShortcuts.size(), &ShortcutsPropertyCallbackProc, 0);

	PROP_SEPARATOR(IDS_CFGRECENT_STRING);
	PROP_CUSTOM_UINT(IDS_CFGRPCOUNT_STRING, m_cvRecent.size(), &RecentPropertyCallbackProc, 0);
}

void COptionsDlg::ApplyProperties()
{
	// counter
	int iPosition=0;

	chcore::TConfig& rConfig = GetConfig();
	rConfig.DelayNotifications();

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_PCLIPBOARDMONITORING>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_PMONITORSCANINTERVAL>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_PRELOADAFTERRESTART>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_PCHECK_FOR_UPDATES_FREQUENCY>(rConfig, GetIndexProp(iPosition++));
	SetPropValue<PP_PUPDATECHANNEL>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_PSHUTDOWNAFTREFINISHED>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_PTIMEBEFORESHUTDOWN>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_PFORCESHUTDOWN>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_PAUTOSAVEINTERVAL>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_PPROCESSPRIORITYCLASS>(rConfig, IndexToPriorityClass(GetIndexProp(iPosition++)));
	// language
	PCTSTR pszSrc=m_vld.at(GetIndexProp(iPosition++)).GetFilename(true);
	if (_tcsnicmp(pszSrc, GetApp().GetProgramPath(), _tcslen(GetApp().GetProgramPath())) == 0)
	{
		// replace the first part of path with <PROGRAM>
		TCHAR szData[_MAX_PATH];
		_sntprintf(szData, _MAX_PATH, _T("<PROGRAM>%s"), pszSrc+_tcslen(GetApp().GetProgramPath()));
		SetPropValue<PP_PLANGUAGE>(rConfig, szData);
	}
	else
		SetPropValue<PP_PLANGUAGE>(rConfig, pszSrc);

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_STATUSREFRESHINTERVAL>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_STATUSAUTOREMOVEFINISHED>(rConfig, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_MVSHOWFILENAMES>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_MVSHOWSINGLETASKS>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_MVREFRESHINTERVAL>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_MVAUTOSHOWWHENRUN>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_MVAUTOHIDEWHENEMPTY>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_MVUSESMOOTHPROGRESS>(rConfig, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_FDEXTENDEDVIEW>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_FDWIDTH>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_FDHEIGHT>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_FDSHORTCUTLISTSTYLE>(rConfig, GetIndexProp(iPosition++));
	SetPropValue<PP_FDIGNORESHELLDIALOGS>(rConfig, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_SHSHOWCOPY>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWMOVE>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWCOPYMOVE>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWPASTE>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWPASTESPECIAL>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWCOPYTO>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWMOVETO>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWCOPYMOVETO>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWFREESPACE>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHSHOWSHELLICONS>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHINTERCEPTDRAGDROP>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHINTERCEPTKEYACTIONS>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SHINTERCEPTCTXMENUACTIONS>(rConfig, GetBoolProp(iPosition++));

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_HIDE_SHELLEXT_UNREGISTERED>(rConfig, GetIndexProp(iPosition++));
	SetPropValue<PP_HIDE_SHELLEXT_VERSIONMISMATCH>(rConfig, GetIndexProp(iPosition++));

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_CMSETDESTATTRIBUTES>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_CMSETDESTDATE>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_CMPROTECTROFILES>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_CMLIMITMAXOPERATIONS>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_CMREADSIZEBEFOREBLOCKING>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_CMDEFAULTPRIORITY>(rConfig, IndexToPriority(GetIndexProp(iPosition++)));
	SetPropValue<PP_CMDISABLEPRIORITYBOOST>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_CMDELETEAFTERFINISHED>(rConfig, GetBoolProp(iPosition++));

	// Buffer
	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_BFUSEONLYDEFAULT>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_BFDEFAULT>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_BFONEDISK>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_BFTWODISKS>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_BFCD>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_BFLAN>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_BFUSENOBUFFERING>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_BFBOUNDARYLIMIT>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_BFQUEUEDEPTH>(rConfig, GetUintProp(iPosition++));

	// log file
	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_LOGENABLELOGGING>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_LOGMAXSIZE>(rConfig, GetUintProp(iPosition++));
	SetPropValue<PP_LOGLEVEL>(rConfig, GetIndexProp(iPosition++));

	// Sounds
	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_SNDPLAYSOUNDS>(rConfig, GetBoolProp(iPosition++));
	SetPropValue<PP_SNDERRORSOUNDPATH>(rConfig, GetStringProp(iPosition++));
	SetPropValue<PP_SNDFINISHEDSOUNDPATH>(rConfig, GetStringProp(iPosition++));

	// shortcuts & recent paths
	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_SHORTCUTS>(rConfig, m_cvShortcuts);

	SKIP_SEPARATOR(iPosition);
	SetPropValue<PP_RECENTPATHS>(rConfig, m_cvRecent);

	rConfig.ResumeNotifications();

	rConfig.Write();
}

void COptionsDlg::OnCancel() 
{
	SendClosingNotify();
	CLanguageDialog::OnCancel();
}

void COptionsDlg::SendClosingNotify()
{
	GetParent()->PostMessage(WM_CONFIGNOTIFY);
}

CString COptionsDlg::MakeCompoundString(UINT uiBase, int iCount, LPCTSTR lpszSeparator)
{
	assert(lpszSeparator);
	if(!lpszSeparator)
		return _T("");
	_tcscpy(m_szBuffer, GetResManager().LoadString(uiBase+0));
	for (int i=1;i<iCount;i++)
	{
		_tcscat(m_szBuffer, lpszSeparator);
		_tcscat(m_szBuffer, GetResManager().LoadString(uiBase+i));
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

void COptionsDlg::OnLanguageChanged()
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
