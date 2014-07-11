#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>
#include <locale>
#include "../TConfig.h"

using namespace chcore;

namespace
{
	std::wstring GetTmpPath()
	{
		TCHAR szPath[_MAX_PATH];
		GetTempPath(_MAX_PATH, szPath);

		TCHAR szFilename[_MAX_PATH];
		GetTempFileName(szPath, _T("TempCfg"), 0, szFilename);

		return szFilename;
	}
}

// fixtures
class FileWithConfigurationFixture : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		m_strTempFilePath = GetTmpPath();

		std::wofstream outFile(m_strTempFilePath.c_str(), std::wofstream::out | std::wofstream::binary);

		std::locale utf8locale(std::locale(), new std::codecvt_byname<wchar_t, char, mbstate_t> ("en_US.UTF-8"));
		outFile.imbue(utf8locale);

		std::wstring wstrData =
			L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\
<CHConfig>\
	<Core>\
		<AutosaveInterval>30000</AutosaveInterval>\
		<Notifications>\
			<Sounds>\
				<Enable>true</Enable>\
				<ErrorSoundPath>&lt;WINDOWS&gt;\\media\\chord.wav</ErrorSoundPath>\
				<FinishedSoundPath>&lt;WINDOWS&gt;\\media\\ding.wav</FinishedSoundPath>\
			</Sounds>\
			<PathList>\
				<Path>c:\\Windows\\System32</Path>\
				<Path>d:\\Movies</Path>\
				<Path>x:\\Music</Path>\
				<Path>s:\\projects\\ch-rw</Path>\
			</PathList>\
		</Notifications>\
		<CompositeObjects>\
			<Object>\
				<Path>&lt;WINDOWS&gt;\\FirstPath</Path>\
				<Name>FirstName</Name>\
			</Object>\
			<Object>\
				<Path>&lt;WINDOWS&gt;\\SecondPath</Path>\
				<Name>SecondName</Name>\
			</Object>\
		</CompositeObjects>\
	</Core>\
</CHConfig>";

		outFile << wstrData;
		outFile.flush();
	}

	virtual void TearDown()
	{
		DeleteFile(m_strTempFilePath.c_str());
	}

	std::wstring m_strTempFilePath;
};

///////////////////////////////////////////////////////////////////////////
// read from/write to file

TEST_F(FileWithConfigurationFixture, ReadFromFile)
{
	TConfig cfg;
	cfg.Read(m_strTempFilePath.c_str());

	EXPECT_EQ(true, cfg.GetBool(_T("CHConfig.Core.Notifications.Sounds.Enable"), false));
	EXPECT_EQ(30000, cfg.GetInt(_T("CHConfig.Core.AutosaveInterval"), 0));
	EXPECT_EQ(TString(_T("<WINDOWS>\\media\\ding.wav")), cfg.GetString(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("")));
}

TEST(TConfigTests, WriteToFile)
{
	TConfig cfg;
	cfg.SetValue(_T("CHConfig.Core.Notifications.Sounds.Enable"), true);
	cfg.SetValue(_T("CHConfig.Core.AutosaveInterval"), 10000);
	cfg.SetValue(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("c:\\Users\\NewUser"));
	
	std::wstring strPath(GetTmpPath());
	cfg.SetFilePath(strPath.c_str());

	cfg.Write();

	std::wstring wstrData;
	std::wifstream inFile(strPath.c_str(), std::wofstream::in | std::wofstream::binary);

	std::locale utf8locale(std::locale(), new std::codecvt_byname<wchar_t, char, mbstate_t> ("en_US.UTF-8"));
	inFile.imbue(utf8locale);

	std::wstringstream wstrStream;
	wstrStream << inFile.rdbuf();

	wstrData = wstrStream.str();

	DeleteFile(strPath.c_str());

	EXPECT_EQ(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<CHConfig><Core><AutosaveInterval>10000</AutosaveInterval><Notifications><Sounds><Enable>true</Enable><FinishedSoundPath>c:\\Users\\NewUser</FinishedSoundPath></Sounds></Notifications></Core></CHConfig>"), wstrData);
}

///////////////////////////////////////////////////////////////////////////
// store in/load from string
TEST(TConfigTests, ReadFromString)
{
	std::wstring wstrData =
		L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\
<CHConfig>\
	<Core>\
		<AutosaveInterval>30000</AutosaveInterval>\
		<Notifications>\
			<Sounds>\
				<Enable>true</Enable>\
				<ErrorSoundPath>&lt;WINDOWS&gt;\\media\\chord.wav</ErrorSoundPath>\
				<FinishedSoundPath>&lt;WINDOWS&gt;\\media\\ding.wav</FinishedSoundPath>\
			</Sounds>\
			<PathList>\
				<Path>c:\\Windows\\System32</Path>\
				<Path>d:\\Movies</Path>\
				<Path>x:\\Music</Path>\
				<Path>s:\\projects\\ch-rw</Path>\
			</PathList>\
		</Notifications>\
		<CompositeObjects>\
			<Object>\
				<Path>&lt;WINDOWS&gt;\\FirstPath</Path>\
				<Name>FirstName</Name>\
			</Object>\
			<Object>\
				<Path>&lt;WINDOWS&gt;\\SecondPath</Path>\
				<Name>SecondName</Name>\
			</Object>\
		</CompositeObjects>\
	</Core>\
</CHConfig>";

	TConfig cfg;
	cfg.ReadFromString(TString(wstrData.c_str()));

	EXPECT_EQ(true, cfg.GetBool(_T("CHConfig.Core.Notifications.Sounds.Enable"), false));
	EXPECT_EQ(30000, cfg.GetInt(_T("CHConfig.Core.AutosaveInterval"), 0));
	EXPECT_EQ(TString(_T("<WINDOWS>\\media\\ding.wav")), cfg.GetString(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("")));
}

TEST(TConfigTests, WriteToString)
{
	TConfig cfg;
	cfg.SetValue(_T("CHConfig.Core.Notifications.Sounds.Enable"), true);
	cfg.SetValue(_T("CHConfig.Core.AutosaveInterval"), 10000);
	cfg.SetValue(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("c:\\Users\\NewUser"));

	TString strData;
	cfg.WriteToString(strData);

	EXPECT_EQ(TString(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<CHConfig><Core><AutosaveInterval>10000</AutosaveInterval><Notifications><Sounds><Enable>true</Enable><FinishedSoundPath>c:\\Users\\NewUser</FinishedSoundPath></Sounds></Notifications></Core></CHConfig>")), strData);
}

///////////////////////////////////////////////////////////////////////////
// value get/set

// bool

TEST(TConfigTests, GetSetBool)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), true);
	cfg.SetValue(_T("Root.Node.Value2"), false);

	// check if stored successfully (typed get)
	EXPECT_EQ(true, cfg.GetBool(_T("Root.Node.Value1")));
	EXPECT_EQ(false, cfg.GetBool(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	bool bValue = false;
	cfg.GetValue(_T("Root.Node.Value1"), bValue);
	EXPECT_EQ(true, bValue);
	cfg.GetValue(_T("Root.Node.Value2"), bValue);
	EXPECT_EQ(false, bValue);
}

TEST(TConfigTests, GetSetBoolExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>true</Value1><Value2>false</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(true, cfg.GetBool(_T("Root.Node.Value1")));
	EXPECT_EQ(false, cfg.GetBool(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

TEST(TConfigTests, GetSetInt)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), 1489);
	cfg.SetValue(_T("Root.Node.Value2"), -12987);

	// check if stored successfully (typed get)
	EXPECT_EQ(1489, cfg.GetInt(_T("Root.Node.Value1")));
	EXPECT_EQ(-12987, cfg.GetInt(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	int iValue = 0;
	cfg.GetValue(_T("Root.Node.Value1"), iValue);
	EXPECT_EQ(1489, iValue);
	cfg.GetValue(_T("Root.Node.Value2"), iValue);
	EXPECT_EQ(-12987, iValue);
}

TEST(TConfigTests, GetSetIntExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>1489</Value1><Value2>-12987</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(1489, cfg.GetInt(_T("Root.Node.Value1")));
	EXPECT_EQ(-12987, cfg.GetInt(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

TEST(TConfigTests, GetSetUInt)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), (unsigned int)1489);
	cfg.SetValue(_T("Root.Node.Value2"), (unsigned int)12987);

	// check if stored successfully (typed get)
	EXPECT_EQ(1489, cfg.GetUInt(_T("Root.Node.Value1")));
	EXPECT_EQ(12987, cfg.GetUInt(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	int value = 0;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_EQ(1489, value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_EQ(12987, value);
}

TEST(TConfigTests, GetSetUIntExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>1489</Value1><Value2>12987</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(1489, cfg.GetInt(_T("Root.Node.Value1")));
	EXPECT_EQ(12987, cfg.GetInt(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}
