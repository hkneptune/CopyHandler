#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TPath.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TSmartPathTests, EmptyPathToString)
{
	TSmartPath tPath;
	EXPECT_STREQ(tPath.ToString(), _T(""));
}

TEST(TSmartPathTests, PathFromString)
{
	TSmartPath tPath;
	tPath.FromString(_T("c:\\test"));
	EXPECT_STREQ(tPath.ToString(), _T("c:\\test"));
}

TEST(TSmartPathTests, PathFromNullString)
{
	TSmartPath tPath;
	EXPECT_THROW(tPath.FromString(NULL), TCoreException);
}

TEST(TSmartPathTests, PathFromTStringToWString)
{
	TSmartPath tPath;
	tPath.FromString(TString(_T("c:\\test")));
	EXPECT_EQ(tPath.ToWString(), TString(_T("c:\\test")));
}

TEST(TSmartPathTests, PathFromEmptyTStringToString)
{
	TSmartPath tPath;
	tPath.FromString(TString());
	EXPECT_STREQ(tPath.ToString(), _T(""));
}

// path comparison
TEST(TSmartPathTests, PathComparison)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\test path"));
	path2.FromString(_T("C:\\Test Path"));

	EXPECT_TRUE(path1 == path2);
}

TEST(TSmartPathTests, PathComparisonLTGT)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("C:\\Test Path"));

	EXPECT_TRUE(path1 < path2);
	EXPECT_TRUE(path2 > path1);
}

TEST(TSmartPathTests, PathClear)
{
	TSmartPath path;

	path.FromString(_T("c:\\First path"));
	path.Clear();

	EXPECT_STREQ(path.ToString(), _T(""));
}

TEST(TSmartPathTests, AppendCopyWithSeparator)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("some directory"));

	TSmartPath retPath = path1.AppendCopy(path2, true);
	EXPECT_STREQ(retPath.ToString(), _T("c:\\First path\\some directory"));
}

TEST(TSmartPathTests, AppendCopyWithoutSeparator)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("some directory"));

	TSmartPath retPath = path1.AppendCopy(path2, false);
	EXPECT_STREQ(retPath.ToString(), _T("c:\\First pathsome directory"));
}


TEST(TSmartPathTests, AppendWithSeparator)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("some directory"));

	path1.Append(path2, true);
	EXPECT_STREQ(path1.ToString(), _T("c:\\First path\\some directory"));
}

TEST(TSmartPathTests, AppendWithoutSeparator)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("some directory"));

	path1.Append(path2, false);
	EXPECT_STREQ(path1.ToString(), _T("c:\\First pathsome directory"));
}

TEST(TSmartPathTests, SplitPath)
{
	TSmartPath path;
	TPathContainer vPaths;

	path.FromString(_T("c:\\First path\\some directory\\file.txt"));
	path.SplitPath(vPaths);

	EXPECT_EQ(4, vPaths.GetCount());
	EXPECT_STREQ(_T("c:"), vPaths.GetAt(0).ToString());
	EXPECT_STREQ(_T("First path"), vPaths.GetAt(1).ToString());
	EXPECT_STREQ(_T("some directory"), vPaths.GetAt(2).ToString());
	EXPECT_STREQ(_T("file.txt"), vPaths.GetAt(3).ToString());
}

TEST(TSmartPathTests, SplitEmptyPath)
{
	TSmartPath path;
	TPathContainer vPaths;

	path.FromString(_T(""));
	path.SplitPath(vPaths);

	EXPECT_EQ(0, vPaths.GetCount());
}

TEST(TSmartPathTests, CompareLTGTCaseInsensitive)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("C:\\Test Path"));

	EXPECT_TRUE(path1.Compare(path2, false) < 0);
	EXPECT_TRUE(path2.Compare(path1, false) > 0);
}

TEST(TSmartPathTests, CompareEQCaseInsensitive)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("C:\\first Path"));

	EXPECT_TRUE(path1.Compare(path2, false) == 0);
}

TEST(TSmartPathTests, CompareLTGTCaseSensitive)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("C:\\Test Path"));

	EXPECT_TRUE(path1.Compare(path2, false) < 0);
	EXPECT_TRUE(path2.Compare(path1, false) > 0);
}

TEST(TSmartPathTests, CompareEQCaseSensitive)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path"));
	path2.FromString(_T("C:\\first Path"));

	EXPECT_TRUE(path1.Compare(path2, true) != 0);
}

TEST(TSmartPathTests, CompareEQCaseSensitive2)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\first path"));
	path2.FromString(_T("c:\\first path"));

	EXPECT_TRUE(path1.Compare(path2, true) == 0);
}

TEST(TSmartPathTests, IsChildOf_CaseInsensitive)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path\\second path"));
	path2.FromString(_T("C:\\first path"));

	EXPECT_TRUE(path1.IsChildOf(path2, false));
}

TEST(TSmartPathTests, IsChildOf_CaseSensitive_NegativeCase)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\First path\\second path"));
	path2.FromString(_T("C:\\first path"));

	EXPECT_FALSE(path1.IsChildOf(path2, true));
}

TEST(TSmartPathTests, IsChildOf_CaseSensitive_PositiveCase)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\first path\\second path"));
	path2.FromString(_T("c:\\first path"));

	EXPECT_TRUE(path1.IsChildOf(path2, true));
}

TEST(TSmartPathTests, MakeRelativePath_CaseInensitive)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\first path\\second path"));
	path2.FromString(_T("C:\\First Path"));

	EXPECT_TRUE(path1.MakeRelativePath(path2, false));
	EXPECT_STREQ(_T("\\second path"), path1.ToString());
}

TEST(TSmartPathTests, MakeRelativePath_CaseSensitive_NegativeCase)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\first path\\second path"));
	path2.FromString(_T("C:\\First Path"));

	EXPECT_FALSE(path1.MakeRelativePath(path2, true));
	EXPECT_STREQ(_T("c:\\first path\\second path"), path1.ToString());
}

TEST(TSmartPathTests, MakeRelativePath_CaseSensitive_PositiveCase)
{
	TSmartPath path1;
	TSmartPath path2;

	path1.FromString(_T("c:\\first path\\second path"));
	path2.FromString(_T("c:\\first path"));

	EXPECT_TRUE(path1.MakeRelativePath(path2, true));
	EXPECT_STREQ(_T("\\second path"), path1.ToString());
}

TEST(TSmartPathTests, AppendIfNotExists_CaseInsensitive_PositiveCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path"));

	path2.AppendIfNotExists(_T("\\second path"), false);
	EXPECT_STREQ(_T("c:\\first path\\second path"), path2.ToString());
}

TEST(TSmartPathTests, AppendIfNotExists_CaseInsensitive_NegativeCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path\\Second Path"));

	path2.AppendIfNotExists(_T("\\second path"), false);
	EXPECT_STREQ(_T("c:\\first path\\Second Path"), path2.ToString());
}


TEST(TSmartPathTests, AppendIfNotExists_CaseSensitive_PositiveCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path"));

	path2.AppendIfNotExists(_T("\\second path"), true);
	EXPECT_STREQ(_T("c:\\first path\\second path"), path2.ToString());
}

TEST(TSmartPathTests, AppendIfNotExists_CaseSensitive_NegativeCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path\\second path"));

	path2.AppendIfNotExists(_T("\\second path"), true);
	EXPECT_STREQ(_T("c:\\first path\\second path"), path2.ToString());
}

TEST(TSmartPathTests, CutIfExists_CaseInsensitive_PositiveCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path"));

	path2.CutIfExists(_T("\\second path"), false);
	EXPECT_STREQ(_T("c:\\first path"), path2.ToString());
}

TEST(TSmartPathTests, CutIfExists_CaseInsensitive_NegativeCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path\\Second Path"));

	path2.CutIfExists(_T("\\second path"), false);
	EXPECT_STREQ(_T("c:\\first path"), path2.ToString());
}


TEST(TSmartPathTests, CutIfExists_CaseSensitive_PositiveCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path"));

	path2.CutIfExists(_T("\\second path"), true);
	EXPECT_STREQ(_T("c:\\first path"), path2.ToString());
}

TEST(TSmartPathTests, CutIfExists_CaseSensitive_NegativeCase)
{
	TSmartPath path2;

	path2.FromString(_T("c:\\first path\\second path"));

	path2.CutIfExists(_T("\\second path"), true);
	EXPECT_STREQ(_T("c:\\first path"), path2.ToString());
}

TEST(TSmartPathTests, IsNetworkPath_Negative)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_FALSE(path.IsNetworkPath());
}

TEST(TSmartPathTests, IsNetworkPath_Positive)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_TRUE(path.IsNetworkPath());
}

TEST(TSmartPathTests, IsDrive_Negative)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_FALSE(path.IsDrive());
}

TEST(TSmartPathTests, IsDrive_Negative_2)
{
	TSmartPath path;
	path.FromString(_T("c:\\"));

	EXPECT_FALSE(path.IsDrive());
}

TEST(TSmartPathTests, IsDrive_Positive)
{
	TSmartPath path;
	path.FromString(_T("c:"));

	EXPECT_TRUE(path.IsDrive());
}

TEST(TSmartPathTests, HasDrive_Positive)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_TRUE(path.HasDrive());
}

TEST(TSmartPathTests, HasDrive_Positive_2)
{
	TSmartPath path;
	path.FromString(_T("c:\\"));

	EXPECT_TRUE(path.HasDrive());
}

TEST(TSmartPathTests, HasDrive_Positive_3)
{
	TSmartPath path;
	path.FromString(_T("c:"));

	EXPECT_TRUE(path.HasDrive());
}

TEST(TSmartPathTests, HasDrive_Negative)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\share"));

	EXPECT_FALSE(path.HasDrive());
}

TEST(TSmartPathTests, GetDriveAndDriveLetterFromLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	TSmartPath pathDrive = path.GetDrive();
	wchar_t wchDrive = path.GetDriveLetter();	// makes drive letter uppercase

	EXPECT_STREQ(_T("c:"), pathDrive.ToString());
	EXPECT_EQ(_T('C'), wchDrive);
}

TEST(TSmartPathTests, GetDriveAndDriveLetterFromNonLocalPath)
{
	TSmartPath path;
	path.FromString(_T("\\serv01\\first path\\second path"));

	TSmartPath pathDrive = path.GetDrive();
	wchar_t wchDrive = path.GetDriveLetter();	// makes drive letter uppercase

	EXPECT_STREQ(_T(""), pathDrive.ToString());
	EXPECT_EQ(_T('\0'), wchDrive);
}

TEST(TSmartPathTests, IsServerName_Negative)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_FALSE(path.IsServerName());
}

TEST(TSmartPathTests, IsServerName_Negative_2)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\"));

	EXPECT_FALSE(path.IsServerName());
}

TEST(TSmartPathTests, IsServerName_Positive)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01"));

	EXPECT_TRUE(path.IsServerName());
}

TEST(TSmartPathTests, HasServerName_Positive)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_TRUE(path.HasServerName());
}

TEST(TSmartPathTests, HasServerName_Positive_2)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\"));

	EXPECT_TRUE(path.HasServerName());
}

TEST(TSmartPathTests, HasServerName_Positive_3)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01"));

	EXPECT_TRUE(path.HasServerName());
}

TEST(TSmartPathTests, HasServerName_Negative)
{
	TSmartPath path;
	path.FromString(_T("c:\\path"));

	EXPECT_FALSE(path.HasServerName());
}

TEST(TSmartPathTests, GetServerName)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	TSmartPath pathServerName = path.GetServerName();

	EXPECT_STREQ(_T("\\\\serv01"), pathServerName.ToString());
}

TEST(TSmartPathTests, GetServerNameFromLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	TSmartPath pathServerName = path.GetServerName();

	EXPECT_STREQ(_T(""), pathServerName.ToString());
}

TEST(TSmartPathTests, HasFileRootLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_TRUE(path.HasFileRoot());
}

TEST(TSmartPathTests, HasFileRootRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_TRUE(path.HasFileRoot());
}

TEST(TSmartPathTests, HasFileRootRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_TRUE(path.HasFileRoot());
}

TEST(TSmartPathTests, HasFileRootFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_FALSE(path.HasFileRoot());
}

TEST(TSmartPathTests, GetFileRootLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_STREQ(_T("c:\\first path\\"), path.GetFileRoot().ToString());
}

TEST(TSmartPathTests, GetFileRootRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_STREQ(_T("\\\\serv01\\first path\\"), path.GetFileRoot().ToString());
}

TEST(TSmartPathTests, GetFileRootRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_STREQ(_T("..\\"), path.GetFileRoot().ToString());
}

TEST(TSmartPathTests, GetFileRootFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_STREQ(_T(""), path.GetFileRoot().ToString());
}

// has/get file dir
TEST(TSmartPathTests, HasFileDirLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_TRUE(path.HasFileDir());
}

TEST(TSmartPathTests, HasFileDirRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_TRUE(path.HasFileDir());
}

TEST(TSmartPathTests, HasFileDirRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_TRUE(path.HasFileDir());
}

TEST(TSmartPathTests, HasFileDirFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_FALSE(path.HasFileDir());
}

TEST(TSmartPathTests, GetFileDirLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_STREQ(_T("\\first path\\"), path.GetFileDir().ToString());
}

TEST(TSmartPathTests, GetFileDirRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_STREQ(_T("\\first path\\"), path.GetFileDir().ToString());
}

TEST(TSmartPathTests, GetFileDirRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_STREQ(_T("..\\"), path.GetFileDir().ToString());
}

TEST(TSmartPathTests, GetFileDirFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_STREQ(_T(""), path.GetFileDir().ToString());
}

// has/get file title
TEST(TSmartPathTests, HasFileTitleLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_TRUE(path.HasFileTitle());
}

TEST(TSmartPathTests, HasFileTitleRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_TRUE(path.HasFileTitle());
}

TEST(TSmartPathTests, HasFileTitleRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_TRUE(path.HasFileTitle());
}

TEST(TSmartPathTests, HasFileTitleFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_TRUE(path.HasFileTitle());
}

TEST(TSmartPathTests, HasFileTitleFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory\\"));

	EXPECT_FALSE(path.HasFileTitle());
}

TEST(TSmartPathTests, GetFileTitleLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_STREQ(_T("second path"), path.GetFileTitle().ToString());
}

TEST(TSmartPathTests, GetFileTitleRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_STREQ(_T("second path"), path.GetFileTitle().ToString());
}

TEST(TSmartPathTests, GetFileTitleRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_STREQ(_T("file"), path.GetFileTitle().ToString());
}

TEST(TSmartPathTests, GetFileTitleFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_STREQ(_T("file"), path.GetFileTitle().ToString());
}

TEST(TSmartPathTests, GetFileTitleFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory\\"));

	EXPECT_STREQ(_T(""), path.GetFileTitle().ToString());
}

// has/get file name
TEST(TSmartPathTests, HasFileNameLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_TRUE(path.HasFileName());
}

TEST(TSmartPathTests, HasFileNameRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_TRUE(path.HasFileName());
}

TEST(TSmartPathTests, HasFileNameRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_TRUE(path.HasFileName());
}

TEST(TSmartPathTests, HasFileNameFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_TRUE(path.HasFileName());
}

TEST(TSmartPathTests, HasFileNameFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory\\"));

	EXPECT_FALSE(path.HasFileName());
}

TEST(TSmartPathTests, GetFileNameLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_STREQ(_T("second path"), path.GetFileName().ToString());
}

TEST(TSmartPathTests, GetFileNameRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_STREQ(_T("second path"), path.GetFileName().ToString());
}

TEST(TSmartPathTests, GetFileNameRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	EXPECT_STREQ(_T("file.txt"), path.GetFileName().ToString());
}

TEST(TSmartPathTests, GetFileNameFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	EXPECT_STREQ(_T("file.txt"), path.GetFileName().ToString());
}

TEST(TSmartPathTests, GetFileNameFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory\\"));

	EXPECT_STREQ(_T(""), path.GetFileName().ToString());
}

// delete filename
TEST(TSmartPathTests, DeleteFileNameLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	path.DeleteFileName();

	EXPECT_STREQ(_T("c:\\first path\\"), path.ToString());
}

TEST(TSmartPathTests, DeleteFileNameRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	path.DeleteFileName();

	EXPECT_STREQ(_T("\\\\serv01\\first path\\"), path.ToString());
}

TEST(TSmartPathTests, DeleteFileNameRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\file.txt"));

	path.DeleteFileName();

	EXPECT_STREQ(_T("..\\"), path.ToString());
}

TEST(TSmartPathTests, DeleteFileNameFileNameOnly)
{
	TSmartPath path;
	path.FromString(_T("file.txt"));

	path.DeleteFileName();

	EXPECT_STREQ(_T(""), path.ToString());
}

TEST(TSmartPathTests, DeleteFileNameFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory\\"));

	path.DeleteFileName();

	EXPECT_STREQ(_T("c:\\directory\\"), path.ToString());
}

// has/get file extension
TEST(TSmartPathTests, HasExtensionLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path.txt"));

	EXPECT_TRUE(path.HasExtension());
}

TEST(TSmartPathTests, HasExtensionRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path.txt"));

	EXPECT_TRUE(path.HasExtension());
}

TEST(TSmartPathTests, HasExtensionLocalPathNegative)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_FALSE(path.HasExtension());
}

TEST(TSmartPathTests, HasExtensionRemotePathNegatice)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_FALSE(path.HasExtension());
}

TEST(TSmartPathTests, HasExtensionRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\directory"));

	EXPECT_FALSE(path.HasExtension());
}

TEST(TSmartPathTests, HasExtensionFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext\\"));

	EXPECT_FALSE(path.HasExtension());
}

TEST(TSmartPathTests, GetExtensionLocalPath)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path.txt"));

	EXPECT_STREQ(_T(".txt"), path.GetExtension().ToString());
}

TEST(TSmartPathTests, GetExtensionRemotePath)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path.txt"));

	EXPECT_STREQ(_T(".txt"), path.GetExtension().ToString());
}

TEST(TSmartPathTests, GetExtensionLocalPathNegative)
{
	TSmartPath path;
	path.FromString(_T("c:\\first path\\second path"));

	EXPECT_STREQ(_T(""), path.GetExtension().ToString());
}

TEST(TSmartPathTests, GetExtensionRemotePathNegatice)
{
	TSmartPath path;
	path.FromString(_T("\\\\serv01\\first path\\second path"));

	EXPECT_STREQ(_T(""), path.GetExtension().ToString());
}

TEST(TSmartPathTests, GetExtensionRelativePath)
{
	TSmartPath path;
	path.FromString(_T("..\\directory"));

	EXPECT_STREQ(_T(""), path.GetExtension().ToString());
}

TEST(TSmartPathTests, GetExtensionFileDirOnly)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext\\"));

	EXPECT_STREQ(_T(""), path.GetExtension().ToString());
}

// separator operations
TEST(TSmartPathTests, EndsWithSeparator_True)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext\\"));

	EXPECT_TRUE(path.EndsWithSeparator());
}

TEST(TSmartPathTests, EndsWithSeparator_False)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext"));

	EXPECT_FALSE(path.EndsWithSeparator());
}

TEST(TSmartPathTests, AppendSeparator_AlreadyExists)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext\\"));
	path.AppendSeparatorIfDoesNotExist();
	EXPECT_STREQ(_T("c:\\directory.ext\\"), path.ToString());
}

TEST(TSmartPathTests, AppendSeparator_DoesNotExist)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext"));
	path.AppendSeparatorIfDoesNotExist();
	EXPECT_STREQ(_T("c:\\directory.ext\\"), path.ToString());
}

TEST(TSmartPathTests, StripSeparator_AlreadyExists)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext\\"));
	path.StripSeparatorAtEnd();
	EXPECT_STREQ(_T("c:\\directory.ext"), path.ToString());
}

TEST(TSmartPathTests, StripSeparator_DoesNotExist)
{
	TSmartPath path;
	path.FromString(_T("c:\\directory.ext"));
	path.StripSeparatorAtEnd();
	EXPECT_STREQ(_T("c:\\directory.ext"), path.ToString());
}

// separator at front
TEST(TSmartPathTests, StartsWithSeparator_True)
{
	TSmartPath path;
	path.FromString(_T("\\directory.ext\\"));

	EXPECT_TRUE(path.StartsWithSeparator());
}

TEST(TSmartPathTests, StartsWithSeparator_False)
{
	TSmartPath path;
	path.FromString(_T("directory.ext"));

	EXPECT_FALSE(path.StartsWithSeparator());
}

TEST(TSmartPathTests, PrependSeparator_AlreadyExists)
{
	TSmartPath path;
	path.FromString(_T("\\directory.ext"));
	path.PrependSeparatorIfDoesNotExist();
	EXPECT_STREQ(_T("\\directory.ext"), path.ToString());
}

TEST(TSmartPathTests, PrependSeparator_DoesNotExist)
{
	TSmartPath path;
	path.FromString(_T("directory.ext"));
	path.PrependSeparatorIfDoesNotExist();
	EXPECT_STREQ(_T("\\directory.ext"), path.ToString());
}

TEST(TSmartPathTests, StripSeparatorAtFront_AlreadyExists)
{
	TSmartPath path;
	path.FromString(_T("\\directory.ext"));
	path.StripSeparatorAtFront();
	EXPECT_STREQ(_T("directory.ext"), path.ToString());
}

TEST(TSmartPathTests, StripSeparatorAtFront_DoesNotExist)
{
	TSmartPath path;
	path.FromString(_T("directory.ext"));
	path.StripSeparatorAtFront();
	EXPECT_STREQ(_T("directory.ext"), path.ToString());
}

// isempty, get length
TEST(TSmartPathTests, IsEmpty_Empty)
{
	TSmartPath path;
	path.FromString(_T(""));
	EXPECT_TRUE(path.IsEmpty());
}

TEST(TSmartPathTests, IsEmpty_NotInitializedEmpty)
{
	TSmartPath path;
	EXPECT_TRUE(path.IsEmpty());
}

TEST(TSmartPathTests, IsEmpty_NotEmpty)
{
	TSmartPath path;
	path.FromString(_T("some path"));
	EXPECT_FALSE(path.IsEmpty());
}

TEST(TSmartPathTests, GetLength_Empty)
{
	TSmartPath path;
	path.FromString(_T(""));
	EXPECT_EQ(0, path.GetLength());
}

TEST(TSmartPathTests, GetLength_NotInitializedEmpty)
{
	TSmartPath path;
	EXPECT_EQ(0, path.GetLength());
}

TEST(TSmartPathTests, GetLength_NotEmpty)
{
	TSmartPath path;
	path.FromString(_T("some path"));
	EXPECT_EQ(9, path.GetLength());
}

/*
bool IsEmpty() const;
size_t GetLength() const;

// Serialization
void Serialize(TReadBinarySerializer& rSerializer);
void Serialize(TWriteBinarySerializer& rSerializer) const;

void StoreInConfig(TConfig& rConfig, PCTSTR pszPropName) const;
bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszPropName);
*/
