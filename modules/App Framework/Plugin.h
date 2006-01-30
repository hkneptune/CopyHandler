/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2003 Ixen Gerthannes (ixen@interia.pl)

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
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "PluginCore.h"
#include "ExceptionEx.h"
#include "afxmt.h"

typedef bool(*PFNINIT)(PFNEXTINFOPROC);
typedef bool(*PFNUNINIT)();
typedef bool(*PFNGETINFO)(_PLUGININFO*);

// error codes for plugins
#define PLERR_UNKNOWN		0
#define PLERR_LOAD			1
#define PLERR_UNLOAD		2
#define PLERR_INIT			3
#define PLERR_UNINIT		4

#define THROW_PLUGINEXCEPTIONEX(str_reason, filename, app_code, last_error) throw new CFileExceptionEx(__FILE__, __LINE__, __FUNCTION__, str_reason, filename, app_code, last_error)

class CPluginException : public CExceptionEx
{
public:
	CPluginException(PCTSTR pszSrcFile, DWORD dwLine, PCTSTR pszFunc, PCTSTR pszReason, PCTSTR pszFilename, DWORD dwReason=PLERR_UNKNOWN, DWORD dwLastError=0) : CExceptionEx(pszSrcFile, dwLine, pszFunc, pszReason, dwReason, dwLastError) { SetFilename(pszFilename); };
	CPluginException(PCTSTR pszSrcFile, DWORD dwLine, PCTSTR pszFunc, TCHAR* pszReason, PCTSTR pszFilename, DWORD dwReason=PLERR_UNKNOWN, DWORD dwLastError=0) : CExceptionEx(pszSrcFile, dwLine, pszFunc, pszReason, dwReason, dwLastError) { SetFilename(pszFilename); };
	virtual ~CPluginException() { delete [] m_pszFilename; };

	virtual int RegisterInfo(__EXCPROPINFO* pInfo)
	{
		// if the pInfo is null - return count of a needed props
		if (pInfo == NULL)
			return 1+CExceptionEx::RegisterInfo(NULL);

		// call base class RegisterInfo
		size_t tIndex=CExceptionEx::RegisterInfo(pInfo);

		// function has to register the info to be displayed (called from within GetInfo)
		RegisterProp(pInfo+tIndex+0, _T("Plugin path"), PropType::dtPtrToString, &m_pszFilename);

		return 1;
	};


protected:
	void SetFilename(PCTSTR pszFilename) { if (pszFilename) { m_pszFilename=new TCHAR[_tcslen(pszFilename)+1]; _tcscpy(m_pszFilename, pszFilename); } else m_pszFilename=NULL; };

public:
	TCHAR *m_pszFilename;		// name of the plugin
};

// plugin load state definitions
#define PLGS_NOTLOADED	0	/* plugin wasn't loaded yet */
#define PLGS_LOADED		1	/* plugin has been loaded and is ready to use */
#define PLGS_PARTIAL	2	/* plugin was once loaded and then unloaded->partial data is available */

class CPlugin
{
public:
	CPlugin(bool bInternal=false);
	~CPlugin();

	void Load(PCTSTR pszName, PFNEXTINFOPROC pfn);		// opens .dll file as a plugin & initializes it
	void Unload(bool bCompletely=false);	// unloads .dll (partially or completely) & deinitializes

	virtual void Init(PFNEXTINFOPROC pfn);		// initializes this plugin with some callback
	virtual void Uninit();						// uninitializes this plugin

	virtual bool GetInfo(_PLUGININFO* pInfo);		// gets info about this plugin (use cache when possible)

	BYTE GetLoadState() { return m_byLoadState; };
	HMODULE GetModule() { return m_hModule; };
	bool IsInternal() { return m_bInternal; };

	void BeginUsage();						// should be used before using function of a plugin
	void EndUsage();						// should be used after using any plugin func

	// for use with internal plugins
	void SetPluginInfo(_PLUGININFO* pData) { m_ppdData=pData; };	// sets the pointer

protected:
	virtual bool LoadExports();				// loads addresses of exported functions (from dll)
	virtual void FreePluginData() { };		// for use in derived classes - have to free internal entries


protected:
	TCHAR *m_pszFilename;		// for external plugins - path to plugin, internal - handle to app hModule
	HMODULE m_hModule;			// dll address/app address
	bool m_bInternal;			// is this an internal plugin
	BYTE m_byLoadState;			// load state of this plugin
	bool m_bNeedChange;			// plugin was temporarily loaded from partial state - unload at the first occasion

	LONG volatile m_lLockCount;		// for usage with partial state (specifies current count of loads)
	CCriticalSection m_cs;

	PFNEXTINFOPROC m_pfnCallback;	// address of a callback

	// cached data
	_PLUGININFO* m_ppdData;		// cached plugin data

	// exports addresses
	PFNINIT m_pfnInit;
	PFNUNINIT m_pfnUninit;
	PFNGETINFO m_pfnGetInfo;
};

#endif