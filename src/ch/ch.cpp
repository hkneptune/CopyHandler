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

#include "CfgProperties.h"
#include "MainWnd.h"
#include "../common/ipcstructs.h"

#pragma warning(push)
#pragma warning(disable: 4091)
	#include <Dbghelp.h>
#pragma warning(pop)

#include "CrashDlg.h"
#include "../common/version.h"
#include "TCommandLineParser.h"
#include "../libchcore/TStringSet.h"
#include "../libchcore/TSimpleTimer.h"
#include "../libchcore/SerializerTrace.h"
#include <boost/container/flat_map.hpp>
#include "../libchcore/TSQLiteTaskSchema.h"
#include "../libchcore/TSQLiteSerializer.h"
#include "../libchcore/ISerializerContainer.h"
#include "../libchcore/ISerializerRowData.h"
#include "../libchcore/TFileInfo.h"
#include "TMsgBox.h"
#include "../libchcore/TWin32ErrorFormatter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp

BEGIN_MESSAGE_MAP(CCopyHandlerApp, CWinApp)
END_MESSAGE_MAP()

int iCount=98;
unsigned short msg[]={	0x40d1, 0x4dcd, 0x8327, 0x6cdf, 0xb912, 0x017b, 0xac78, 0x1e04, 0x5637,
						0x1822, 0x0a69, 0x1b40, 0x4169, 0x504d, 0x80ff, 0x6c2f, 0xa612, 0x017e,
						0xac84, 0x1c8c, 0x552b, 0x16e2, 0x0a4b, 0x1dc0, 0x4179, 0x4d0d, 0x8337,
						0x6c4f, 0x6512, 0x0169, 0xac46, 0x1db4, 0x55cf, 0x1652, 0x0a0b, 0x1480,
						0x40fd, 0x470d, 0x822f, 0x6b8f, 0x6512, 0x013a, 0xac5a, 0x1d24, 0x5627,
						0x1762, 0x0a27, 0x1240, 0x40f5, 0x3f8d, 0x8187, 0x690f, 0x6e12, 0x011c,
						0xabc0, 0x1cc4, 0x567f, 0x1952, 0x0a51, 0x1cc0, 0x4175, 0x3ccd, 0x8377,
						0x6c5f, 0x6512, 0x0186, 0xac7c, 0x1e04, 0x5677, 0x1412, 0x0a61, 0x1d80,
						0x4169, 0x4e8d, 0x838f, 0x6c0f, 0xb212, 0x0132, 0xac7e, 0x1e54, 0x5593,
						0x1412, 0x0a15, 0x3dc0, 0x4195, 0x4e0d, 0x832f, 0x67ff, 0x9812, 0x0186,
						0xac6e, 0x1e4c, 0x5667, 0x1942, 0x0a47, 0x1f80, 0x4191, 0x4f8d };

int iOffCount=12;
unsigned char off[]={ 2, 6, 3, 4, 8, 0, 1, 3, 2, 4, 1, 6 };
unsigned short _hash[]={ 0x3fad, 0x34cd, 0x7fff, 0x65ff, 0x4512, 0x0112, 0xabac, 0x1abc, 0x54ab, 0x1212, 0x0981, 0x0100 };

/////////////////////////////////////////////////////////////////////////////
// The one and only CCopyHandlerApp object

CCopyHandlerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp construction

// main routing function - routes any message that comes from modules
void ResManCallback(unsigned int uiMsg)
{
	theApp.OnResManNotify(uiMsg);
}

void ConfigPropertyChangedCallback(const chcore::TStringSet& setPropNames, void* /*pParam*/)
{
	theApp.OnConfigNotify(setPropNames);
}

CCopyHandlerApp::CCopyHandlerApp() :
	m_pMainWindow(NULL)
{
	// this is the one-instance application
	InitProtection();
}

CCopyHandlerApp::~CCopyHandlerApp()
{
	if (m_pMainWindow)
	{
		((CMainWnd*)m_pMainWindow)->DestroyWindow();
		delete m_pMainWindow;
		m_pMainWnd=NULL;
	}
}

CCopyHandlerApp& GetApp()
{
	return theApp;
}

ictranslate::CResourceManager& GetResManager()
{
	return ictranslate::CResourceManager::Acquire();
}

chcore::TConfig& GetConfig()
{
	static chcore::TConfig tCfg;
	return tCfg;
}

int MsgBox(UINT uiID, UINT nType, UINT nIDHelp)
{
	return AfxMessageBox(GetResManager().LoadString(uiID), nType, nIDHelp);
}

bool CCopyHandlerApp::UpdateHelpPaths()
{
	bool bChanged=false;		// flag that'll be returned - if the paths has changed

	// generate the current filename - uses language from config
	CString strHelpPath = ExpandPath(_T("<PROGRAM>\\Help\\"));
	strHelpPath += GetResManager().m_ld.GetHelpName();
	if(strHelpPath != m_pszHelpFilePath)
	{
		free((void*)m_pszHelpFilePath);
		m_pszHelpFilePath = _tcsdup(strHelpPath);
		bChanged=true;
	}

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp initialization

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	// Step1 - should not fail - prepare some more unique crash name, create under the path where ch data exists
	TCHAR szPath[_MAX_PATH];
	HRESULT hResult = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath);
	if(FAILED(hResult))
		_tcscpy(szPath, _T("c:\\"));

	CString strPath(szPath);
	// make sure to create the required directories if they does not exist
	strPath += _T("\\Copy Handler");
	CreateDirectory(strPath, NULL);
	strPath += _T("\\Dumps");
	CreateDirectory(strPath, NULL);

	// current date
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	TCHAR szName[_MAX_PATH];
	_sntprintf(szName, _MAX_PATH, _T("%s\\ch_crashdump-%s-%I64u-%s.dmp"), (PCTSTR)strPath, _T(PRODUCT_VERSION), (unsigned long long)_time64(NULL),
#ifdef _WIN64
		_T("64")
#else
		_T("32")
#endif
		);
	szName[_MAX_PATH - 1] = _T('\0');

	// Step 2 - create the crash dump in case anything happens later
	bool bResult = false;
	HANDLE hFile = CreateFile(szName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION mei;
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = ExceptionInfo;
		mei.ClientPointers = TRUE;

		if(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithProcessThreadData, &mei, NULL, NULL))
			bResult = true;
	}

	CloseHandle(hFile);

	CCrashDlg dlgCrash(bResult, szName);
	dlgCrash.DoModal();

	return EXCEPTION_EXECUTE_HANDLER;
}

//#define DO_TEST

BOOL CCopyHandlerApp::InitInstance()
{
#ifdef DO_TEST
	using namespace chcore;

	DeleteFile(_T("C:\\Users\\ixen\\AppData\\Local\\Copy Handler\\Tasks\\test.sqlite"));
	TSQLiteTaskSchemaPtr spTaskSchema(new TSQLiteTaskSchema);

	TSQLiteSerializer serializer(PathFromString(_T("C:\\Users\\ixen\\AppData\\Local\\Copy Handler\\Tasks\\test.sqlite")), spTaskSchema);
	//TSQLiteSerializer serializer(PathFromString(_T(":memory:")), spTaskSchema);

	TSimpleTimer timer(true);

	{
		ISerializerContainerPtr spContainer = serializer.GetContainer(_T("scanned_files"));

		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		TFileInfo::InitColumns(rColumns);

		const size_t rel_path = rColumns.GetColumnIndex(_T("rel_path"));
		const size_t base_path_id = rColumns.GetColumnIndex(_T("base_path_id"));
		const size_t attr = rColumns.GetColumnIndex(_T("attr"));
		const size_t size = rColumns.GetColumnIndex(_T("size"));
		const size_t time_created = rColumns.GetColumnIndex(_T("time_created"));
		const size_t time_last_write = rColumns.GetColumnIndex(_T("time_last_write"));
		const size_t time_last_access = rColumns.GetColumnIndex(_T("time_last_access"));
		const size_t flags = rColumns.GetColumnIndex(_T("flags"));

		TString strPath(_T("C:\\Users\\ixen\\AppData\\Local\\Copy Handler\\Tasks\\sometask.xxx"));
		TSmartPath path(PathFromString(_T("C:\\Users\\ixen\\AppData\\Local\\Copy Handler\\Tasks\\sometask.xxx")));

		for(object_id_t oidIndex = 0; oidIndex < 200000; ++oidIndex)
		{
			ISerializerRowData& rRow = spContainer->GetRow(oidIndex, true);
			rRow.SetValue(rel_path, path);	//C:\\Users\\ixen\\AppData\\Local\\Copy Handler\\Tasks\\sometask.xxx
			rRow.SetValue(base_path_id, 24735275ull);
			rRow.SetValue(attr, 0x56533234ul);
			rRow.SetValue(size, 0x565332340897ff12ull);
			rRow.SetValue(time_created, 0x565332340897ff12ull);
			rRow.SetValue(time_last_write, 0x565122340897ff12ull);
			rRow.SetValue(time_last_access, 0x565517840897ff12ull);
			rRow.SetValue(flags, 0x56551114u);
		}

		unsigned long long ullFillTime = timer.Checkpoint(); ullFillTime;
		GTRACE1(_T("***** [FillTime]: %I64u ms\n"), ullFillTime);

		serializer.Flush();
		unsigned long long ullFlushTime = timer.Checkpoint(); ullFlushTime;
		GTRACE1(_T("***** [FlushTime]: %I64u ms\n"), ullFlushTime);

		spContainer.reset();
		unsigned long long ullDeleteContainerTime = timer.Checkpoint(); ullDeleteContainerTime;
		GTRACE1(_T("***** [DeleteContainer]: %I64u ms\n"), ullDeleteContainerTime);
	}

	unsigned long long ullDestructTime = timer.Checkpoint(); ullDestructTime;
	GTRACE1(_T("***** [DestructTime]: %I64u ms\n"), ullDestructTime);

	return FALSE;
#else

	// ================================= Crash handling =======================================
	SetUnhandledExceptionFilter(&MyUnhandledExceptionFilter);

	// ================================= Handle command line ==================================
	// parse the command line this early, so we can support as much options as possible in the future
	// (i.e. override the defaults used below)
	if (!ParseCommandLine())
		return FALSE;

	// ================================= Configuration ========================================
	CString strPath;
	CString strCfgPath;
	CString strLogPath;

	// note that the GetProgramDataPath() below should create a directory; ExpandPath() could
	// depend on the directory to be created earlier
	if(!GetProgramDataPath(strPath))
	{
		AfxMessageBox(_T("Cannot initialize Copy Handler (data path cannot be established)."), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	strCfgPath = strPath + _T("\\ch.xml");

	// initialize configuration file
	chcore::TConfig& rCfg = GetConfig();
	rCfg.ConnectToNotifier(ConfigPropertyChangedCallback, NULL);

	// read the configuration
	try
	{
		GetConfig().Read(strCfgPath);
	}
	catch(...)
	{
	}

	// ================================= Logging ========================================
	// initialize the global log file if it is requested by configuration file
	strLogPath = strPath + _T("\\ch.log");

	chcore::TLogger& rLogger = chcore::TLogger::Acquire();
	try
	{
		rLogger.init(strLogPath, GetPropValue<PP_LOGMAXSIZE>(rCfg), GetPropValue<PP_LOGLEVEL>(rCfg), false, false);
		rLogger.Enable(GetPropValue<PP_LOGENABLELOGGING>(rCfg));
	}
	catch(...)
	{
		BOOST_ASSERT(false);
	}

	LOG_INFO(_T("============================ Initializing Copy Handler ============================"));
	LOG_INFO(_T(""));

	// ================================= COM ========================================
	LOG_INFO(_T("Initializing COM"));

	HRESULT hResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hResult))
	{
		CString strMsg;
		strMsg.Format(_T("Cannot initialize COM, the application will now exit (result = 0x%lx)"), hResult);

		LOG_ERROR(strMsg);
		AfxMessageBox(strMsg, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// ================================= Resource manager ========================================
	LOG_INFO(_T("Initializing resource manager..."));

	ictranslate::CResourceManager& rResManager = ictranslate::CResourceManager::Acquire();

	// set current language
	rResManager.Init(AfxGetInstanceHandle());
	rResManager.SetCallback(ResManCallback);
	GetPropValue<PP_PLANGUAGE>(rCfg, strPath);
	TRACE(_T("Help path=%s\n"), strPath);
	if(!rResManager.SetLanguage(ExpandPath(strPath)))
	{
		TCHAR szData[2048];
		_sntprintf(szData, 2048, _T("Couldn't find the language file specified in configuration file:\n%s\nPlease correct this path to point the language file to use.\nProgram will now exit."), (PCTSTR)strPath);
		LOG_ERROR(szData);
		AfxMessageBox(szData, MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	UpdateHelpPaths();

	// for dialogs
	ictranslate::CLanguageDialog::SetResManager(&rResManager);

	EnableHtmlHelp();

	// ================================= Checking for running instances of CH ========================================
	// check instance - return false if it's the second one
	LOG_INFO(_T("Checking for other running instances of Copy Handler"));
	if(!IsFirstInstance())
	{
		// if there is a command line specified, send it to the existing instance
		if(m_cmdLineParser.HasCommandLineParams())
		{
			HWND hWnd = ::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
			if(hWnd == NULL)
			{
				// cannot pass command line to running ch
				LOG_ERROR(_T("Cannot determine running CH's window. Cannot pass command line there."));
				MsgBox(IDS_COMMAND_LINE_FAILED_STRING, MB_OK | MB_ICONERROR);
				return FALSE;
			}

			CString strCmdLine = ::GetCommandLine();

			COPYDATASTRUCT cds;
			cds.dwData = eCDType_CommandLineArguments;
			cds.cbData = (DWORD)(strCmdLine.GetLength() + 1) * sizeof(wchar_t);
			cds.lpData = (void*)(PCTSTR)strCmdLine;

			// send a message to ch
			if(::SendMessage(hWnd, WM_COPYDATA, NULL, reinterpret_cast<LPARAM>(&cds)) == 0)
			{
				LOG_ERROR(_T("Command line was not processed properly at the running CH's instance."));
				MsgBox(IDS_COMMAND_LINE_FAILED_STRING, MB_OK | MB_ICONERROR);
			}

			return FALSE;
		}
		else
		{
			LOG_WARNING(_T("Other instance of Copy Handler is already running. Exiting."));
			MsgBox(IDS_ONECOPY_STRING, MB_OK | MB_ICONWARNING);
			return FALSE;
		}
	}

	// ================================= Common controls ========================================
	LOG_INFO(_T("Initializing GUI common controls"));

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	if(!InitCommonControlsEx(&InitCtrls))
	{
		LOG_ERROR(_T("Cannot initialize common controls."));
		MsgBox(IDS_ERROR_INITIALIZING_COMMON_CONTROLS, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if(!AfxInitRichEdit2())
	{
		LOG_ERROR(_T("Cannot initialize rich edit control."));
		MsgBox(IDS_ERROR_INITIALIZING_RICH_EDIT_CONTROL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// ================================= Shell extension ========================================
	LOG_INFO(_T("Checking shell extension compatibility"));

	InitShellExtension();

	// ================================= Initial settings ========================================
	LOG_INFO(_T("Applying initial settings"));

	// set this process priority class
	HANDLE hProcess = GetCurrentProcess();
	::SetPriorityClass(hProcess, GetPropValue<PP_PPROCESSPRIORITYCLASS>(rCfg));

#ifndef _DEBUG		// for easier writing the program - doesn't collide with std CH
	// set "run with system" registry settings
	SetAutorun(GetPropValue<PP_PRELOADAFTERRESTART>(rCfg));
#endif

	// ================================= Main window ========================================
	LOG_INFO(_T("Creating main application window"));
	// create main window
	m_pMainWindow=new CMainWnd;
	if (!((CMainWnd*)m_pMainWindow)->Create())
		return FALSE;				// will be deleted at destructor

	m_pMainWnd = m_pMainWindow;
	CWinApp::InitInstance();

	LOG_INFO(_T("Copy Handler initialized successfully"));

	return TRUE;
#endif
}

bool CCopyHandlerApp::ParseCommandLine()
{
	CString strError;
	try
	{
		m_cmdLineParser.ParseCommandLine(::GetCommandLine());
	}
	catch (const std::exception& e)
	{
		strError = e.what();
	}

	if (!strError.IsEmpty())
	{
		CString strFmt;
		strFmt.Format(_T("Error processing command line options. Reason: %s"), (PCTSTR) strError);
		AfxMessageBox(strFmt, MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}

void CCopyHandlerApp::InitShellExtension()
{
	// validate ch version against extension version
	CString strExtensionStringVersion;
	long lExtensionVersion = 0;
	INT_PTR iDlgResult = IDNO;

	chcore::TConfig& rConfig = GetConfig();

	int iDoNotShowAgain_Unregistered = GetPropValue<PP_HIDE_SHELLEXT_UNREGISTERED>(rConfig);
	int iDoNotShowAgain_VersionMismatch = GetPropValue<PP_HIDE_SHELLEXT_VERSIONMISMATCH>(rConfig);
	bool bDontShow = false;

	// first try to just enable the extension (assume that it has already been registered)
	HRESULT hResult = m_tShellExtClient.EnableExtensionIfCompatible(PRODUCT_VERSION1 << 24 | PRODUCT_VERSION2 << 16 | PRODUCT_VERSION3 << 8 | PRODUCT_VERSION4, lExtensionVersion, strExtensionStringVersion);
	if(FAILED(hResult))
	{
		CString strMsg;
		strMsg.Format(_T("Shell extension is not registered."));
		LOG_WARNING(strMsg);

		switch(iDoNotShowAgain_Unregistered)
		{
		case eDNS_HideAndRegister:
			iDlgResult = IDYES;
			break;

		case eDNS_HideAndDontRegister:
			iDlgResult = IDNO;
			break;

		case eDNS_AlwaysShow:
		default:
			{
				iDlgResult = TMsgBox::MsgBox(IDS_SHELL_EXTENSION_UNREGISTERED_STRING, TMsgBox::eYesNo, TMsgBox::eIcon_Warning, IDS_DO_NOT_SHOW_AGAIN_STRING, &bDontShow);
				if(bDontShow)
					SetPropValue<PP_HIDE_SHELLEXT_UNREGISTERED>(rConfig, (iDlgResult == IDYES) ? eDNS_HideAndRegister : eDNS_HideAndDontRegister);
			}
		}
	}
	else if(hResult == S_FALSE)
	{
		CString strMsg;
		strMsg.Format(_T("Shell extension has different version (0x%lx) than Copy Handler (0x%lx)."), (unsigned long)lExtensionVersion, (unsigned long)(PRODUCT_VERSION1 << 24 | PRODUCT_VERSION2 << 16 | PRODUCT_VERSION3 << 8 | PRODUCT_VERSION4));
		LOG_WARNING(strMsg);

		switch(iDoNotShowAgain_VersionMismatch)
		{
		case eDNS_HideAndRegister:
			iDlgResult = IDYES;
			break;

		case eDNS_HideAndDontRegister:
			iDlgResult = IDNO;
			break;

		case eDNS_AlwaysShow:
		default:
			{
				iDlgResult = TMsgBox::MsgBox(IDS_SHELL_EXTENSION_MISMATCH_STRING, TMsgBox::eYesNo, TMsgBox::eIcon_Warning, IDS_DO_NOT_SHOW_AGAIN_STRING, &bDontShow);
				if(bDontShow)
					SetPropValue<PP_HIDE_SHELLEXT_VERSIONMISMATCH>(rConfig, (iDlgResult == IDYES) ? eDNS_HideAndRegister : eDNS_HideAndDontRegister);
			}
		}
	}

	if(bDontShow)
		rConfig.Write();

	// we didn't succeed, but want to fix this
	if(iDlgResult == IDYES)
	{
		// try to register the extension
		RegisterShellExtension();
	}
}

void CCopyHandlerApp::RegisterShellExtension() 
{
	CString strPath = CString(GetProgramPath()) + _T("\\");

#ifdef _WIN64
	strPath += _T("chext64.dll");
#else
	strPath += _T("chext.dll");
#endif

	long lExtensionVersion = 0;
	CString strExtensionVersion;

	HRESULT hResult = m_tShellExtClient.RegisterShellExtDll(strPath, PRODUCT_VERSION1 << 24 | PRODUCT_VERSION2 << 16 | PRODUCT_VERSION3 << 8 | PRODUCT_VERSION4,
															lExtensionVersion, strExtensionVersion);
	if(FAILED(hResult))
	{
		// normal failure
		chcore::TString strError = chcore::TWin32ErrorFormatter::FormatWin32ErrorCode(hResult, true);

		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_REGISTERERR_STRING));
		fmt.SetParam(_T("%errno"), (unsigned long)hResult);
		fmt.SetParam(_T("%errdesc"), strError.c_str());
		AfxMessageBox(fmt, MB_ICONERROR | MB_OK);
	}
	else if(hResult == S_FALSE)
	{
		// registered ok, but incompatible versions - probably restart required
		CString strMsg;
		strMsg.Format(_T("Registration succeeded, but still the shell extension has different version (0x%lx) than Copy Handler (0x%lx)."), (unsigned long)lExtensionVersion, (unsigned long)(PRODUCT_VERSION1 << 24 | PRODUCT_VERSION2 << 16 | PRODUCT_VERSION3 << 8 | PRODUCT_VERSION4));
		LOG_WARNING(strMsg);

		MsgBox(IDS_SHELL_EXTENSION_REGISTERED_MISMATCH_STRING, MB_ICONWARNING | MB_OK);
	}
	else if(hResult == S_OK)
		MsgBox(IDS_REGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
}

void CCopyHandlerApp::UnregisterShellExtension() 
{
	CString strPath = CString(GetProgramPath()) + _T("\\");

#ifdef _WIN64
	strPath += _T("chext64.dll");
#else
	strPath += _T("chext.dll");
#endif

	HRESULT hResult = m_tShellExtClient.UnRegisterShellExtDll(strPath);
	if(hResult == TYPE_E_REGISTRYACCESS)
	{
		MsgBox(IDS_CHEXT_ALREADY_UNREGISTERED, MB_ICONINFORMATION | MB_OK);
	}
	else if(FAILED(hResult))
	{
		chcore::TString strError = chcore::TWin32ErrorFormatter::FormatWin32ErrorCode(hResult, true);

		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_UNREGISTERERR_STRING));
		fmt.SetParam(_T("%errno"), (unsigned long)hResult);
		fmt.SetParam(_T("%errdesc"), strError.c_str());

		AfxMessageBox(fmt, MB_ICONERROR | MB_OK);
	}
	else if(hResult == S_OK)
		MsgBox(IDS_UNREGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
}

void CCopyHandlerApp::OnConfigNotify(const chcore::TStringSet& setPropNames)
{
	// is this language
	if(setPropNames.HasValue(PropData<PP_PLANGUAGE>::GetPropertyName()))
	{
		// update language in resource manager
		CString strPath;
		GetPropValue<PP_PLANGUAGE>(GetConfig(), strPath);
		GetResManager().SetLanguage(ExpandPath(strPath));
	}

	if(setPropNames.HasValue(PropData<PP_LOGENABLELOGGING>::GetPropertyName()))
	{
		chcore::TLogger& rLogger = chcore::TLogger::Acquire();

		rLogger.Enable(GetPropValue<PP_LOGENABLELOGGING>(GetConfig()));
	}

	if(setPropNames.HasValue(PropData<PP_LOGLEVEL>::GetPropertyName()))
	{
		chcore::TLogger& rLogger = chcore::TLogger::Acquire();

		rLogger.set_log_level(GetPropValue<PP_LOGLEVEL>(GetConfig()));
	}

	if(setPropNames.HasValue(PropData<PP_LOGMAXSIZE>::GetPropertyName()))
	{
		chcore::TLogger& rLogger = chcore::TLogger::Acquire();

		rLogger.set_max_size(GetPropValue<PP_LOGMAXSIZE>(GetConfig()));
	}
}

void CCopyHandlerApp::OnResManNotify(UINT uiType)
{
	if (uiType == RMNT_LANGCHANGE)
	{
		// language has been changed - close the current help file
		if (UpdateHelpPaths())
			HtmlHelp(NULL, HH_CLOSE_ALL);
	}
}

HWND CCopyHandlerApp::HHelp(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD_PTR dwData)
{
	PCTSTR pszPath=NULL;
	WIN32_FIND_DATA wfd;
	HANDLE handle=::FindFirstFile(m_pszHelpFilePath, &wfd);
	if (handle != INVALID_HANDLE_VALUE)
	{
		pszPath=m_pszHelpFilePath;
		::FindClose(handle);
	}

	if (pszPath == NULL)
		return NULL;

	if (pszFile != NULL)
	{
		TCHAR szAdd[2*_MAX_PATH];
		_tcscpy(szAdd, pszPath);
		_tcscat(szAdd, pszFile);
		return ::HtmlHelp(hwndCaller, szAdd, uCommand, dwData);
	}
	else
		return ::HtmlHelp(hwndCaller, pszPath, uCommand, dwData);
}

void CCopyHandlerApp::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	switch (nCmd)
	{
	case HH_DISPLAY_TOPIC:
	case HH_HELP_CONTEXT:
		{
			HHelp(GetDesktopWindow(), NULL, nCmd, dwData);
			break;
		}
	case HH_CLOSE_ALL:
		::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, NULL);
		break;
	case HH_DISPLAY_TEXT_POPUP:
		{
			HELPINFO* pHelp=(HELPINFO*)dwData;
			if ( pHelp->dwContextId == 0 || pHelp->iCtrlId == 0
				|| ::GetWindowContextHelpId((HWND)pHelp->hItemHandle) == 0)
				return;

			HH_POPUP hhp;
			hhp.cbStruct=sizeof(HH_POPUP);
			hhp.hinst=NULL;
			hhp.idString=(pHelp->dwContextId & 0xffff);
			hhp.pszText=NULL;
			hhp.pt=pHelp->MousePos;
			hhp.pt.y+=::GetSystemMetrics(SM_CYCURSOR)/2;
			hhp.clrForeground=(COLORREF)-1;
			hhp.clrBackground=(COLORREF)-1;
			hhp.rcMargins.left=-1;
			hhp.rcMargins.right=-1;
			hhp.rcMargins.top=-1;
			hhp.rcMargins.bottom=-1;
			hhp.pszFont=_T("Tahoma, 8, , ");

			TCHAR szPath[_MAX_PATH];
			_sntprintf(szPath, _MAX_PATH, _T("::/%lu.txt"), (DWORD)((pHelp->dwContextId >> 16) & 0x7fff));
			HHelp(GetDesktopWindow(), szPath, HH_DISPLAY_TEXT_POPUP, (DWORD_PTR)&hhp);

			break;
		}
	}
}

int CCopyHandlerApp::ExitInstance()
{
	LOG_INFO(_T("Pre-exit step - releasing shell extension"));

	m_tShellExtClient.Close();

	LOG_INFO(_T("Pre-exit step - uninitializing COM"));
	CoUninitialize();

	LOG_INFO(_T("============================ Leaving Copy Handler ============================"));

	return __super::ExitInstance();
}
