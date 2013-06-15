#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSimpleTimer.h"
#include "../ITimestampProvider.h"

class ITimestampProviderMock : public chcore::ITimestampProvider
{
public:
	MOCK_CONST_METHOD0(GetCurrentTimestamp, unsigned long long());
};

using namespace testing;
using namespace chcore;

TEST(TSimpleTimerTests, SimpleStartStop_CalculatesTimeProperly)
{
	boost::shared_ptr<StrictMock<ITimestampProviderMock> > spTimestampProviderMock(new StrictMock<ITimestampProviderMock>);

	EXPECT_CALL(*spTimestampProviderMock, GetCurrentTimestamp())
		.WillOnce(Return(30))
		.WillOnce(Return(100));

	TSimpleTimer tTimer(false, spTimestampProviderMock);
	tTimer.Start();
	EXPECT_EQ(70, tTimer.Stop());
	EXPECT_EQ(70, tTimer.GetTotalTime());
	EXPECT_EQ(100, tTimer.GetLastTimestamp());
}

TEST(TSimpleTimerTests, TickWithoutStarting_UpdatesLastTimestamp)
{
	boost::shared_ptr<StrictMock<ITimestampProviderMock> > spTimestampProviderMock(new StrictMock<ITimestampProviderMock>);

	EXPECT_CALL(*spTimestampProviderMock, GetCurrentTimestamp())
		.WillOnce(Return(30));

	TSimpleTimer tTimer(false, spTimestampProviderMock);
	tTimer.Tick();
	EXPECT_EQ(30, tTimer.GetLastTimestamp());
}

TEST(TSimpleTimerTests, StartAndTicking_CorrectlyCountsTime)
{
	boost::shared_ptr<StrictMock<ITimestampProviderMock> > spTimestampProviderMock(new StrictMock<ITimestampProviderMock>);

	EXPECT_CALL(*spTimestampProviderMock, GetCurrentTimestamp())
		.WillOnce(Return(30))
		.WillOnce(Return(40))
		.WillOnce(Return(990));

	TSimpleTimer tTimer(false, spTimestampProviderMock);
	tTimer.Start();
	tTimer.Tick();
	EXPECT_EQ(10, tTimer.GetTotalTime());
	tTimer.Tick();
	EXPECT_EQ((990 - 30), tTimer.GetTotalTime());
}

TEST(TSimpleTimerTests, StartAndTicking_TickReturnsLastTimestamp)
{
	boost::shared_ptr<StrictMock<ITimestampProviderMock> > spTimestampProviderMock(new StrictMock<ITimestampProviderMock>);

	EXPECT_CALL(*spTimestampProviderMock, GetCurrentTimestamp())
		.WillOnce(Return(30))
		.WillOnce(Return(40))
		.WillOnce(Return(990));

	TSimpleTimer tTimer(false, spTimestampProviderMock);
	tTimer.Start();
	EXPECT_EQ(40, tTimer.Tick());
	EXPECT_EQ(990, tTimer.Tick());
}

TEST(TSimpleTimerTests, Reset_StopsTimeCountingAndResetsTime)
{
	boost::shared_ptr<StrictMock<ITimestampProviderMock> > spTimestampProviderMock(new StrictMock<ITimestampProviderMock>);

	EXPECT_CALL(*spTimestampProviderMock, GetCurrentTimestamp())
		.WillOnce(Return(30))
		.WillOnce(Return(40));

	TSimpleTimer tTimer(false, spTimestampProviderMock);
	tTimer.Start();
	tTimer.Tick();

	tTimer.Reset();
	EXPECT_EQ(tTimer.GetTotalTime(), 0);
	EXPECT_EQ(tTimer.GetLastTimestamp(), 0);
}
