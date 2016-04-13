#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TDestinationPathProvider.h"
#include "../TFileInfo.h"
#include "../TBasePathData.h"

using namespace testing;
using namespace chcore;

class IFilesystemMock : public IFilesystem
{
public:
	MOCK_METHOD1(PathExist, bool(const TSmartPath&));
	MOCK_METHOD4(SetFileDirectoryTime, void(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime));
	MOCK_METHOD2(SetAttributes, void(const TSmartPath& pathFileDir, DWORD dwAttributes));

	MOCK_METHOD2(CreateDirectory, void(const TSmartPath& pathDirectory, bool bCreateFullPath));
	MOCK_METHOD1(RemoveDirectory, void(const TSmartPath& pathFile));
	MOCK_METHOD1(DeleteFile, void(const TSmartPath& pathFile));

	MOCK_METHOD3(GetFileInfo, void(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData));
	MOCK_METHOD2(FastMove, void(const TSmartPath& pathSource, const TSmartPath& pathDestination));

	MOCK_METHOD2(CreateFinderObject, IFilesystemFindPtr(const TSmartPath& pathDir, const TSmartPath& pathMask));
	MOCK_METHOD2(CreateFileObject, IFilesystemFilePtr(const TSmartPath& pathFile, bool bNoBuffering));

	MOCK_METHOD2(GetPathsRelation, EPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond));

	MOCK_METHOD2(GetDynamicFreeSpace, void(const TSmartPath& path, unsigned long long& rullFree));
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

INSTANTIATE_TEST_CASE_P(BasicTransformTests, TransformTest, ::testing::Values(
	std::make_tuple(TString(L"c:"), TString(L"c:"), true, TString(L"x:\\"), false, false, TString(L"x:\\c"))
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
	std::shared_ptr<StrictMock<IFilesystemMock> > spFilesystemMock(new StrictMock<IFilesystemMock>);
	EXPECT_CALL(*spFilesystemMock, PathExist(_))
		.Times(0);

	// test execution
	TSmartPath pathResult = TransformPath(spFilesystemMock,
		pathSrcBase,
		pathSrcRelativeToBase,
		bSrcIsFile,
		pathDst,
		bIgnoreFolders,
		bForceDirectories);

	// verification
	EXPECT_STREQ(L"x:\\c", pathResult.ToString());
}
