#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TTaskManagerSerializer.h"

using namespace chcore;

TEST(TaskManagerSerializer, BasicTest)
{
	TTaskManagerSerializer tSerializer(PathFromString(_T("c:\\projects\\abc.sqlite")), PathFromString(_T("c:\\projects\\")));
	tSerializer.Setup();

	EXPECT_TRUE(true);
}
