#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TModificationTracker.h"
#include "../TString.h"

using namespace chengine;

TEST(TModificationTrackerTests, DefaultConstructor)
{
	TModificationTracker<TString> tracker;

	EXPECT_TRUE(tracker.IsModified());
	EXPECT_EQ(TString(), tracker);
	EXPECT_FALSE(tracker.IsAdded());
}

TEST(TModificationTrackerTests, ValueConstructor_NotAdded)
{
	TModificationTracker<TString> tracker(L"SomeString", false);

	EXPECT_EQ(TString(L"SomeString"), tracker);
	EXPECT_TRUE(tracker.IsModified());
	EXPECT_FALSE(tracker.IsAdded());
}

TEST(TModificationTrackerTests, ValueConstructor_Added)
{
	TModificationTracker<TString> tracker(L"SomeString", true);

	EXPECT_EQ(TString(L"SomeString"), tracker);
	EXPECT_TRUE(tracker.IsModified());
	EXPECT_TRUE(tracker.IsAdded());
}

TEST(TModificationTrackerTests, CopyConstructor_NotAdded)
{
	TModificationTracker<TString> tracker(L"SomeString", false);
	TModificationTracker<TString> tracker2(tracker);

	EXPECT_EQ(TString(L"SomeString"), tracker2);
	EXPECT_TRUE(tracker2.IsModified());
	EXPECT_FALSE(tracker2.IsAdded());
}

TEST(TModificationTrackerTests, CopyConstructor_Added)
{
	TModificationTracker<TString> tracker(L"SomeString", true);
	TModificationTracker<TString> tracker2(tracker);

	EXPECT_EQ(TString(L"SomeString"), tracker2);
	EXPECT_TRUE(tracker2.IsModified());
	EXPECT_TRUE(tracker2.IsAdded());
}

///////////////////////////////////////////////////////////////////////////
TEST(TModificationTrackerTests, AssignmentOperator_ModificationTracker_NotAdded)
{
	TModificationTracker<TString> tracker(L"SomeString", false);
	TModificationTracker<TString> tracker2;
	
	tracker2 = tracker;

	EXPECT_EQ(TString(L"SomeString"), tracker2);
	EXPECT_TRUE(tracker2.IsModified());
	EXPECT_FALSE(tracker2.IsAdded());
}

TEST(TModificationTrackerTests, AssignmentOperator_ModificationTracker_Added)
{
	TModificationTracker<TString> tracker(L"SomeString", true);
	TModificationTracker<TString> tracker2;

	tracker2 = tracker;

	EXPECT_EQ(TString(L"SomeString"), tracker2);
	EXPECT_TRUE(tracker2.IsModified());
	EXPECT_TRUE(tracker2.IsAdded());
}

TEST(TModificationTrackerTests, AssignmentOperator_ValueModified)
{
	TModificationTracker<TString> tracker(L"SomeString", false);
	tracker.ClearModifications();

	tracker = L"OtherString";

	EXPECT_EQ(TString(L"OtherString"), tracker);
	EXPECT_TRUE(tracker.IsModified());
	EXPECT_FALSE(tracker.IsAdded());
}

TEST(TModificationTrackerTests, AssignmentOperator_ValueNotModified)
{
	TModificationTracker<TString> tracker(L"SomeString", false);
	tracker.ClearModifications();

	tracker = L"SomeString";

	EXPECT_EQ(TString(L"SomeString"), tracker);
	EXPECT_FALSE(tracker.IsModified());
	EXPECT_FALSE(tracker.IsAdded());
}

///////////////////////////////////////////////////////////////////////////
TEST(TModificationTrackerTests, Modify)
{
	TModificationTracker<TString> tracker(L"SomeString", false);
	tracker.ClearModifications();

	tracker.Modify() = L"SomeString";

	EXPECT_EQ(TString(L"SomeString"), tracker);
	EXPECT_TRUE(tracker.IsModified());
	EXPECT_FALSE(tracker.IsAdded());
}

///////////////////////////////////////////////////////////////////////////
TEST(TModificationTrackerTests, IsModified_IsAdded_ClearModifications)
{
	TModificationTracker<TString> tracker(L"SomeString", true);
	EXPECT_TRUE(tracker.IsModified());
	EXPECT_TRUE(tracker.IsAdded());

	tracker.ClearModifications();
	EXPECT_FALSE(tracker.IsModified());
	EXPECT_FALSE(tracker.IsAdded());
}
