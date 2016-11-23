#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSpeedTracker.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TSpeedTrackerTests, FirstSample_DoesNotChangeSpeed)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(100, 0);		// 100 bytes at timestamp 0
	EXPECT_DOUBLE_EQ(0.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, SingleCorrectSample_ExactSingleSampleBoundary)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);
	tTracker.AddSample(8000, 100);		// 100 bytes at timestamp 100

	// should be a single sample of 80k/s (due to the normalization)
	// with all other samples equal to 0 giving finally 8000/s;
	// (or from a different perspective - we processes 8000 items in the entire
	// second that we track)
	EXPECT_DOUBLE_EQ(8000.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, SingleCorrectSample_NotAtSampleBoundary)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);
	tTracker.AddSample(8000, 333);		// 100 bytes at timestamp 333

	// processed 8000 items in entire second (as there were no other samples)
	EXPECT_DOUBLE_EQ(8000.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, ClearContents)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);
	tTracker.AddSample(8000, 100);		// 100 bytes at timestamp 333

	tTracker.Clear();

	EXPECT_DOUBLE_EQ(0.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleAlignedSamples_SampleRollOver)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 100);
	tTracker.AddSample(4000, 500);
	tTracker.AddSample(5000, 1500);	// one second sample; should overwrite entire sample cache

	// processed 8000 items in entire second (as there were no other samples)
	EXPECT_DOUBLE_EQ(5000.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleUnAlignedSamplesWithNoPartialSamples_SampleRollOver)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 110);
	tTracker.AddSample(4000, 530);
	tTracker.AddSample(5000, 1530);	// one second sample; should overwrite entire sample cache

	// processed 8000 items in entire second (as there were no other samples)
	EXPECT_DOUBLE_EQ(5000.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleUnAlignedSamplesWithPartialSamples_SampleRollOver)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 110);
	tTracker.AddSample(4000, 140);	// a partial sample (30ms)
	tTracker.AddSample(5000, 1140);	// one second sample; should overwrite entire sample cache

	// processed 8000 items in entire second (as there were no other samples)
	EXPECT_DOUBLE_EQ(5000.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleSamplesRecedingInTime_ThrowsException)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 110);
	EXPECT_THROW(tTracker.AddSample(4000, 40), TCoreException);
}

TEST(TSpeedTrackerTests, MultipleSamplesCornerCase_VeryLargeInterval)
{
	TSpeedTracker tTracker(1000, 100);		// track last second with 100ms samples
	tTracker.AddSample(0, 0);			// start
	// interval exceeds 32bit by a lot (was causing problems on 32-bit windows)
	EXPECT_NO_THROW(tTracker.AddSample(40000, 18446744073709551546));
}

TEST(TSpeedTrackerTests, SingleSampleWithEqualTrackTimeAndSampleTime)
{
	TSpeedTracker tTracker(1000, 1000);		// track last second with 1 second samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 1000);

	// processed 8000 items in entire second (as there were no other samples)
	EXPECT_DOUBLE_EQ(8000.0, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleSamplesWithEqualTrackTimeAndSampleTime)
{
	TSpeedTracker tTracker(1000, 1000);		// track last second with 1 second samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 1000);
	tTracker.AddSample(1000, 1500);
	tTracker.AddSample(30000, 1800);

	// processed 8000 items in entire second (as there were no other samples)
	EXPECT_DOUBLE_EQ(32600, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleSamplesWithBigTrackTime)
{
	TSpeedTracker tTracker(5000, 1000);		// track last 5 seconds with 1 second samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 900);
	tTracker.AddSample(1000, 1700);		// 800
	tTracker.AddSample(30000, 2600);	// 900

	// processed 39000 items in last 5s, so speed is 7800
	EXPECT_DOUBLE_EQ(7800, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleSamplesWithBigTrackTimeAndSampleTime)
{
	TSpeedTracker tTracker(5000, 5000);		// track last 5 seconds with 1 second samples
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 900);
	tTracker.AddSample(1000, 1700);		// 800
	tTracker.AddSample(30000, 2600);	// 900

	// processed 39000 items in last 5s, so speed is 7800
	EXPECT_DOUBLE_EQ(7800, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleSamplesWithTrackTimeAndSampleTimeDoesntMatch)
{
	TSpeedTracker tTracker(1000, 300);		// there should be 3 samples inside
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(8000, 400);
	tTracker.AddSample(1000, 800);
	tTracker.AddSample(27000, 900);

	// processed 39000 items in last 5s, so speed is 7800
	EXPECT_DOUBLE_EQ(40000, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, ConstructionWithBadParams)
{
	// zero length sample is not allowed
	EXPECT_THROW(TSpeedTracker tTracker(5000, 0), TCoreException);
	// tracking time less than sample time is not allowed
	EXPECT_THROW(TSpeedTracker tTracker(0, 1000), TCoreException);
}

TEST(TSpeedTrackerTests, MultipleVeryShortSamplesWithZeroIntervalData_CornerCase)
{
	TSpeedTracker tTracker(1000, 100);
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(80, 10);
	tTracker.AddSample(120, 10);
	tTracker.AddSample(10, 10);
	tTracker.AddSample(50, 10);
	tTracker.AddSample(0, 10);
	tTracker.AddSample(0, 10);
	tTracker.AddSample(10, 20);
	tTracker.AddSample(10, 20);
	tTracker.AddSample(10, 20);
	tTracker.AddSample(0, 20);
	tTracker.AddSample(10, 30);
	tTracker.AddSample(10, 30);
	tTracker.AddSample(0, 30);
	tTracker.AddSample(10, 40);
	tTracker.AddSample(20, 40);	// <-- this sample might be skipped when calculating speed
	tTracker.AddSample(0, 40);
	tTracker.AddSample(0, 40);

	// 340 in 1000ms = 340 items/s
	// however 320 is reported, because the sample (marked above) is not counted
	// in the speed calculation (has the same timestamp as previous sample)
	// and this is the last timestamp used in the sample list.

	EXPECT_DOUBLE_EQ(320, tTracker.GetSpeed());
}

TEST(TSpeedTrackerTests, MultipleVeryShortSamplesWithZeroIntervalData)
{
	TSpeedTracker tTracker(1000, 100);
	tTracker.AddSample(0, 0);			// start
	tTracker.AddSample(80, 10);
	tTracker.AddSample(120, 10);
	tTracker.AddSample(10, 10);
	tTracker.AddSample(50, 10);
	tTracker.AddSample(0, 10);
	tTracker.AddSample(0, 10);
	tTracker.AddSample(10, 20);
	tTracker.AddSample(10, 20);
	tTracker.AddSample(10, 20);
	tTracker.AddSample(0, 20);
	tTracker.AddSample(10, 30);
	tTracker.AddSample(10, 30);
	tTracker.AddSample(0, 30);
	tTracker.AddSample(10, 40);
	tTracker.AddSample(20, 40);
	tTracker.AddSample(0, 40);
	tTracker.AddSample(0, 40);
	tTracker.AddSample(0, 50);

	// 340 in 1000ms = 340 items/s
	// this case is similar to MultipleVeryShortSamplesWithZeroIntervalData_CornerCase
	// but here we have additional empty sample added at the end with newer timestamp
	// that should trigger inclusion of the previous same-timestamp samples

	EXPECT_DOUBLE_EQ(340, tTracker.GetSpeed());
}
