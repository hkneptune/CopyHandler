#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TDestinationPathProvider.h"
#include "../TFileInfo.h"
#include "../TBasePathData.h"

using namespace testing;
using namespace chengine;

class IFilesystemMock : public IFilesystem
{
public:
	MOCK_METHOD1(PathExist, bool(const TSmartPath&));
	MOCK_METHOD5(SetFileDirBasicInfo, void(const TSmartPath& pathFileDir, DWORD dwAttributes, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime));
	MOCK_METHOD2(SetAttributes, void(const TSmartPath& pathFileDir, DWORD dwAttributes));

	MOCK_METHOD2(CreateDirectory, void(const TSmartPath& pathDirectory, bool bCreateFullPath));
	MOCK_METHOD1(RemoveDirectory, void(const TSmartPath& pathFile));
	MOCK_METHOD1(DeleteFile, void(const TSmartPath& pathFile));

	MOCK_METHOD3(GetFileInfo, void(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData));
	MOCK_METHOD2(FastMove, void(const TSmartPath& pathSource, const TSmartPath& pathDestination));

	MOCK_METHOD2(CreateFinderObject, IFilesystemFindPtr(const TSmartPath& pathDir, const TSmartPath& pathMask));
	MOCK_METHOD4(CreateFileObject, IFilesystemFilePtr(IFilesystemFile::EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles));

	MOCK_METHOD2(GetPathsRelation, EPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond));

	MOCK_METHOD3(GetDynamicFreeSpace, void(const TSmartPath& path, unsigned long long& rullFree, unsigned long long& rullTotal));
};

using TestTuple = std::tuple<TString, TString, bool, TString, bool, bool, TString>;

class TransformTest : public ::testing::TestWithParam<TestTuple>
{
	// You can implement all the usual fixture class members here.
	// To access the test parameter, call GetParam() from class
	// TestWithParam<T>.
public:
	TSmartPath TransformPath(
		IFilesystemPtr spFilesystem,
		TSmartPath pathSrcBase,
		TSmartPath pathSrcRelativeToBase,
		bool bSrcIsFile,
		TSmartPath pathDst,
		bool bIgnoreFolders,
		bool bForceDirectories
	)
	{
		TString strFirstAltName = L"Copy of %name";
		TString strNextAltName = L"Copy (%count) of %name";

		TBasePathDataPtr spBasePath(new TBasePathData(0, pathSrcBase));
		TFileInfoPtr spFileInfo(new TFileInfo(spBasePath, pathSrcRelativeToBase, bSrcIsFile ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY,
			0, TFileTime(), TFileTime(), TFileTime(), 0));

		TDestinationPathProvider tDstPathProvider(spFilesystem, pathDst, bIgnoreFolders, bForceDirectories, strFirstAltName, strNextAltName);
		return tDstPathProvider.CalculateDestinationPath(spFileInfo);
	}
};

// base src path, full src path, is file, destination path, ignore folders, force directories, expected resulting path
INSTANTIATE_TEST_CASE_P(BasicTransformTests, TransformTest, ::testing::Values(
	/////////////////////////////////////////
	// full drive copy, std options, root
	std::make_tuple(TString(L"c:"), TString(L"c:"), false, TString(L"x:\\"), false, false, TString(L"x:\\c")),			// 0
	std::make_tuple(TString(L"c:\\"), TString(L"c:\\"), false, TString(L"x:\\"), false, false, TString(L"x:\\c")),		// 1
	// full drive copy, ignore folders, root
	std::make_tuple(TString(L"c:"), TString(L"c:"), false, TString(L"x:\\"), true, false, TString(L"x:\\c")),			// 2
	std::make_tuple(TString(L"c:\\"), TString(L"c:\\"), false, TString(L"x:\\"), true, false, TString(L"x:\\c")),		// 3
	// full drive copy, force directories, root
	std::make_tuple(TString(L"c:"), TString(L"c:"), false, TString(L"x:\\"), false, true, TString(L"x:\\c")),			// 4
	std::make_tuple(TString(L"c:\\"), TString(L"c:\\"), false, TString(L"x:\\"), false, true, TString(L"x:\\c")),		// 5

	/////////////////////////////////////////
	// full drive copy, std options, non-root
	std::make_tuple(TString(L"c:"), TString(L"c:\\folder\\file.txt"), true, TString(L"x:\\"), false, false, TString(L"x:\\c\\folder\\file.txt")),		// 6
	std::make_tuple(TString(L"c:\\"), TString(L"c:\\folder\\file.txt"), true, TString(L"x:\\"), false, false, TString(L"x:\\c\\folder\\file.txt")),		// 7
	// full drive copy, ignore folders, non-root
	std::make_tuple(TString(L"c:"), TString(L"c:\\folder\\file.txt"), true, TString(L"x:\\"), true, false, TString(L"x:\\file.txt")),					// 8
	std::make_tuple(TString(L"c:\\"), TString(L"c:\\folder\\file.txt"), true, TString(L"x:\\"), true, false, TString(L"x:\\file.txt")),					// 9
	// full drive copy, force directories, non-root
	std::make_tuple(TString(L"c:"), TString(L"c:\\folder\\file.txt"), true, TString(L"x:\\"), false, true, TString(L"x:\\c\\folder\\file.txt")),		// 10
	std::make_tuple(TString(L"c:\\"), TString(L"c:\\folder\\file.txt"), true, TString(L"x:\\"), false, true, TString(L"x:\\c\\folder\\file.txt")),		// 11

	/////////////////////////////////////////
	// folder copy, std options, folder root
	std::make_tuple(TString(L"c:\\folder"), TString(L"c:\\folder"), false, TString(L"x:\\"), false, false, TString(L"x:\\folder")),					// 12
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder\\"), false, TString(L"x:\\"), false, false, TString(L"x:\\folder")),				// 13
	// folder copy, ignore folders, folder root
	std::make_tuple(TString(L"c:\\folder"), TString(L"c:\\folder"), false, TString(L"x:\\"), true, false, TString(L"x:\\folder")),					// 14
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder\\"), false, TString(L"x:\\"), true, false, TString(L"x:\\folder")),				// 15
	// folder copy, force directories, folder root
	std::make_tuple(TString(L"c:\\folder"), TString(L"c:\\folder"), false, TString(L"x:\\"), false, true, TString(L"x:\\c\\folder")),					// 16
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder\\"), false, TString(L"x:\\"), false, true, TString(L"x:\\c\\folder")),				// 17

	/////////////////////////////////////////
	// folder copy, std options, non folder root
	std::make_tuple(TString(L"c:\\folder"), TString(L"c:\\folder\\folder2\\file.txt"), true, TString(L"x:\\"), false, false, TString(L"x:\\folder\\folder2\\file.txt")),		// 18
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder\\folder2\\file.txt"), true, TString(L"x:\\"), false, false, TString(L"x:\\folder\\folder2\\file.txt")),		// 19
	// folder copy, ignore folders, non folder root
	std::make_tuple(TString(L"c:\\folder"), TString(L"c:\\folder\\folder2\\file.txt"), true, TString(L"x:\\"), true, false, TString(L"x:\\file.txt")),			// 20
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder\\folder2\\file.txt"), true, TString(L"x:\\"), true, false, TString(L"x:\\file.txt")),		// 21
	// folder copy, force directories, non folder root
	std::make_tuple(TString(L"c:\\folder"), TString(L"c:\\folder\\folder2\\file.txt"), true, TString(L"x:\\"), false, true, TString(L"x:\\c\\folder\\folder2\\file.txt")),			// 22
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder\\folder2\\file.txt"), true, TString(L"x:\\"), false, true, TString(L"x:\\c\\folder\\folder2\\file.txt")),		// 23

	/////////////////////////////////////////
	// special cases
	// base path slightly differs from normal path (by a separator which should be ignored)
	std::make_tuple(TString(L"c:\\folder\\"), TString(L"c:\\folder"), false, TString(L"x:\\"), false, false, TString(L"x:\\folder")),			// 24

	// case insensitivity
	std::make_tuple(TString(L"c:\\Folder"), TString(L"c:\\folder"), false, TString(L"x:\\"), false, false, TString(L"x:\\Folder"))			// 25
));

TEST_P(TransformTest, PathTest)
{
	const TestTuple& rTestData = GetParam();

	// parameters
	TSmartPath pathSrcBase = PathFromWString(std::get<0>(rTestData));
	TSmartPath pathSrcRelativeToBase = PathFromWString(std::get<1>(rTestData));
	bool bSrcIsFile = std::get<2>(rTestData);
	TSmartPath pathDst = PathFromWString(std::get<3>(rTestData));
	TString strExpectedResultPath = std::get<6>(rTestData);

	bool bIgnoreFolders = std::get<4>(rTestData);
	bool bForceDirectories = std::get<5>(rTestData);

	// setup
	std::shared_ptr<NiceMock<IFilesystemMock> > spFilesystemMock(new NiceMock<IFilesystemMock>);
	EXPECT_CALL(*spFilesystemMock, PathExist(_))
		.WillRepeatedly(Return(false));

	// test execution
	TSmartPath pathResult = TransformPath(spFilesystemMock,
		pathSrcBase,
		pathSrcRelativeToBase,
		bSrcIsFile,
		pathDst,
		bIgnoreFolders,
		bForceDirectories);

	// verification
	ASSERT_STREQ(strExpectedResultPath.c_str(), pathResult.ToString());
}
