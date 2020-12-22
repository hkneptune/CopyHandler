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
#include "stdafx.h"
#include "DllRegistration.h"
#include "guids.h"
#include "../common/TRegistry.h"

namespace
{
	std::wstring CLSID2String(REFCLSID rclsid)
	{
		const size_t stBufferSize = 64;
		wchar_t szBuffer[stBufferSize] = {};
		_sntprintf_s(szBuffer, stBufferSize, _TRUNCATE, L"{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",
			rclsid.Data1, rclsid.Data2, rclsid.Data3,
			rclsid.Data4[0], rclsid.Data4[1], rclsid.Data4[2], rclsid.Data4[3],
			rclsid.Data4[4], rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7]);

		return szBuffer;
	}

	std::wstring GetModulePath(HMODULE hModule)
	{
		wchar_t szPath[MAX_PATH + 2] = {};
		DWORD dwSize = ::GetModuleFileName(hModule, szPath, MAX_PATH + 1);
		if (dwSize != 0 && dwSize < MAX_PATH)
		{
			szPath[dwSize] = L'\0';
			return szPath;
		}

		return std::wstring();
	}

	void CreateSingleValue(HKEY key, const wchar_t* pszKey, const wchar_t* pszValueKey, const wchar_t* pszValue)
	{
		TRegistry reg(key, pszKey, false);
		reg.SetString(pszValueKey, pszValue);
	}

	void DeleteSingleValue(HKEY key, const wchar_t* pszKey, const wchar_t* pszValueKey)
	{
		TRegistry reg(key, pszKey, false, false);
		if(reg.IsOpen())
			reg.DeleteValue(pszValueKey);
	}

	void CreateNodes(HKEY key, const wchar_t* pszKey, std::wstring strSubKey)
	{
		TRegistry reg(key, pszKey, false);
		reg.CreateSubKey(strSubKey.c_str());
	}

	void CreateNodeWithDefaultValue(HKEY key, const wchar_t* pszKey, const wchar_t* pszSubKey, const wchar_t* pszValue)
	{
		// ensure key exists
		CreateNodes(key, L"", pszKey);

		TRegistry reg(key, pszKey, false);
		reg.CreateSubKey(pszSubKey);

		std::wstring strNewKey = pszKey;
		strNewKey += L'\\';
		strNewKey += pszSubKey;

		reg.ReOpen(key, strNewKey.c_str(), false);
		reg.SetString(L"", pszValue);
	}

	void DeleteSingleNode(HKEY key, const wchar_t* pszKey, const wchar_t* pszValueKey)
	{
		TRegistry reg(key, pszKey, false, false);
		if(reg.IsOpen())
			reg.DeleteSubKey(pszValueKey);
	}
}

DllRegistration::DllRegistration(HMODULE hModule)
{
	if (hModule == nullptr)
		throw std::invalid_argument("hModule");

	m_strModulePath = GetModulePath(hModule);
	if (m_strModulePath.empty())
		throw std::runtime_error("Cannot retrieve module path");
}

void DllRegistration::RegisterAll()
{
	RemoveLegacyEntries();

	RegisterShellExtensionControl();
	RegisterMenuExt();
	RegisterDropMenuExt();
}

void DllRegistration::UnregisterAll()
{
	RemoveLegacyEntries();

	UnregisterShellExtensionControl();
	UnregisterMenuExt();
	UnregisterDropMenuExt();
}

void DllRegistration::RegisterMenuExt()
{
	std::wstring strClsID = CLSID2String(CLSID_MenuExt);

	RegisterClass(strClsID, L"MenuExt Class", L"Apartment");

	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"Directory\\Shellex\\ContextMenuHandlers", L"chext", strClsID.c_str());
	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"Directory\\Background\\Shellex\\ContextMenuHandlers", L"chext", strClsID.c_str());
	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"Folder\\Shellex\\ContextMenuHandlers", L"chext", strClsID.c_str());
	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"*\\Shellex\\ContextMenuHandlers", L"chext", strClsID.c_str());

	CreateSingleValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", strClsID.c_str(), L"chext");
}

void DllRegistration::RegisterDropMenuExt()
{
	std::wstring strClsID = CLSID2String(CLSID_DropMenuExt);
	RegisterClass(strClsID, L"DropMenuExt Class", L"Apartment");

	CreateNodes(HKEY_CLASSES_ROOT, L"CLSID", strClsID + L"\\shellex\\MayChangeDefaultMenu");

	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"Directory\\Shellex\\DragDropHandlers", L"chext", strClsID.c_str());
	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"Drive\\Shellex\\DragDropHandlers", L"chext", strClsID.c_str());
	CreateNodeWithDefaultValue(HKEY_CLASSES_ROOT, L"Folder\\Shellex\\DragDropHandlers", L"chext", strClsID.c_str());

	CreateSingleValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", strClsID.c_str(), L"chext");
}

void DllRegistration::RegisterShellExtensionControl()
{
	RegisterClass(CLSID2String(CLSID_CShellExtControl), L"ShellExtControl Class", L"Both");
}

void DllRegistration::UnregisterMenuExt()
{
	std::wstring strClsID = CLSID2String(CLSID_MenuExt);

	UnregisterClass(strClsID);
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Directory\\Shellex\\ContextMenuHandlers", L"chext");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Directory\\Background\\Shellex\\ContextMenuHandlers", L"chext");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Folder\\Shellex\\ContextMenuHandlers", L"chext");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"*\\Shellex\\ContextMenuHandlers", L"chext");

	DeleteSingleValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", strClsID.c_str());
}

void DllRegistration::UnregisterDropMenuExt()
{
	std::wstring strClsID = CLSID2String(CLSID_DropMenuExt);

	UnregisterClass(strClsID);
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Directory\\Shellex\\DragDropHandlers", L"chext");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Drive\\Shellex\\DragDropHandlers", L"chext");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Folder\\Shellex\\DragDropHandlers", L"chext");

	DeleteSingleValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", strClsID.c_str());
}

void DllRegistration::UnregisterShellExtensionControl()
{
	UnregisterClass(CLSID2String(CLSID_CShellExtControl));
}

void DllRegistration::RemoveLegacyEntries()
{
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"", L"chext.MenuExt");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"", L"chext.MenuExt.1");

	DeleteSingleNode(HKEY_CLASSES_ROOT, L"", L"chext.DropMenuExt");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"", L"chext.DropMenuExt.1");

	DeleteSingleNode(HKEY_CLASSES_ROOT, L"", L"chext.ShellExtControl");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"", L"chext.ShellExtControl.1");

	DeleteSingleNode(HKEY_CLASSES_ROOT, L"Interface", L"{317E503A-9D2F-4F42-995E-D314CB9D89B0}");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"TypeLib", L"{68FAFC14-8EB8-4DA1-90EB-6B3D22010505}");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"AppID", L"{9D4C4C5F-EE90-4a6b-9245-244C369E4FAE}");
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"AppID", L"chext.dll");
}

void DllRegistration::RegisterClass(std::wstring strClsId, std::wstring strClassDescription, std::wstring strThreadingModel)
{
	std::wstring strGuidNode = L"CLSID\\" + strClsId;
	std::wstring strInprocServerNode = strGuidNode + L"\\InprocServer32";

	CreateNodes(HKEY_CLASSES_ROOT, L"CLSID", strClsId + L"\\InprocServer32");

	CreateSingleValue(HKEY_CLASSES_ROOT, strGuidNode.c_str(), L"", strClassDescription.c_str());

	TRegistry reg(HKEY_CLASSES_ROOT, strInprocServerNode.c_str(), false);
	reg.SetString(L"", m_strModulePath.c_str());
	reg.SetString(L"ThreadingModel", strThreadingModel.c_str());
}

void DllRegistration::UnregisterClass(std::wstring strClsId)
{
	DeleteSingleNode(HKEY_CLASSES_ROOT, L"CLSID", strClsId.c_str());
}
