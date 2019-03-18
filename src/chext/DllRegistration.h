// ============================================================================
//  Copyright (C) 2001-2019 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================

#pragma once

class DllRegistration
{
public:
	DllRegistration(HMODULE hModule);

	void RegisterAll();
	void UnregisterAll();

	void RegisterMenuExt();
	void RegisterDropMenuExt();
	void RegisterShellExtensionControl();

	void UnregisterMenuExt();
	void UnregisterDropMenuExt();
	void UnregisterShellExtensionControl();

	void RemoveLegacyEntries();

private:
	void RegisterClass(std::wstring strClsId, std::wstring strClassDescription, std::wstring strThreadingModel);
	void UnregisterClass(std::wstring strClsId);

private:
	std::wstring m_strModulePath;
};
