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
#include "stdafx.h"
#include "plugin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPlugin::CPlugin(bool bInternal)
{
	m_hModule=NULL;
	m_bInternal=bInternal;
	m_byLoadState=PLGS_NOTLOADED;
	m_lLockCount=0;
	m_pszFilename=NULL;
	m_bNeedChange=false;

	// cache
	m_ppdData=NULL;

	// exports addresses
	m_pfnInit=NULL;
	m_pfnUninit=NULL;
	m_pfnGetInfo=NULL;
}

CPlugin::~CPlugin()
{
	// free all
	Unload(true);		// unload if loaded

	// free filename
	delete [] m_pszFilename;
	m_pszFilename=NULL;
}

void CPlugin::Load(PCTSTR pszName, PFNEXTINFOPROC pfn)
{
	if (pszName && m_byLoadState != PLGS_NOTLOADED)
	{
		// trying to load a plugin to already initialized class
		if (m_bInternal)
			THROW_PLUGINEXCEPTIONEX(_T("Trying to load a plugin to already initialized class (internal plugin)."), NULL, PLERR_LOAD, 0);
		else
			THROW_PLUGINEXCEPTIONEX(_T("Trying to load a plugin to already initialized class (external plugin)."), pszName, PLERR_LOAD, 0);
	}

	// check if loading is needed
	if (!pszName)
	{
		if (m_byLoadState == PLGS_LOADED)
			return;
		else if (m_byLoadState == PLGS_NOTLOADED)
			THROW_PLUGINEXCEPTIONEX(_T("Cannot re-load plugin (with NULL path) when it wasn't loaded before."), NULL, PLERR_LOAD, 0);		// not loaded and pszName==NULL - this shouldn't happen
	}

	// load - copy data to internal members
	if (m_bInternal)
	{
		// copy only if there is anything to copy
		if (pszName)
		{
			m_hModule=(HMODULE)pszName;		// should be HMODULE of current app
			m_pszFilename=(TCHAR*)pszName;
		}
		else
			m_hModule=(HMODULE)m_pszFilename;
		m_byLoadState=PLGS_LOADED;
	}
	else
	{
		// remember the name of this plugin (path)
		if (pszName)
		{
			delete [] m_pszFilename;
			m_pszFilename=new TCHAR[_tcslen(pszName)+1];
			_tcscpy(m_pszFilename, pszName);
		}

		// try to open
		m_hModule=LoadLibrary(m_pszFilename);
		if (!m_hModule || !LoadExports())
		{
			m_byLoadState=PLGS_NOTLOADED;
			if (m_bInternal)
				THROW_PLUGINEXCEPTIONEX(_T("Cannot load an internal plugin - probably LoadExports failed."), NULL, PLERR_LOAD, GetLastError());
			else
				THROW_PLUGINEXCEPTIONEX(_T("Cannot load an external plugin - either LoadLibrary failed or LoadExports failed."), pszName, PLERR_LOAD, GetLastError());
		}
		else
			m_byLoadState=PLGS_LOADED;
	}

	// now initialize the plugin
	Init(pfn);
}

void CPlugin::Unload(bool bCompletely)
{
	if (m_byLoadState == PLGS_NOTLOADED)
		return;

	// uninit if loaded
	if (m_byLoadState == PLGS_LOADED)
		Uninit();

	// release the loaded library (if partial - doesn't need to)
	if (!m_bInternal && m_byLoadState == PLGS_LOADED)
	{
		if (!FreeLibrary(m_hModule))
			THROW_PLUGINEXCEPTIONEX(_T("Cannot release loaded plugin (FreeLibrary failed)."), m_pszFilename, PLERR_UNLOAD, GetLastError());
	}
	m_hModule=NULL;

	// release the rest of data if completely
	if (bCompletely)
	{
		// filename
		if (!m_bInternal)
			delete [] m_pszFilename;
		m_pszFilename=NULL;

		// cached data
		delete m_ppdData;
		m_ppdData=NULL;

		// free other data
		FreePluginData();
	}

	// determine load state
	m_byLoadState=(BYTE)(bCompletely ? PLGS_NOTLOADED : PLGS_PARTIAL);
}

bool CPlugin::LoadExports()
{
	_ASSERT(m_hModule);

	m_pfnInit=(PFNINIT)GetProcAddress(m_hModule, "Init");
	if (!m_pfnInit)
	{
		TRACE("Last Error: %lu\n!!!", GetLastError());
		return false;
	}
	m_pfnUninit=(PFNUNINIT)GetProcAddress(m_hModule, "Uninit");
	if (!m_pfnUninit)
		return false;
	m_pfnGetInfo=(PFNGETINFO)GetProcAddress(m_hModule, "GetInfo");
	if (!m_pfnUninit)
		return false;

	return true;
}

void CPlugin::Init(PFNEXTINFOPROC pfn)
{
	_ASSERT(m_hModule);
	if (!m_bInternal)
	{
		// store pfn for future use
		if (pfn)
			m_pfnCallback=pfn;

		// call Init
		TRACE("External CPlugin::Init()\n");
		if (!(*m_pfnInit)(m_pfnCallback))
			THROW_PLUGINEXCEPTIONEX(_T("External plugin initialization failed."), m_pszFilename, PLERR_INIT, 0);
	}
	else
		TRACE("Internal CPlugin::Init() - does nothing\n");
}

void CPlugin::Uninit()
{
	if (!m_hModule)
		return;

	if (!m_bInternal)
	{
		TRACE("External CPlugin::Uninit()\n");
		if (!(*m_pfnUninit)())
			THROW_PLUGINEXCEPTIONEX(_T("External plugin deinitialization failed."), m_pszFilename, PLERR_UNINIT, 0);
	}
	else
		TRACE("Internal CPlugin::Uninit() - does nothing\n");
}

bool CPlugin::GetInfo(_PLUGININFO* pInfo)
{
	// alloc new _PLUGININFO
	if (m_ppdData)
	{
		*pInfo=*m_ppdData;		// get data from cache
		return true;
	}
	else
	{
		if (!m_bInternal)
		{
			BeginUsage();
			// call
			bool bRes=(*m_pfnGetInfo)(pInfo);
			if (bRes)
			{
				// fill cache
				m_ppdData=new _PLUGININFO;
				*m_ppdData=*pInfo;
			}
			EndUsage();
			return bRes;
		}
		else
			return false;			// must override in internal plugin
	}
}

void CPlugin::BeginUsage()
{
	// only means something for partial plugin state
	m_cs.Lock();
	if (m_byLoadState == PLGS_PARTIAL)
	{
		try
		{
			m_lLockCount++;
			Load(NULL, m_pfnCallback);
			m_bNeedChange=true;
		}
		catch (CPluginException*)
		{
			m_cs.Unlock();
			throw;
		}
	}
	else if (m_bNeedChange)
		m_lLockCount++;

	m_cs.Unlock();
}

void CPlugin::EndUsage()
{
	m_cs.Lock();
	try
	{
		if (m_bNeedChange)
		{
			if (m_lLockCount > 0)
				m_lLockCount--;
			if (m_lLockCount == 0)
			{
				Unload(false);
				m_bNeedChange=false;
			}
		}
	}
	catch (CPluginException*)
	{
		m_cs.Unlock();
		throw;
	}

	m_cs.Unlock();
}
