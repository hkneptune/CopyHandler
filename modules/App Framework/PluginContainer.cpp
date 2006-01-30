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
#include "PluginContainer.h"

// member functions
CPluginContainer::CPluginContainer()
{

}

CPluginContainer::~CPluginContainer()
{
	Close();
}

// scans (recursively) a folder for plugins that meet uiType criteria
void CPluginContainer::Scan(PCTSTR pszFolder, UINT uiType)
{
	// before we scan...
	Prescan(pszFolder, uiType);

	// place for additional "\*.dll"
	TCHAR *pszPath=new TCHAR[_tcslen(pszFolder)+7];
	_tcscpy(pszPath, pszFolder);
	_tcscat(pszPath, _T("\\*.dll"));

	WIN32_FIND_DATA wfd;
	CPlugin* pplg=NULL;
	_PLUGININFO pi;
	HANDLE hFind=FindFirstFile(pszPath, &wfd);
	BOOL bFnd=TRUE;
	TCHAR szPluginPath[_MAX_PATH];
	while (hFind != INVALID_HANDLE_VALUE && bFnd)
	{
		// found next file - establish full path to that file
		_tcscpy(szPluginPath, pszFolder);
		_tcscat(szPluginPath, _T("\\"));
		_tcscat(szPluginPath, wfd.cFileName);

		// load external plugin
		pplg=NewPlugin();
		try
		{
			pplg->Load(szPluginPath, m_pfnCallback);

			// get info about this plugin
			if (!pplg->GetInfo(&pi) || !(pi.uiMask & uiType))
			{
				pplg->Unload(true);
				FreePlugin(pplg);
			}
			else
				Add(pplg, &pi);			// add to internal storage structure
		}
		catch (CPluginException* e)
		{
			FreePlugin(pplg);
			delete e;
		}

		// and next plugin to go
		bFnd=FindNextFile(hFind, &wfd);
	}

	// delete not needed pplg
	delete pszPath;
	Postscan(pszFolder, uiType);
}

ULONGLONG CPluginContainer::Add(CPlugin* tPlugin, _PLUGININFO* ppi)
{
	UINT uiRes=OnPluginLoading(tPlugin, ppi);
	if (uiRes == PL_REMOVE)
	{
		// plugin cannot be loaded
		tPlugin->Unload(true);
		FreePlugin(tPlugin);
		return (ULONGLONG)-1;
	}
	else
	{
		// add plugin
		if (uiRes == PL_ADDUNLOADED)
			tPlugin->Unload(false);

		// add
		m_mPlugins.insert(plugin_map::value_type(ppi->uliSignature.QuadPart, tPlugin));

		// plugin has been successfully loaded into memory
		OnPluginLoaded(tPlugin, ppi);
	}

	return ppi->uliSignature.QuadPart;
}

void CPluginContainer::Remove(ULONGLONG uiID)
{
	plugin_map::iterator it=m_mPlugins.find(uiID);
	if (it != m_mPlugins.end())
	{
		CPlugin* pplg=it->second;

		pplg->Unload(true);
		FreePlugin(pplg);
		m_mPlugins.erase(uiID);
	}
}

void CPluginContainer::Close()
{
	plugin_map::const_iterator it=m_mPlugins.begin();
	CPlugin *pplg;
	while (it != m_mPlugins.end())
	{
		pplg=it->second;
		pplg->Unload(true);
		FreePlugin(pplg);
		it++;
	}
	m_mPlugins.clear();
}
