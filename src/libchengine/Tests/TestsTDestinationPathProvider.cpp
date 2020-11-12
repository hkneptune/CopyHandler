#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TDestinationPathProvider.h"
#include "../TFileInfo.h"
#include "../TBasePathData.h"

using namespace testing;
using namespace chengine;
using namespace string;
using namespace chcore;

class IFilesystemMock : public IFilesystem
{
public:
	MOCK_METHOD1(PathExist, bool(const TSmartPath&));
	MOCK_METHOD5(SetFileDirBasicInfo, void(const TSmartPath& pathFileDir, DWORD dwAttributes, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime));
	MOCK_METHOD2(SetAttributes, void(const TSmartPath& pathFileDir, DWORD dwAttributes));

	MOCK_METHOD2(CreateDirectory, void(const TSmartPath&, bool));
	MOCK_METHOD1(RemoveDirectory, void(const TSmartPath& pathFile));
	MOCK_METHOD1(DeleteFile, void(const TSmartPath& pathFile));

	MOCK_METHOD3(GetFileInfo, void(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData));
	MOCK_METHOD2(FastMove, void(const TSmartPath&, const TSmartPath&));

	MOCK_METHOD2(CreateFinderObject, IFilesystemFindPtr(const TSmartPath& pathDir, const TSmartPath& pathMask));
	MOCK_METHOD4(CreateFileObject, IFilesystemFilePtr(IFilesystemFile::EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles));

	MOCK_METHOD2(GetPathsRelation, EPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond));

	MOCK_METHOD3(GetDynamicFreeSpace, void(const TSmartPath& path, unsigned long long& rullFree, unsigned long long& rullTotal));
};

// base src path, full src path, is file, destination path, ignore folders, force directories, expected resulting path
using TestTuple = std::tuple<const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, bool, const wchar_t*, bool, bool, const wchar_t*>;

class TransformTest : public ::testing::TestWithParam<TestTuple>
{
public:
	TSmartPath TransformPath(
		IFilesystemPtr spFilesystem,
		TSmartPath pathSrcBase,
		TSmartPath pathSrcRelativeToBase,
		TSmartPath pathBaseDstOverride,
		TSmartPath pathDstOverride,
		bool bSrcIsFile,
		TSmartPath pathDst,
		bool bIgnoreFolders,
		bool bForceDirectories
	)
	{
		TString strFirstAltName = L"Copy of %name";
		TString strNextAltName = L"Copy (%count) of %name";

		TBasePathDataPtr spBasePath(new TBasePathData(0, pathSrcBase));
		spBasePath->SetDestinationPath(pathBaseDstOverride);
		TFileInfoPtr spFileInfo(new TFileInfo(spBasePath, pathSrcRelativeToBase, bSrcIsFile ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY,
			0, TFileTime(), TFileTime(), TFileTime(), 0));
		spFileInfo->SetDstRelativePath(pathDstOverride);

		TDestinationPathProvider tDstPathProvider(spFilesystem, pathDst, bIgnoreFolders, bForceDirectories, strFirstAltName, strNextAltName);
		return tDstPathProvider.CalculateDestinationPath(spFileInfo);
	}
};

// base src path, full src path, is file, destination path, ignore folders, force directories, expected resulting path
INSTANTIATE_TEST_CASE_P(TransformTests_NoDstOverride, TransformTest, ::testing::Values(
	/////////////////////////////////////////
	// full drive copy, std options, root
	std::make_tuple(L"c:",		L"c:",		L"", L"", false, L"x:\\", false, false, L"x:\\c"),			// 0
	std::make_tuple(L"c:\\",	L"c:\\",	L"", L"", false, L"x:\\", false, false, L"x:\\c"),		// 1
	// full drive copy, ignore folders, root
	std::make_tuple(L"c:",		L"c:",		L"", L"", false, L"x:\\", true, false, L"x:\\c"),			// 2
	std::make_tuple(L"c:\\",	L"c:\\",	L"", L"", false, L"x:\\", true, false, L"x:\\c"),		// 3
	// full drive copy, force directories, root
	std::make_tuple(L"c:",		L"c:",		L"", L"", false, L"x:\\", false, true, L"x:\\c"),			// 4
	std::make_tuple(L"c:\\",	L"c:\\",	L"", L"", false, L"x:\\", false, true, L"x:\\c"),		// 5

	/////////////////////////////////////////
	// full drive copy, std options, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"", L"", true, L"x:\\", false, false, L"x:\\c\\folder\\file.txt"),		// 6
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"", L"", true, L"x:\\", false, false, L"x:\\c\\folder\\file.txt"),		// 7
	// full drive copy, ignore folders, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"", L"", true, L"x:\\", true, false, L"x:\\file.txt"),					// 8
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"", L"", true, L"x:\\", true, false, L"x:\\file.txt"),					// 9
	// full drive copy, force directories, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\file.txt"),		// 10
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\file.txt"),		// 11

	/////////////////////////////////////////
	// folder copy, std options, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"", L"", false, L"x:\\", false, false, L"x:\\folder"),					// 12
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"", L"", false, L"x:\\", false, false, L"x:\\folder"),				// 13
	// folder copy, ignore folders, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"", L"", false, L"x:\\", true, false, L"x:\\folder"),					// 14
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"", L"", false, L"x:\\", true, false, L"x:\\folder"),				// 15
	// folder copy, force directories, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"", L"", false, L"x:\\", false, true, L"x:\\c\\folder"),					// 16
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"", L"", false, L"x:\\", false, true, L"x:\\c\\folder"),				// 17

	/////////////////////////////////////////
	// folder copy, std options, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"", L"", true, L"x:\\", false, false, L"x:\\folder\\folder2\\file.txt"),		// 18
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"", L"", true, L"x:\\", false, false, L"x:\\folder\\folder2\\file.txt"),		// 19
	// folder copy, ignore folders, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"", L"", true, L"x:\\", true, false, L"x:\\file.txt"),			// 20
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"", L"", true, L"x:\\", true, false, L"x:\\file.txt"),		// 21
	// folder copy, force directories, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\file.txt"),			// 22
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\file.txt"),		// 23

	/////////////////////////////////////////
	// special cases
	// base path slightly differs from normal path (by a separator which should be ignored)
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder", L"", L"", false, L"x:\\", false, false, L"x:\\folder"),			// 24

	// case insensitivity
	std::make_tuple(L"c:\\Folder",		L"c:\\folder", L"", L"", false, L"x:\\", false, false, L"x:\\Folder")			// 25
));


// base src path, full src path, is file, destination path, ignore folders, force directories, expected resulting path
INSTANTIATE_TEST_CASE_P(TransformTests_TopLevelOverride, TransformTest, ::testing::Values(
	/////////////////////////////////////////
	// full drive copy, std options, root
	std::make_tuple(L"c:",		L"c:",		L"AltTL", L"", false, L"x:\\", false, false, L"x:\\AltTL"),			// 0
	std::make_tuple(L"c:\\",	L"c:\\",	L"AltTL", L"", false, L"x:\\", false, false, L"x:\\AltTL"),		// 1
	// full drive copy, ignore folders, root
	std::make_tuple(L"c:",		L"c:",		L"AltTL", L"", false, L"x:\\", true, false, L"x:\\c"),			// 2
	std::make_tuple(L"c:\\",	L"c:\\",	L"AltTL", L"", false, L"x:\\", true, false, L"x:\\c"),		// 3
	// full drive copy, force directories, root
	std::make_tuple(L"c:",		L"c:",		L"AltTL", L"", false, L"x:\\", false, true, L"x:\\c"),			// 4
	std::make_tuple(L"c:\\",	L"c:\\",	L"AltTL", L"", false, L"x:\\", false, true, L"x:\\c"),		// 5

	/////////////////////////////////////////
	// full drive copy, std options, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"AltTL", L"", true, L"x:\\", false, false, L"x:\\AltTL\\folder\\file.txt"),		// 6
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"AltTL", L"", true, L"x:\\", false, false, L"x:\\AltTL\\folder\\file.txt"),		// 7
	// full drive copy, ignore folders, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"AltTL", L"", true, L"x:\\", true, false, L"x:\\file.txt"),					// 8
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"AltTL", L"", true, L"x:\\", true, false, L"x:\\file.txt"),					// 9
	// full drive copy, force directories, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"AltTL", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\file.txt"),		// 10
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"AltTL", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\file.txt"),		// 11

	/////////////////////////////////////////
	// folder copy, std options, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"AltTL", L"", false, L"x:\\", false, false, L"x:\\AltTL"),					// 12
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"AltTL", L"", false, L"x:\\", false, false, L"x:\\AltTL"),				// 13
	// folder copy, ignore folders, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"AltTL", L"", false, L"x:\\", true, false, L"x:\\folder"),					// 14
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"AltTL", L"", false, L"x:\\", true, false, L"x:\\folder"),				// 15
	// folder copy, force directories, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"AltTL", L"", false, L"x:\\", false, true, L"x:\\c\\folder"),					// 16
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"AltTL", L"", false, L"x:\\", false, true, L"x:\\c\\folder"),				// 17

	/////////////////////////////////////////
	// folder copy, std options, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"AltTL", L"", true, L"x:\\", false, false, L"x:\\AltTL\\folder2\\file.txt"),		// 18
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"AltTL", L"", true, L"x:\\", false, false, L"x:\\AltTL\\folder2\\file.txt"),		// 19
	// folder copy, ignore folders, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"AltTL", L"", true, L"x:\\", true, false, L"x:\\file.txt"),			// 20
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"AltTL", L"", true, L"x:\\", true, false, L"x:\\file.txt"),		// 21
	// folder copy, force directories, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"AltTL", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\file.txt"),			// 22
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"AltTL", L"", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\file.txt"),		// 23

	/////////////////////////////////////////
	// special cases
	// base path slightly differs from normal path (by a separator which should be ignored)
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder", L"AltTL", L"", false, L"x:\\", false, false, L"x:\\AltTL"),			// 24

	// case insensitivity
	std::make_tuple(L"c:\\Folder",		L"c:\\folder", L"AltTL", L"", false, L"x:\\", false, false, L"x:\\AltTL")			// 25
));

INSTANTIATE_TEST_CASE_P(TransformTests_DstOverride, TransformTest, ::testing::Values(
	/////////////////////////////////////////
	// full drive copy, std options, root
	std::make_tuple(L"c:",		L"c:",		L"", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),			// 0
	std::make_tuple(L"c:\\",	L"c:\\",	L"", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),		// 1
	// full drive copy, ignore folders, root
	std::make_tuple(L"c:",		L"c:",		L"", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),			// 2
	std::make_tuple(L"c:\\",	L"c:\\",	L"", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),		// 3
	// full drive copy, force directories, root
	std::make_tuple(L"c:",		L"c:",		L"", L"AltName", false, L"x:\\", false, true, L"x:\\AltName"),			// 4
	std::make_tuple(L"c:\\",	L"c:\\",	L"", L"AltName", false, L"x:\\", false, true, L"x:\\AltName"),		// 5

	/////////////////////////////////////////
	// full drive copy, std options, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"", L"AltName", true, L"x:\\", false, false, L"x:\\c\\folder\\AltName"),		// 6
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"", L"AltName", true, L"x:\\", false, false, L"x:\\c\\folder\\AltName"),		// 7
	// full drive copy, ignore folders, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),					// 8
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),					// 9
	// full drive copy, force directories, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\AltName"),		// 10
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\AltName"),		// 11

	/////////////////////////////////////////
	// folder copy, std options, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),					// 12
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),				// 13
	// folder copy, ignore folders, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),					// 14
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),				// 15
	// folder copy, force directories, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"", L"AltName", false, L"x:\\", false, true, L"x:\\c\\AltName"),					// 16
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"", L"AltName", false, L"x:\\", false, true, L"x:\\c\\AltName"),				// 17

	/////////////////////////////////////////
	// folder copy, std options, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"", L"AltName", true, L"x:\\", false, false, L"x:\\folder\\folder2\\AltName"),		// 18
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"", L"AltName", true, L"x:\\", false, false, L"x:\\folder\\folder2\\AltName"),		// 19
	// folder copy, ignore folders, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),			// 20
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),		// 21
	// folder copy, force directories, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\AltName"),			// 22
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\AltName"),		// 23

	/////////////////////////////////////////
	// special cases
	// base path slightly differs from normal path (by a separator which should be ignored)
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder", L"", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),			// 24

	// case insensitivity
	std::make_tuple(L"c:\\Folder",		L"c:\\folder", L"", L"AltName", false, L"x:\\", false, false, L"x:\\AltName")			// 25
));

INSTANTIATE_TEST_CASE_P(TransformTests_TopLevelOverrideAndDstOverride, TransformTest, ::testing::Values(
	/////////////////////////////////////////
	// full drive copy, std options, root
	std::make_tuple(L"c:",		L"c:",		L"AltTL", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),			// 0
	std::make_tuple(L"c:\\",	L"c:\\",	L"AltTL", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),		// 1
	// full drive copy, ignore folders, root
	std::make_tuple(L"c:",		L"c:",		L"AltTL", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),			// 2
	std::make_tuple(L"c:\\",	L"c:\\",	L"AltTL", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),		// 3
	// full drive copy, force directories, root
	std::make_tuple(L"c:",		L"c:",		L"AltTL", L"AltName", false, L"x:\\", false, true, L"x:\\AltName"),			// 4
	std::make_tuple(L"c:\\",	L"c:\\",	L"AltTL", L"AltName", false, L"x:\\", false, true, L"x:\\AltName"),		// 5

	/////////////////////////////////////////
	// full drive copy, std options, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, false, L"x:\\AltTL\\folder\\AltName"),		// 6
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, false, L"x:\\AltTL\\folder\\AltName"),		// 7
	// full drive copy, ignore folders, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"AltTL", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),					// 8
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"AltTL", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),					// 9
	// full drive copy, force directories, non-root
	std::make_tuple(L"c:",		L"c:\\folder\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\AltName"),		// 10
	std::make_tuple(L"c:\\",	L"c:\\folder\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\AltName"),		// 11

	/////////////////////////////////////////
	// folder copy, std options, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"AltTL", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),					// 12
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"AltTL", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),				// 13
	// folder copy, ignore folders, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"AltTL", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),					// 14
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"AltTL", L"AltName", false, L"x:\\", true, false, L"x:\\AltName"),				// 15
	// folder copy, force directories, folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder", L"AltTL", L"AltName", false, L"x:\\", false, true, L"x:\\c\\AltName"),					// 16
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\", L"AltTL", L"AltName", false, L"x:\\", false, true, L"x:\\c\\AltName"),				// 17

	/////////////////////////////////////////
	// folder copy, std options, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, false, L"x:\\AltTL\\folder2\\AltName"),		// 18
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, false, L"x:\\AltTL\\folder2\\AltName"),		// 19
	// folder copy, ignore folders, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"AltTL", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),			// 20
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"AltTL", L"AltName", true, L"x:\\", true, false, L"x:\\AltName"),		// 21
	// folder copy, force directories, non folder root
	std::make_tuple(L"c:\\folder",		L"c:\\folder\\folder2\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\AltName"),			// 22
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder\\folder2\\file.txt", L"AltTL", L"AltName", true, L"x:\\", false, true, L"x:\\c\\folder\\folder2\\AltName"),		// 23

	/////////////////////////////////////////
	// special cases
	// base path slightly differs from normal path (by a separator which should be ignored)
	std::make_tuple(L"c:\\folder\\",	L"c:\\folder", L"AltTL", L"AltName", false, L"x:\\", false, false, L"x:\\AltName"),			// 24

	// case insensitivity
	std::make_tuple(L"c:\\Folder",		L"c:\\folder", L"AltTL", L"AltName", false, L"x:\\", false, false, L"x:\\AltName")			// 25
));

TEST_P(TransformTest, PathTest)
{
	const TestTuple& rTestData = GetParam();

	// parameters
	TSmartPath pathSrcBase = PathFromString(std::get<0>(rTestData));
	TSmartPath pathSrcRelativeToBase = PathFromString(std::get<1>(rTestData));
	TSmartPath pathBaseDstOverride = PathFromString(std::get<2>(rTestData));
	TSmartPath pathDstOverride = PathFromString(std::get<3>(rTestData));
	bool bSrcIsFile = std::get<4>(rTestData);
	TSmartPath pathDst = PathFromString(std::get<5>(rTestData));
	TString strExpectedResultPath = std::get<8>(rTestData);

	bool bIgnoreFolders = std::get<6>(rTestData);
	bool bForceDirectories = std::get<7>(rTestData);

	// setup
	std::shared_ptr<NiceMock<IFilesystemMock> > spFilesystemMock(new NiceMock<IFilesystemMock>);
	EXPECT_CALL(*spFilesystemMock, PathExist(_))
		.WillRepeatedly(Return(false));

	// test execution
	TSmartPath pathResult = TransformPath(spFilesystemMock,
		pathSrcBase,
		pathSrcRelativeToBase,
		pathBaseDstOverride,
		pathDstOverride,
		bSrcIsFile,
		pathDst,
		bIgnoreFolders,
		bForceDirectories);

	// verification
	ASSERT_STREQ(strExpectedResultPath.c_str(), pathResult.ToString());
}
