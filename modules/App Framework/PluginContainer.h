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
#ifndef __PLUGINCONTAINER_H__
#define __PLUGINCONTAINER_H__

#include <map>
#include "Plugin.h"

using namespace std;

// constants - used in OnPluginLoading for determining action for plugin
#define PL_REMOVE			0
#define PL_ADDUNLOADED		1
#define PL_ADD				2

class CPluginContainer
{
public:
	CPluginContainer();
	~CPluginContainer();

	void Scan(PCTSTR pszFolder, UINT uiType);				// scans for external plugins
	ULONGLONG Add(CPlugin* tPlugin, _PLUGININFO* ppi);		// adds plugin to plugin map
	void Remove(ULONGLONG uiID);							// removes plugin from map
	void Close();

	virtual CPlugin* NewPlugin() { return new CPlugin(); };			// allocates mem for plugin (depends on container type)
	virtual void FreePlugin(CPlugin* pPlugin) { delete pPlugin; };

	virtual void Prescan(PCTSTR /*pszFolder*/, UINT /*uiType*/) { };	// called before scanning
	virtual void Postscan(PCTSTR /*pszFolder*/, UINT /*uiType*/) { };	// after scanning finished

	virtual UINT OnPluginLoading(CPlugin* /*tPlugin*/, _PLUGININFO* /*pInfo*/) { return PL_ADD; };	// plugin found/parially loaded
																		// - what to do ?
	virtual void OnPluginLoaded(CPlugin* /*tPlugin*/, _PLUGININFO* /*pInfo*/) { };	// plugin has been added to a list

public:
	typedef map<ULONGLONG, CPlugin*> plugin_map;
	PFNEXTINFOPROC m_pfnCallback;						// callback for plugin usage - communication with app
	plugin_map m_mPlugins;
};


#endif
