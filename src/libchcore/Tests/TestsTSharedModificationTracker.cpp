#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSharedModificationTracker.h"
#include <bitset>
#include "../TString.h"

using namespace chcore;

TEST(TSharedModificationTrackerTests, DefaultConstructor)
{
	enum { eMyElement, eLast };
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);

	EXPECT_EQ(true, tracker.IsModified());
	EXPECT_EQ(true, setBits[eMyElement]);
	EXPECT_EQ(TString(), tracker);
}

TEST(TSharedModificationTrackerTests, AlmostCopyConstructor_Modified)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);

	std::bitset<eLast> setBits2;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker2(tracker, setBits2);

	EXPECT_EQ(true, tracker2.IsModified());
	EXPECT_EQ(true, setBits2[eMyElement]);
	EXPECT_EQ(TString(), tracker2);
}

TEST(TSharedModificationTrackerTests, AlmostCopyConstructor_Modified_WithValue)
{
	enum
	{
		eMyElement, eLast
	};

	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);
	tracker = L"SomeString";

	std::bitset<eLast> setBits2;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker2(tracker, setBits2);

	EXPECT_EQ(true, tracker2.IsModified());
	EXPECT_EQ(true, setBits2[eMyElement]);
	EXPECT_EQ(TString(L"SomeString"), tracker2);
}

TEST(TSharedModificationTrackerTests, AlmostCopyConstructor_Unmodified)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);
	tracker.MarkAsUnmodified();

	std::bitset<eLast> setBits2;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker2(tracker, setBits2);

	EXPECT_EQ(false, tracker2.IsModified());
	EXPECT_EQ(false, setBits2[eMyElement]);
}

TEST(TSharedModificationTrackerTests, AlmostCopyConstructor_Unmodified_WithValue)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);
	tracker = L"SomeString";
	tracker.MarkAsUnmodified();

	std::bitset<eLast> setBits2;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker2(tracker, setBits2);

	EXPECT_EQ(false, tracker2.IsModified());
	EXPECT_EQ(false, setBits2[eMyElement]);
	EXPECT_EQ(TString(L"SomeString"), tracker2);
}

TEST(TSharedModificationTrackerTests, ConstructorWithValue)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits, L"SomeString");

	EXPECT_EQ(true, tracker.IsModified());
	EXPECT_EQ(true, setBits[eMyElement]);
	EXPECT_EQ(TString(L"SomeString"), tracker);
}

///////////////////////////////////////////////////////////////////////////
TEST(TSharedModificationTrackerTests, AssignmentOperator_SharedTracker)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits, L"SomeString");

	std::bitset<eLast> setBits2;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker2(setBits2);
	tracker2 = tracker;

	EXPECT_EQ(true, tracker2.IsModified());
	EXPECT_EQ(true, setBits2[eMyElement]);
	EXPECT_EQ(TString(L"SomeString"), tracker2);
}

TEST(TSharedModificationTrackerTests, AssignmentOperator_OtherValue_ModifiesValue)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);

	tracker = L"SomeString";

	EXPECT_EQ(true, tracker.IsModified());
	EXPECT_EQ(true, setBits[eMyElement]);
	EXPECT_EQ(TString(L"SomeString"), tracker);
}

TEST(TSharedModificationTrackerTests, AssignmentOperator_OtherValue_DoesNotModifyValue)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits);

	tracker = L"SomeString";

	EXPECT_EQ(true, tracker.IsModified());
	EXPECT_EQ(true, setBits[eMyElement]);
	EXPECT_EQ(TString(L"SomeString"), tracker);
}

///////////////////////////////////////////////////////////////////////////
TEST(TSharedModificationTrackerTests, Modify)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits, L"SomeString");
	tracker.MarkAsUnmodified();

	tracker.Modify() = L"OtherString";

	EXPECT_EQ(true, tracker.IsModified());
	EXPECT_EQ(true, setBits[eMyElement]);
	EXPECT_EQ(TString(L"OtherString"), tracker);
}

///////////////////////////////////////////////////////////////////////////
TEST(TSharedModificationTrackerTests, MarkAsModified_MarkAsUnmodified)
{
	enum
	{
		eMyElement, eLast
	};
	std::bitset<eLast> setBits;
	TSharedModificationTracker<TString, std::bitset<eLast>, eMyElement> tracker(setBits, L"SomeString");

	EXPECT_EQ(true, tracker.IsModified());

	tracker.MarkAsUnmodified();
	EXPECT_EQ(false, tracker.IsModified());

	tracker.MarkAsModified();
	EXPECT_EQ(true, tracker.IsModified());
}
