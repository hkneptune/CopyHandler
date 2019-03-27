#include "stdafx.h"
#include "AppAutorun.h"
#include "../common/TRegistry.h"
#include "../libstring/TString.h"
#include "../libchcore/TPath.h"

AppAutorun::AppAutorun(const string::TString& strAppName, const chcore::TSmartPath& pathApp) :
	m_strAppName(strAppName),
	m_pathApp(pathApp)
{
}

bool AppAutorun::IsAutorunEnabled() const
{
	TRegistry reg(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), true);

	std::wstring wstrPath;
	if (!reg.QueryString(m_strAppName.c_str(), wstrPath))
		return false;

	if (m_pathApp.Compare(chcore::PathFromString(wstrPath.c_str())) != 0)
		return false;

	return true;
}

void AppAutorun::SetAutorun(bool bEnable)
{
	if (IsAutorunEnabled() == bEnable)
		return;

	TRegistry reg(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), false);
	if (bEnable)
		reg.SetString(m_strAppName.c_str(), m_pathApp.ToString());
	else
		reg.DeleteValue(m_strAppName.c_str());
}
