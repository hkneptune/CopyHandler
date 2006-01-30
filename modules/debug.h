#ifndef __DEBUG_H__
#define __DEBUG_H__

// time measuring macros
#ifdef _DEBUG
	static long __dwst, __dwen;
	#define MEASURE_TIME(lines, text) __dwst=GetTickCount(); lines __dwen=GetTickCount(); TRACE(text, __dwen-__dwst);
#else
	#define MEASURE_TIME(lines, text) lines
#endif

#ifdef _DEBUG
	#define TRACELOGFONT(str, lf)\
	{\
		TRACE( str "\tlfHeight: %ld\n\tlfWidth: %ld\n\tlfEscapement: %ld\n\tlfOrientation: %ld\n\tlfWeight: %ld\n",  (lf).lfHeight, (lf).lfWidth, (lf).lfEscapement, (lf).lfOrientation, (lf).lfWeight);\
		TRACE("\tlfItalic: %d\n\tlfUnderline: %d\n\tlfStrikeOut: %d\n\tlfCharSet: %d\n\tlfOutPrecision: %d\n\tlfClipPrecision: %d\n\tlfQuality: %d\n\tlfPitchAndFamily: %d\n\tlfFaceName: %s\n", (lf).lfItalic, (lf).lfUnderline, (lf).lfStrikeOut, (lf).lfCharSet, (lf).lfOutPrecision, (lf).lfClipPrecision, (lf).lfQuality, (lf).lfPitchAndFamily, (lf).lfFaceName);\
	}
#else
	#define TRACELOGFONT(lf)
#endif

// window messages
#ifdef _DEBUG
struct __dbg_msg__
{
	UINT uiMsg;
	TCHAR *pszText;
};

//////////////////////////////////////

#define GEN_PAIR(str) { str, #str }

static __dbg_msg__ __msgs__[] = {
	GEN_PAIR(WM_NULL),
	GEN_PAIR(WM_CREATE),
	GEN_PAIR(WM_DESTROY),
	GEN_PAIR(WM_MOVE),
	GEN_PAIR(WM_SIZE),
	GEN_PAIR(WM_ACTIVATE),
	GEN_PAIR(WM_SETFOCUS),
	GEN_PAIR(WM_KILLFOCUS),
	GEN_PAIR(WM_ENABLE),
	GEN_PAIR(WM_SETREDRAW),
	GEN_PAIR(WM_SETTEXT),
	GEN_PAIR(WM_GETTEXT),
	GEN_PAIR(WM_GETTEXTLENGTH),
	GEN_PAIR(WM_PAINT),
	GEN_PAIR(WM_CLOSE),
	GEN_PAIR(WM_QUERYENDSESSION),
	GEN_PAIR(WM_QUERYOPEN),
	GEN_PAIR(WM_ENDSESSION),
	GEN_PAIR(WM_QUIT),
	GEN_PAIR(WM_ERASEBKGND),
	GEN_PAIR(WM_SYSCOLORCHANGE),
	GEN_PAIR(WM_SHOWWINDOW),
	GEN_PAIR(WM_WININICHANGE),
	GEN_PAIR(WM_DEVMODECHANGE),
	GEN_PAIR(WM_ACTIVATEAPP),
	GEN_PAIR(WM_FONTCHANGE),
	GEN_PAIR(WM_TIMECHANGE),
	GEN_PAIR(WM_CANCELMODE),
	GEN_PAIR(WM_SETCURSOR),
	GEN_PAIR(WM_MOUSEACTIVATE),
	GEN_PAIR(WM_CHILDACTIVATE),
	GEN_PAIR(WM_QUEUESYNC),
	GEN_PAIR(WM_GETMINMAXINFO),
	GEN_PAIR(WM_PAINTICON),
	GEN_PAIR(WM_ICONERASEBKGND),
	GEN_PAIR(WM_NEXTDLGCTL),
	GEN_PAIR(WM_SPOOLERSTATUS),
	GEN_PAIR(WM_DRAWITEM),
	GEN_PAIR(WM_MEASUREITEM),
	GEN_PAIR(WM_DELETEITEM),
	GEN_PAIR(WM_VKEYTOITEM),
	GEN_PAIR(WM_CHARTOITEM),
	GEN_PAIR(WM_SETFONT),
	GEN_PAIR(WM_GETFONT),
	GEN_PAIR(WM_SETHOTKEY),
	GEN_PAIR(WM_GETHOTKEY),
	GEN_PAIR(WM_QUERYDRAGICON),
	GEN_PAIR(WM_COMPAREITEM),
	{ 0x003d, "WM_GETOBJECT" },
	GEN_PAIR(WM_COMPACTING),
	GEN_PAIR(WM_COMMNOTIFY),
	GEN_PAIR(WM_WINDOWPOSCHANGING),
	GEN_PAIR(WM_WINDOWPOSCHANGED),
	GEN_PAIR(WM_POWER),
	GEN_PAIR(WM_COPYDATA),
	GEN_PAIR(WM_CANCELJOURNAL),
	GEN_PAIR(WM_NOTIFY),
	GEN_PAIR(WM_INPUTLANGCHANGEREQUEST),
	GEN_PAIR(WM_INPUTLANGCHANGE),
	GEN_PAIR(WM_TCARD),
	GEN_PAIR(WM_HELP),
	GEN_PAIR(WM_USERCHANGED),
	GEN_PAIR(WM_NOTIFYFORMAT),
	GEN_PAIR(WM_CONTEXTMENU),
	GEN_PAIR(WM_STYLECHANGING),
	GEN_PAIR(WM_STYLECHANGED),
	GEN_PAIR(WM_DISPLAYCHANGE),
	GEN_PAIR(WM_GETICON),
	GEN_PAIR(WM_SETICON),
	GEN_PAIR(WM_NCCREATE),
	GEN_PAIR(WM_NCDESTROY),
	GEN_PAIR(WM_NCCALCSIZE),
	GEN_PAIR(WM_NCHITTEST),
	GEN_PAIR(WM_NCPAINT),
	GEN_PAIR(WM_NCACTIVATE),
	GEN_PAIR(WM_GETDLGCODE),
	GEN_PAIR(WM_SYNCPAINT),
	GEN_PAIR(WM_NCMOUSEMOVE),
	GEN_PAIR(WM_NCLBUTTONDOWN),
	GEN_PAIR(WM_NCLBUTTONUP),
	GEN_PAIR(WM_NCLBUTTONDBLCLK),
	GEN_PAIR(WM_NCRBUTTONDOWN),
	GEN_PAIR(WM_NCRBUTTONUP),
	GEN_PAIR(WM_NCRBUTTONDBLCLK),
	GEN_PAIR(WM_NCMBUTTONDOWN),
	GEN_PAIR(WM_NCMBUTTONUP),
	GEN_PAIR(WM_NCMBUTTONDBLCLK),
	{ 0x00AB, "WM_NCXBUTTONDOWN" },
	{ 0x00AC, "WM_NCXBUTTONUP" },
	{ 0x00AD, "WM_NCXBUTTONDBLCLK" },
	{ 0x00FF, "WM_INPUT" },
	GEN_PAIR(WM_KEYFIRST),
	GEN_PAIR(WM_KEYDOWN),
	GEN_PAIR(WM_KEYUP),
	GEN_PAIR(WM_CHAR),
	GEN_PAIR(WM_DEADCHAR),
	GEN_PAIR(WM_SYSKEYDOWN),
	GEN_PAIR(WM_SYSKEYUP),
	GEN_PAIR(WM_SYSCHAR),
	GEN_PAIR(WM_SYSDEADCHAR),
	GEN_PAIR(WM_UNICHAR),
	GEN_PAIR(WM_KEYLAST),
	GEN_PAIR(WM_IME_STARTCOMPOSITION),
	GEN_PAIR(WM_IME_ENDCOMPOSITION),
	GEN_PAIR(WM_IME_COMPOSITION),
	GEN_PAIR(WM_IME_KEYLAST),
	GEN_PAIR(WM_INITDIALOG),
	GEN_PAIR(WM_COMMAND),
	GEN_PAIR(WM_SYSCOMMAND),
	GEN_PAIR(WM_TIMER),
	GEN_PAIR(WM_HSCROLL),
	GEN_PAIR(WM_VSCROLL),
	GEN_PAIR(WM_INITMENU),
	GEN_PAIR(WM_INITMENUPOPUP),
	GEN_PAIR(WM_MENUSELECT),
	GEN_PAIR(WM_MENUCHAR),
	GEN_PAIR(WM_ENTERIDLE),
	{ 0x0122, "WM_MENURBUTTONUP" },
	{ 0x0123, "WM_MENUDRAG" },
	{ 0x0124, "WM_MENUGETOBJECT" },
	{ 0x0125, "WM_UNINITMENUPOPUP" },
	{ 0x0126, "WM_MENUCOMMAND" },
	{ 0x0127, "WM_CHANGEUISTATE" },
	{ 0x0128, "WM_UPDATEUISTATE" },
	{ 0x0129, "WM_QUERYUISTATE" },
	GEN_PAIR(WM_CTLCOLORMSGBOX),
	GEN_PAIR(WM_CTLCOLOREDIT),
	GEN_PAIR(WM_CTLCOLORLISTBOX),
	GEN_PAIR(WM_CTLCOLORBTN),
	GEN_PAIR(WM_CTLCOLORDLG),
	GEN_PAIR(WM_CTLCOLORSCROLLBAR),
	GEN_PAIR(WM_CTLCOLORSTATIC),
	GEN_PAIR(WM_MOUSEFIRST),
	GEN_PAIR(WM_MOUSEMOVE),
	GEN_PAIR(WM_LBUTTONDOWN),
	GEN_PAIR(WM_LBUTTONUP),
	GEN_PAIR(WM_LBUTTONDBLCLK),
	GEN_PAIR(WM_RBUTTONDOWN),
	GEN_PAIR(WM_RBUTTONUP),
	GEN_PAIR(WM_RBUTTONDBLCLK),
	GEN_PAIR(WM_MBUTTONDOWN),
	GEN_PAIR(WM_MBUTTONUP),
	GEN_PAIR(WM_MBUTTONDBLCLK),
	GEN_PAIR(WM_MOUSEWHEEL),
	{ 0x020B, "WM_XBUTTONDOWN" },
	{ 0x020C, "WM_XBUTTONUP" },
	{ 0x020D, "WM_XBUTTONDBLCLK" },
	GEN_PAIR(WM_MOUSELAST),
	GEN_PAIR(WM_MOUSELAST),
	GEN_PAIR(WM_MOUSELAST),
	GEN_PAIR(WM_PARENTNOTIFY),
	GEN_PAIR(WM_ENTERMENULOOP),
	GEN_PAIR(WM_EXITMENULOOP),
	GEN_PAIR(WM_NEXTMENU),
	GEN_PAIR(WM_SIZING),
	GEN_PAIR(WM_CAPTURECHANGED),
	GEN_PAIR(WM_MOVING),
	GEN_PAIR(WM_POWERBROADCAST),
	GEN_PAIR(WM_DEVICECHANGE),
	GEN_PAIR(WM_MDICREATE),
	GEN_PAIR(WM_MDIDESTROY),
	GEN_PAIR(WM_MDIACTIVATE),
	GEN_PAIR(WM_MDIRESTORE),
	GEN_PAIR(WM_MDINEXT),
	GEN_PAIR(WM_MDIMAXIMIZE),
	GEN_PAIR(WM_MDITILE),
	GEN_PAIR(WM_MDICASCADE),
	GEN_PAIR(WM_MDIICONARRANGE),
	GEN_PAIR(WM_MDIGETACTIVE),
	GEN_PAIR(WM_MDISETMENU),
	GEN_PAIR(WM_ENTERSIZEMOVE),
	GEN_PAIR(WM_EXITSIZEMOVE),
	GEN_PAIR(WM_DROPFILES),
	GEN_PAIR(WM_MDIREFRESHMENU),
	GEN_PAIR(WM_IME_SETCONTEXT),
	GEN_PAIR(WM_IME_NOTIFY),
	GEN_PAIR(WM_IME_CONTROL),
	GEN_PAIR(WM_IME_COMPOSITIONFULL),
	GEN_PAIR(WM_IME_SELECT),
	GEN_PAIR(WM_IME_CHAR),
	{ 0x0288, "WM_IME_REQUEST" },
	GEN_PAIR(WM_IME_KEYDOWN),
	GEN_PAIR(WM_IME_KEYUP),
	GEN_PAIR(WM_MOUSEHOVER),
	GEN_PAIR(WM_MOUSELEAVE),
	{ 0x02A0, "WM_NCMOUSEHOVER" },
	{ 0x02A2, "WM_NCMOUSELEAVE" },
	{ 0x02B1, "WM_WTSSESSION_CHANGE" },
	{ 0x02C0, "WM_TABLET_FIRST" },
	{ 0x02DF, "WM_TABLET_LAST" },
	GEN_PAIR(WM_CUT),
	GEN_PAIR(WM_COPY),
	GEN_PAIR(WM_PASTE),
	GEN_PAIR(WM_CLEAR),
	GEN_PAIR(WM_UNDO),
	GEN_PAIR(WM_RENDERFORMAT),
	GEN_PAIR(WM_RENDERALLFORMATS),
	GEN_PAIR(WM_DESTROYCLIPBOARD),
	GEN_PAIR(WM_DRAWCLIPBOARD),
	GEN_PAIR(WM_PAINTCLIPBOARD),
	GEN_PAIR(WM_VSCROLLCLIPBOARD),
	GEN_PAIR(WM_SIZECLIPBOARD),
	GEN_PAIR(WM_ASKCBFORMATNAME),
	GEN_PAIR(WM_CHANGECBCHAIN),
	GEN_PAIR(WM_HSCROLLCLIPBOARD),
	GEN_PAIR(WM_QUERYNEWPALETTE),
	GEN_PAIR(WM_PALETTEISCHANGING),
	GEN_PAIR(WM_PALETTECHANGED),
	GEN_PAIR(WM_HOTKEY),
	GEN_PAIR(WM_PRINT),
	GEN_PAIR(WM_PRINTCLIENT),
	{ 0x0319, "WM_APPCOMMAND" },
	{ 0x031A, "WM_THEMECHANGED" },
	GEN_PAIR(WM_HANDHELDFIRST),
	GEN_PAIR(WM_HANDHELDLAST),
	GEN_PAIR(WM_AFXFIRST),
	GEN_PAIR(WM_AFXLAST),
	GEN_PAIR(WM_PENWINFIRST),
	GEN_PAIR(WM_PENWINLAST),
	GEN_PAIR(WM_APP),
	GEN_PAIR(WM_USER)
						};
/////////////////////////////////

static char* UINTToMsg(UINT uiMsg, char* szBuffer)
{
	int iCount=sizeof(__msgs__)/(sizeof(UINT)+sizeof(char*));
	for (int i=0;i<iCount;i++)
	{
		if (uiMsg == (__msgs__[i]).uiMsg)
		{
			strcpy(szBuffer, (__msgs__[i]).pszText);
			return szBuffer;
		}
	}
	_ltoa(uiMsg, szBuffer, 10);
	return szBuffer;
}

	#define TRACEMSG(str, msg, wparam, lparam) { char szBuf[64]; TRACE(str "uiMsg: %s (%lu), wParam: %lu, lParam:%lu\n", UINTToMsg(msg, szBuf), msg, wparam, lparam); }
#else
	#define TRACEMSG(str, msg, wparam, lparam)
#endif


#endif