#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>
#include <locale>
#include <codecvt>
#include <boost/algorithm/string/replace.hpp>
#include "../TConfig.h"
#include "../TStringArray.h"
#include "../TConfigArray.h"

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
	void SetUp() override
	{
		m_strTempFilePath = GetTmpPath();

		std::wofstream outFile(m_strTempFilePath.c_str(), std::wofstream::out | std::wofstream::binary);

		std::locale utf8locale(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>);
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
				<FinishedSoundPath>&lt;WINDOWS&gt;\\\x597D\x8FD0\\ding.wav</FinishedSoundPath>\
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

	void TearDown() override
	{
		DeleteFile(m_strTempFilePath.c_str());
	}

	std::wstring m_strTempFilePath;
};

class InitializedConfigFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{
		m_strInputXmlString =
			L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\
<CHConfig>\
	<Core>\
		<AutosaveInterval>30000</AutosaveInterval>\
		<Notifications>\
			<Sounds>\
				<Enable>true</Enable>\
				<ErrorSoundPath>&lt;WINDOWS&gt;\\media\\chord.wav</ErrorSoundPath>\
				<FinishedSoundPath>&lt;WINDOWS&gt;\\\x597D\x8FD0\\ding.wav</FinishedSoundPath>\
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

		m_cfg.ReadFromString(m_strInputXmlString);
	}

	TString m_strInputXmlString;
	TConfig m_cfg;
};

///////////////////////////////////////////////////////////////////////////
// read from/write to file

TEST_F(FileWithConfigurationFixture, ReadFromFile)
{
	TConfig cfg;
	cfg.Read(m_strTempFilePath.c_str());

	EXPECT_EQ(true, cfg.GetBool(_T("CHConfig.Core.Notifications.Sounds.Enable"), false));
	EXPECT_EQ(30000, cfg.GetInt(_T("CHConfig.Core.AutosaveInterval"), 0));
	EXPECT_EQ(TString(_T("<WINDOWS>\\\x597D\x8FD0\\ding.wav")), cfg.GetString(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("")));
}

TEST(TConfigTests, WriteToFile)
{
	TConfig cfg;
	cfg.SetValue(_T("CHConfig.Core.Notifications.Sounds.Enable"), true);
	cfg.SetValue(_T("CHConfig.Core.AutosaveInterval"), 10000);
	cfg.SetValue(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("c:\\Users\\\x597D\x8FD0"));
	
	std::wstring strPath(GetTmpPath());
	cfg.SetFilePath(strPath.c_str());

	cfg.Write();

	std::wstring wstrData;
	std::wifstream inFile(strPath.c_str(), std::wofstream::in | std::wofstream::binary);

	std::locale utf8locale(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>);
	inFile.imbue(utf8locale);

	std::wstringstream wstrStream;
	wstrStream << inFile.rdbuf();

	wstrData = wstrStream.str();

	DeleteFile(strPath.c_str());

	boost::replace_all(wstrData, _T("\r\n"), _T("\n"));

	EXPECT_EQ(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<CHConfig><Core><AutosaveInterval>10000</AutosaveInterval><Notifications><Sounds><Enable>true</Enable><FinishedSoundPath>c:\\Users\\\x597D\x8FD0</FinishedSoundPath></Sounds></Notifications></Core></CHConfig>"),
wstrData);
}

///////////////////////////////////////////////////////////////////////////
// store in/load from string
TEST_F(InitializedConfigFixture, ReadFromString)
{
	EXPECT_EQ(true, m_cfg.GetBool(_T("CHConfig.Core.Notifications.Sounds.Enable"), false));
	EXPECT_EQ(30000, m_cfg.GetInt(_T("CHConfig.Core.AutosaveInterval"), 0));
	EXPECT_EQ(TString(_T("<WINDOWS>\\\x597D\x8FD0\\ding.wav")), m_cfg.GetString(_T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("")));
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

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetInt)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), 2147483647);
	cfg.SetValue(_T("Root.Node.Value2"), (-2147483647 - 1));

	// check if stored successfully (typed get)
	EXPECT_EQ(2147483647, cfg.GetInt(_T("Root.Node.Value1")));
	EXPECT_EQ((-2147483647 - 1), cfg.GetInt(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	int iValue = 0;
	cfg.GetValue(_T("Root.Node.Value1"), iValue);
	EXPECT_EQ(2147483647, iValue);
	cfg.GetValue(_T("Root.Node.Value2"), iValue);
	EXPECT_EQ((-2147483647 - 1), iValue);
}

TEST(TConfigTests, GetSetIntExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>2147483647</Value1><Value2>-2147483648</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(2147483647, cfg.GetInt(_T("Root.Node.Value1")));
	EXPECT_EQ((-2147483647 - 1), cfg.GetInt(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetUInt)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), (unsigned int)1489);
	cfg.SetValue(_T("Root.Node.Value2"), (unsigned int)4294967295);

	// check if stored successfully (typed get)
	EXPECT_EQ(1489, cfg.GetUInt(_T("Root.Node.Value1")));
	EXPECT_EQ(4294967295, cfg.GetUInt(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	unsigned int value = 0;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_EQ(1489, value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_EQ(4294967295, value);
}

TEST(TConfigTests, GetSetUIntExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>1489</Value1><Value2>4294967295</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(1489, cfg.GetUInt(_T("Root.Node.Value1")));
	EXPECT_EQ(4294967295, cfg.GetUInt(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetLongLong)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), (long long)9223372036854775807);
	cfg.SetValue(_T("Root.Node.Value2"), (long long)(-9223372036854775807 - 1));

	// check if stored successfully (typed get)
	EXPECT_EQ((long long)9223372036854775807, cfg.GetLongLong(_T("Root.Node.Value1")));
	EXPECT_EQ((long long)(-9223372036854775807 - 1), cfg.GetLongLong(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	long long value = 0;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_EQ((long long)9223372036854775807, value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_EQ((long long)(-9223372036854775807 - 1), value);
}

TEST(TConfigTests, GetSetLongLongExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>9223372036854775807</Value1><Value2>-9223372036854775808</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ((long long)9223372036854775807, cfg.GetLongLong(_T("Root.Node.Value1")));
	EXPECT_EQ((long long)(-9223372036854775807 - 1), cfg.GetLongLong(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetULongLong)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), (unsigned long long)1489);
	cfg.SetValue(_T("Root.Node.Value2"), (unsigned long long)18446744073709551615);

	// check if stored successfully (typed get)
	EXPECT_EQ((unsigned long long)1489, cfg.GetULongLong(_T("Root.Node.Value1")));
	EXPECT_EQ((unsigned long long)18446744073709551615, cfg.GetULongLong(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	unsigned long long value = 0;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_EQ((unsigned long long)1489, value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_EQ((unsigned long long)18446744073709551615, value);
}

TEST(TConfigTests, GetSetULongLongExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>1489</Value1><Value2>18446744073709551615</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ((unsigned long long)1489, cfg.GetULongLong(_T("Root.Node.Value1")));
	EXPECT_EQ((unsigned long long)18446744073709551615, cfg.GetULongLong(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetDouble)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), (double)0.0);
	cfg.SetValue(_T("Root.Node.Value2"), 1.7976931348623158e+308);

	// check if stored successfully (typed get)
	EXPECT_DOUBLE_EQ(0.0, cfg.GetDouble(_T("Root.Node.Value1")));
	EXPECT_DOUBLE_EQ(1.7976931348623158e+308, cfg.GetDouble(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	double value = 0;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_DOUBLE_EQ(0.0, value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_DOUBLE_EQ(1.7976931348623158e+308, value);
}

TEST(TConfigTests, GetSetDoubleExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>0</Value1><Value2>1.7976931348623158e+308</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_DOUBLE_EQ(0.0, cfg.GetDouble(_T("Root.Node.Value1")));
	EXPECT_DOUBLE_EQ(1.7976931348623158e+308, cfg.GetDouble(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetString)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), _T("Some basic string"));
	cfg.SetValue(_T("Root.Node.Value2"), L"!@#$%^&*()[]\\/<>,.QWERTYqwerty\x2021");

	// check if stored successfully (typed get)
	EXPECT_EQ(TString(_T("Some basic string")), cfg.GetString(_T("Root.Node.Value1")));
	EXPECT_EQ(TString(L"!@#$%^&*()[]\\/<>,.QWERTYqwerty\x2021"), cfg.GetString(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	TString value;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_EQ(TString(_T("Some basic string")), value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_EQ(TString(L"!@#$%^&*()[]\\/<>,.QWERTYqwerty\x2021"), value);
}

TEST(TConfigTests, GetSetStringExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>Some basic string</Value1><Value2>!@#$%^&amp;*()[]\\/&lt;&gt;,.QWERTYqwerty\x2021</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(TString(_T("Some basic string")), cfg.GetString(_T("Root.Node.Value1")));
	EXPECT_EQ(TString(L"!@#$%^&*()[]\\/<>,.QWERTYqwerty\x2021"), cfg.GetString(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetPath)
{
	TConfig cfg;

	// store data in config
	cfg.SetValue(_T("Root.Node.Value1"), _T("c:\\Windows\\System32"));
	cfg.SetValue(_T("Root.Node.Value2"), _T("\\\\machine\\share\\SomeFile.dat"));

	// check if stored successfully (typed get)
	EXPECT_EQ(PathFromString(_T("c:\\Windows\\System32")), cfg.GetPath(_T("Root.Node.Value1")));
	EXPECT_EQ(PathFromString(_T("\\\\machine\\share\\SomeFile.dat")), cfg.GetPath(_T("Root.Node.Value2")));

	// check if stored successfully (GetValue)
	TSmartPath value;
	cfg.GetValue(_T("Root.Node.Value1"), value);
	EXPECT_EQ(PathFromString(_T("c:\\Windows\\System32")), value);
	cfg.GetValue(_T("Root.Node.Value2"), value);
	EXPECT_EQ(PathFromString(_T("\\\\machine\\share\\SomeFile.dat")), value);
}

TEST(TConfigTests, GetSetPathExport)
{
	TConfig cfg;

	TString strXmlData(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Value1>c:\\Windows\\System32</Value1><Value2>\\\\machine\\share\\SomeFile.dat</Value2></Node></Root>"));

	// store in string
	cfg.ReadFromString(strXmlData);

	EXPECT_EQ(PathFromString(_T("c:\\Windows\\System32")), cfg.GetPath(_T("Root.Node.Value1")));
	EXPECT_EQ(PathFromString(_T("\\\\machine\\share\\SomeFile.dat")), cfg.GetPath(_T("Root.Node.Value2")));

	TString strWriteXmlData;
	cfg.WriteToString(strWriteXmlData);

	EXPECT_EQ(strXmlData, strWriteXmlData);
}

///////////////////////////////////////////////////////////////////////////
TEST(TConfigTests, GetSetStringArray)
{
	TConfig cfg;

	TStringArray arrString;
	arrString.Add(_T("String1"));
	arrString.Add(_T("SampleStringPath"));
	arrString.Add(_T("!@#$%^&*()"));
	arrString.Add(_T(""));

	// store data in config
	cfg.SetValue(_T("Root.Node.Values.Node"), arrString);

	// check if stored successfully (typed get)
	TStringArray arrRead;
	cfg.GetValue(_T("Root.Node.Values.Node"), arrRead);

	EXPECT_EQ(arrString.GetCount(), arrRead.GetCount());
	EXPECT_EQ(arrString.GetAt(0), arrRead.GetAt(0));
	EXPECT_EQ(arrString.GetAt(1), arrRead.GetAt(1));
	EXPECT_EQ(arrString.GetAt(2), arrRead.GetAt(2));
	EXPECT_EQ(arrString.GetAt(3), arrRead.GetAt(3));
}

TEST_F(InitializedConfigFixture, GetSetStringArrayImport)
{
	TStringArray arrRead;
	m_cfg.GetValue(_T("CHConfig.Core.Notifications.PathList.Path"), arrRead);

	EXPECT_EQ(arrRead.GetCount(), 4);
	EXPECT_EQ(TString(_T("c:\\Windows\\System32")), arrRead.GetAt(0));
	EXPECT_EQ(TString(_T("d:\\Movies")), arrRead.GetAt(1));
	EXPECT_EQ(TString(_T("x:\\Music")), arrRead.GetAt(2));
	EXPECT_EQ(TString(_T("s:\\projects\\ch-rw")), arrRead.GetAt(3));
}

TEST(TConfigTests, GetSetStringArrayExport)
{
	TConfig cfg;

	TStringArray arrString;
	arrString.Add(_T("String1"));
	arrString.Add(_T("SampleStringPath"));
	arrString.Add(_T("!@#$%^&*()"));
	arrString.Add(_T(""));

	// store data in config
	cfg.SetValue(_T("Root.Node.Values.Node"), arrString);

	// store in string
	TString wstrData;
	cfg.WriteToString(wstrData);

	EXPECT_EQ(TString(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><Node><Values><Node>String1</Node><Node>SampleStringPath</Node><Node>!@#$%^&amp;*()</Node><Node/></Values></Node></Root>")), wstrData);
}

///////////////////////////////////////////////////////////////////////////
TEST_F(InitializedConfigFixture, CompositeObjectsReadWriteString)
{
	TString wstrWithDeletion;
	m_cfg.WriteToString(wstrWithDeletion);

	EXPECT_EQ(TString(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<CHConfig><Core><AutosaveInterval>30000</AutosaveInterval>\
<CompositeObjects><Object><Name>FirstName</Name><Path>&lt;WINDOWS&gt;\\FirstPath</Path></Object><Object><Name>SecondName</Name><Path>&lt;WINDOWS&gt;\\SecondPath</Path></Object></CompositeObjects>\
<Notifications><PathList><Path>c:\\Windows\\System32</Path><Path>d:\\Movies</Path><Path>x:\\Music</Path><Path>s:\\projects\\ch-rw</Path></PathList>\
<Sounds><Enable>true</Enable><ErrorSoundPath>&lt;WINDOWS&gt;\\media\\chord.wav</ErrorSoundPath>\
<FinishedSoundPath>&lt;WINDOWS&gt;\\\x597D\x8FD0\\ding.wav</FinishedSoundPath></Sounds></Notifications></Core></CHConfig>")), wstrWithDeletion);
}

///////////////////////////////////////////////////////////////////////////
TEST_F(InitializedConfigFixture, DeleteNodeTest)
{
	m_cfg.DeleteNode(_T("CHConfig.Core.Notifications"));

	TString wstrWithDeletion;
	m_cfg.WriteToString(wstrWithDeletion);

	EXPECT_EQ(TString(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<CHConfig><Core><AutosaveInterval>30000</AutosaveInterval><CompositeObjects><Object><Name>FirstName</Name><Path>&lt;WINDOWS&gt;\\FirstPath</Path></Object>\
<Object><Name>SecondName</Name><Path>&lt;WINDOWS&gt;\\SecondPath</Path></Object></CompositeObjects></Core></CHConfig>")), wstrWithDeletion);
}

///////////////////////////////////////////////////////////////////////////
TEST_F(InitializedConfigFixture, ExtractConfig)
{
	TConfig cfgSub;

	m_cfg.ExtractSubConfig(_T("CHConfig.Core.Notifications.Sounds"), cfgSub);
	EXPECT_EQ(true, cfgSub.GetBool(_T("Enable")));
	EXPECT_EQ(TString(_T("<WINDOWS>\\media\\chord.wav")), cfgSub.GetString(_T("ErrorSoundPath")));
	EXPECT_EQ(TString(_T("<WINDOWS>\\\x597D\x8FD0\\ding.wav")), cfgSub.GetString(_T("FinishedSoundPath")));
}

TEST_F(InitializedConfigFixture, ExtractMultipleConfigs)
{
	TConfigArray cfgSubArray;

	m_cfg.ExtractMultiSubConfigs(_T("CHConfig.Core.CompositeObjects.Object"), cfgSubArray);

	EXPECT_EQ(2, cfgSubArray.GetCount());

	EXPECT_EQ(TString(_T("FirstName")), cfgSubArray.GetAt(0).GetString(_T("Name")));
	EXPECT_EQ(TString(_T("<WINDOWS>\\FirstPath")), cfgSubArray.GetAt(0).GetString(_T("Path")));
	EXPECT_EQ(TString(_T("SecondName")), cfgSubArray.GetAt(1).GetString(_T("Name")));
	EXPECT_EQ(TString(_T("<WINDOWS>\\SecondPath")), cfgSubArray.GetAt(1).GetString(_T("Path")));
}

TEST(TConfigTests, PutSubConfig)
{
	TConfig mainCfg;

	TConfig cfgSub;
	cfgSub.SetValue(_T("Node1"), true);
	cfgSub.SetValue(_T("NameNode"), _T("TestNode"));

	mainCfg.PutSubConfig(_T("Root.NodeSet"), cfgSub);

	EXPECT_EQ(true, mainCfg.GetBool(_T("Root.NodeSet.Node1")));
	EXPECT_EQ(TString(_T("TestNode")), mainCfg.GetString(_T("Root.NodeSet.NameNode")));
}

TEST(TConfigTests, AddSubConfig)
{
	TConfig mainCfg;

	TConfig cfgSub;
	cfgSub.SetValue(_T("Node1"), true);
	cfgSub.SetValue(_T("NameNode"), _T("TestNode"));

	mainCfg.AddSubConfig(_T("Root.NodeSet.Node"), cfgSub);
	mainCfg.AddSubConfig(_T("Root.NodeSet.Node"), cfgSub);

	TString strXml;
	mainCfg.WriteToString(strXml);

	EXPECT_EQ(TString(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<Root><NodeSet><Node><NameNode>TestNode</NameNode><Node1>true</Node1></Node><Node><NameNode>TestNode</NameNode><Node1>true</Node1></Node></NodeSet></Root>")), strXml);
}
