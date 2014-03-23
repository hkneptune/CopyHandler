#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TString.h"
#include "../TStringException.h"
#include "../TStringArray.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TStringTests, DefaultConstructor_CheckEmptyString)
{
	TString strEmpty;
	EXPECT_EQ(strEmpty.IsEmpty(), true);
}

TEST(TStringTests, WcharConstructor_CompareContent)
{
	TString strValue(L"Some test string");
	EXPECT_EQ(strValue, L"Some test string");
}

TEST(TStringTests, WcharConstructorWithNullValue_CompareContent)
{
	TString strValue(NULL);
	EXPECT_EQ(strValue, L"");
}

TEST(TStringTests, WcharRangeConstructor_CompareContent)
{
	const wchar_t* pszText = L"Some test string";
	TString strValue(pszText, pszText + 5);
	EXPECT_EQ(strValue, L"Some ");
}

TEST(TStringTests, WcharRangeConstructor_WithUnsupportedNullValues)
{
	const wchar_t* pszText = L"Some test string";
	EXPECT_THROW(TString strValue(pszText, (const wchar_t*)NULL), TStringException);
	EXPECT_THROW(TString strValue((const wchar_t*)NULL, pszText), TStringException);
}

TEST(TStringTests, WcharRangeConstructor_WithSupportedNullValues_CompareContent)
{
	TString strValue((const wchar_t*)NULL, (const wchar_t*)NULL);
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, WcharRangeConstructor_WithTooLongString)
{
	const wchar_t* pszText = L"Some test string";
	EXPECT_THROW(TString strValue(pszText, pszText + 5, 4), TStringException);
}

TEST(TStringTests, WcharRangeConstructorWithMaxStringLen_CompareContent)
{
	const wchar_t* pszText = L"Some test string";
	TString strValue(pszText, pszText + 5, 5);
	EXPECT_EQ(strValue, _T("Some "));
}

TEST(TStringTests, AssignmentOperator_WithNormalValue)
{
	TString strValue;
	strValue = _T("Some sample string");
	EXPECT_EQ(strValue, _T("Some sample string"));
}

TEST(TStringTests, AssignmentOperator_WithNullValue)
{
	TString strValue;
	strValue = NULL;
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, AssignmentOperator_WithTString)
{
	TString strValue(_T("Some initial string"));
	strValue = TString(_T("Overwriting string"));
	EXPECT_EQ(strValue, _T("Overwriting string"));
}

TEST(TStringTests, AssignmentOperator_WithEmptyTString)
{
	TString strValue(_T("Some initial string"));
	strValue = TString();
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, OperatorAdd_WithPtrToString)
{
	TString strValue = TString(_T("Text: ")) + _T("Some test string");
	EXPECT_EQ(strValue, _T("Text: Some test string"));
}

TEST(TStringTests, OperatorAdd_WithNullPtrToString)
{
	TString strValue = TString(_T("Text: ")) + (const wchar_t*)NULL;
	EXPECT_EQ(strValue, _T("Text: "));
}

TEST(TStringTests, OperatorAdd_EmptyWithEmptyPtrToString)
{
	TString strValue = TString() + _T("");
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, OperatorAdd_EmptyWithNullPtrToString)
{
	TString strValue = TString() + (const wchar_t*)NULL;
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, OperatorAdd_WithTString)
{
	TString strValue = TString(_T("Text: ")) + TString(_T("Some test string"));
	EXPECT_EQ(strValue, _T("Text: Some test string"));
}

TEST(TStringTests, OperatorInplaceAdd_WithPtrToString)
{
	TString strValue(_T("Text: "));
	strValue += _T("Some test string");
	EXPECT_EQ(strValue, _T("Text: Some test string"));
}

TEST(TStringTests, OperatorInplaceAdd_WithNullPtrToString)
{
	TString strValue(_T("Text: "));
	strValue += (const wchar_t*)NULL;
	EXPECT_EQ(strValue, _T("Text: "));
}

TEST(TStringTests, OperatorInplaceAdd_EmptyWithEmptyPtrToString)
{
	TString strValue;
	strValue += _T("");
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, OperatorInplaceAdd_EmptyWithNullPtrToString)
{
	TString strValue;
	strValue += (const wchar_t*)NULL;
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, OperatorInplaceAdd_WithTString)
{
	TString strValue(_T("Text: "));
	strValue += TString(_T("Some test string"));
	EXPECT_EQ(strValue, _T("Text: Some test string"));
}

// comparisons
TEST(PtrToStringTests, OperatorLT_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue < _T("The other string"));
}

TEST(PtrToStringTests, OperatorLTE_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue <= _T("Some string2"));
	EXPECT_TRUE(strValue <= _T("Some string"));
}

TEST(PtrToStringTests, OperatorEQ_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue == _T("Some string"));
}

TEST(PtrToStringTests, OperatorGT_WithPtrToString)
{
	TString strValue(_T("The string"));
	EXPECT_TRUE(strValue > _T("Some other string"));
}

TEST(PtrToStringTests, OperatorGTE_WithPtrToString)
{
	TString strValue(_T("The string"));
	EXPECT_TRUE(strValue >= _T("The string"));
	EXPECT_TRUE(strValue >= _T("The string"));
}

// comparisons with TStrings
TEST(TStringTests, OperatorLT_WithTString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue < TString(_T("The other string")));
}

TEST(TStringTests, OperatorLTE_WithTString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue <= TString(_T("Some string2")));
	EXPECT_TRUE(strValue <= TString(_T("Some string")));
}

TEST(TStringTests, OperatorEQ_WithTString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue == TString(_T("Some string")));
}

TEST(TStringTests, OperatorGT_WithTString)
{
	TString strValue(_T("The string"));
	EXPECT_TRUE(strValue > TString(_T("Some other string")));
}

TEST(TStringTests, OperatorGTE_WithTString)
{
	TString strValue(_T("The string"));
	EXPECT_TRUE(strValue >= TString(_T("The string")));
	EXPECT_TRUE(strValue >= TString(_T("The string")));
}

// cast
TEST(TStringTests, CastToPtrWcharT)
{
	TString strValue(_T("Some string"));
	EXPECT_STREQ((const wchar_t*)strValue, _T("Some string"));
}

TEST(TStringTests, CastEmptyToPtrWcharT)
{
	TString strValue;
	EXPECT_STREQ((const wchar_t*)strValue, _T(""));
}

// append
TEST(TStringTests, AppendString_WithPtrToString)
{
	TString strValue(_T("Some string"));
	strValue.Append(_T("... appended."));
	EXPECT_EQ(strValue, _T("Some string... appended."));
}

TEST(TStringTests, AppendString_WithNullPtrToString)
{
	TString strValue(_T("Some string"));
	strValue.Append((const wchar_t*)NULL);
	EXPECT_EQ(strValue, _T("Some string"));
}

TEST(TStringTests, AppendString_WithTString)
{
	TString strValue(_T("Some string"));
	strValue.Append(TString(_T("... appended.")));
	EXPECT_EQ(strValue, _T("Some string... appended."));
}

// left, right, mid
TEST(TStringTests, Left)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.Left(3), _T("Som"));
	EXPECT_EQ(strValue.Left(0), _T(""));
	EXPECT_EQ(strValue.Left(100), _T("Some string"));
}

TEST(TStringTests, Right)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.Right(3), _T("ing"));
	EXPECT_EQ(strValue.Right(0), _T(""));
	EXPECT_EQ(strValue.Right(100), _T("Some string"));
}

TEST(TStringTests, Mid)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.Mid(3), _T("e string"));
	EXPECT_EQ(strValue.Mid(3, 2), _T("e "));
	EXPECT_EQ(strValue.Mid(100, 50), _T(""));
}

TEST(TStringTests, MidRange)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.MidRange(3, 4), _T("e"));
	EXPECT_EQ(strValue.MidRange(3, 5), _T("e "));
	EXPECT_EQ(strValue.MidRange(3, 50), _T("e string"));

	EXPECT_EQ(strValue.MidRange(3, 2), _T(""));
	EXPECT_EQ(strValue.MidRange(50, 55), _T(""));
	EXPECT_EQ(strValue.MidRange(50, 0), _T(""));
}

// left self, right self, mid self
TEST(TStringTests, LeftSelf_NormalCase)
{
	TString strValue(_T("Some string"));
	strValue.LeftSelf(3);
	EXPECT_EQ(strValue, _T("Som"));
}

TEST(TStringTests, LeftSelf_ZeroLength)
{
	TString strValue(_T("Some string"));
	strValue.LeftSelf(0);
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, LeftSelf_LongerThanInput)
{
	TString strValue(_T("Some string"));
	strValue.LeftSelf(100);
	EXPECT_EQ(strValue, _T("Some string"));
}

TEST(TStringTests, RightSelf_NormalCase)
{
	TString strValue(_T("Some string"));
	strValue.RightSelf(3);
	EXPECT_EQ(strValue, _T("ing"));
}

TEST(TStringTests, RightSelf_ZeroLength)
{
	TString strValue(_T("Some string"));
	strValue.RightSelf(0);
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, RightSelf_LongerThanInput)
{
	TString strValue(_T("Some string"));
	strValue.RightSelf(100);
	EXPECT_EQ(strValue, _T("Some string"));
}

TEST(TStringTests, MidSelf_NormalCase)
{
	TString strValue(_T("Some string"));
	strValue.MidSelf(3);
	EXPECT_EQ(strValue, _T("e string"));
}

TEST(TStringTests, MidSelf_ZeroLength)
{
	TString strValue(_T("Some string"));
	strValue.MidSelf(3, 2);
	EXPECT_EQ(strValue, _T("e "));
}

TEST(TStringTests, MidSelf_LongerThanInput)
{
	TString strValue(_T("Some string"));
	strValue.MidSelf(100, 50);
	EXPECT_EQ(strValue, _T(""));
}

// delete
TEST(TStringTests, Delete_Normal)
{
	TString strValue(_T("Some string"));
	strValue.Delete(1, 2);
	EXPECT_EQ(strValue, _T("Se string"));
}

TEST(TStringTests, Delete_TooMuch)
{
	TString strValue(_T("Some string"));
	strValue.Delete(1, 200);
	EXPECT_EQ(strValue, _T("S"));
}

TEST(TStringTests, Delete_OutOfRange)
{
	TString strValue(_T("Some string"));
	strValue.Delete(50, 3);
	EXPECT_EQ(strValue, _T("Some string"));
}

TEST(TStringTests, Delete_ZeroLength)
{
	TString strValue(_T("Some string"));
	strValue.Delete(1, 0);
	EXPECT_EQ(strValue, _T("Some string"));
}

TEST(TStringTests, Delete_OnEmptyString)
{
	TString strValue;
	strValue.Delete(1, 0);
	EXPECT_EQ(strValue, _T(""));
}

// split
TEST(TStringTests, Split_NormalStringSingleSeparator)
{
	TString strData(_T("A;B;c;d;e;%$;;;"));
	TStringArray arrSplitted;
	strData.Split(_T(";"), arrSplitted);
	EXPECT_EQ(arrSplitted.GetAt(0), _T("A"));
	EXPECT_EQ(arrSplitted.GetAt(1), _T("B"));
	EXPECT_EQ(arrSplitted.GetAt(2), _T("c"));
	EXPECT_EQ(arrSplitted.GetAt(3), _T("d"));
	EXPECT_EQ(arrSplitted.GetAt(4), _T("e"));
	EXPECT_EQ(arrSplitted.GetAt(5), _T("%$"));
	EXPECT_EQ(arrSplitted.GetAt(6), _T(""));
	EXPECT_EQ(arrSplitted.GetAt(7), _T(""));
	EXPECT_EQ(arrSplitted.GetAt(8), _T(""));
	EXPECT_THROW(arrSplitted.GetAt(9), TCoreException);
}

TEST(TStringTests, Split_NormalStringMultipleSeparators)
{
	TString strData(_T("A;B;c:d-e;%$;^;"));
	TStringArray arrSplitted;
	strData.Split(_T(";:-^"), arrSplitted);
	EXPECT_EQ(arrSplitted.GetAt(0), _T("A"));
	EXPECT_EQ(arrSplitted.GetAt(1), _T("B"));
	EXPECT_EQ(arrSplitted.GetAt(2), _T("c"));
	EXPECT_EQ(arrSplitted.GetAt(3), _T("d"));
	EXPECT_EQ(arrSplitted.GetAt(4), _T("e"));
	EXPECT_EQ(arrSplitted.GetAt(5), _T("%$"));
	EXPECT_EQ(arrSplitted.GetAt(6), _T(""));
	EXPECT_EQ(arrSplitted.GetAt(7), _T(""));
	EXPECT_EQ(arrSplitted.GetAt(8), _T(""));
	EXPECT_THROW(arrSplitted.GetAt(9), TCoreException);
}

TEST(TStringTests, CompareCaseSensitive_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_LT(strValue.Compare(_T("The string")), 0);
	EXPECT_GT(strValue.Compare(_T("A string")), 0);
	EXPECT_EQ(strValue.Compare(_T("Some string")), 0);
}

TEST(TStringTests, CompareCaseInsensitive_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_LT(strValue.CompareNoCase(_T("the strIng")), 0);
	EXPECT_GT(strValue.CompareNoCase(_T("a sTring")), 0);
	EXPECT_EQ(strValue.CompareNoCase(_T("soMe String")), 0);
}


TEST(TStringTests, CompareCaseSensitive_WithTString)
{
	TString strValue(_T("Some string"));
	EXPECT_LT(strValue.Compare(TString(_T("The string"))), 0);
	EXPECT_GT(strValue.Compare(TString(_T("A string"))), 0);
	EXPECT_EQ(strValue.Compare(TString(_T("Some string"))), 0);
}

TEST(TStringTests, CompareCaseInsensitive_WithTString)
{
	TString strValue(_T("Some string"));
	EXPECT_LT(strValue.CompareNoCase(TString(_T("the strIng"))), 0);
	EXPECT_GT(strValue.CompareNoCase(TString(_T("a sTring"))), 0);
	EXPECT_EQ(strValue.CompareNoCase(TString(_T("soMe String"))), 0);
}

// starts with
TEST(TStringTests, StartsWithCaseSensitive_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue.StartsWith(_T("Some")));
	EXPECT_FALSE(strValue.StartsWith(_T("some")));
}

TEST(TStringTests, StartsWithCaseInsensitive_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue.StartsWithNoCase(_T("Some")));
	EXPECT_TRUE(strValue.StartsWithNoCase(_T("some")));
}

// ends with
TEST(TStringTests, EndsWithCaseSensitive_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue.EndsWith(_T("string")));
	EXPECT_FALSE(strValue.EndsWith(_T("String")));
}

TEST(TStringTests, EndsWithCaseInsensitive_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_TRUE(strValue.EndsWithNoCase(_T("string")));
	EXPECT_TRUE(strValue.EndsWithNoCase(_T("String")));
}

// find first/last of
TEST(TStringTests, FindFirstOf_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.FindFirstOf(_T("er"), 3), 3);
	EXPECT_EQ(strValue.FindFirstOf(_T("er"), 4), 7);
	EXPECT_EQ(strValue.FindFirstOf(_T(""), 0), TString::npos);
	EXPECT_EQ(strValue.FindFirstOf(NULL, 0), TString::npos);
}

TEST(TStringTests, FindLastOf_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.FindLastOf(_T("er")), 7);
	EXPECT_EQ(strValue.FindLastOf(_T("")), TString::npos);
	EXPECT_EQ(strValue.FindLastOf(NULL), TString::npos);
}

// find and replace
TEST(TStringTests, Find_WithPtrToString)
{
	TString strValue(_T("Some string"));
	EXPECT_EQ(strValue.Find(_T("tri"), 6), 6);
	EXPECT_EQ(strValue.Find(_T("tri"), 7), TString::npos);
	EXPECT_EQ(strValue.Find(_T(""), 0), TString::npos);
	EXPECT_EQ(strValue.Find(NULL, 0), TString::npos);
}

TEST(TStringTests, Replace_WithPtrToString)
{
	TString strValue(_T("Some string"));
	strValue.Replace(_T("tri"), _T("o"));
	EXPECT_EQ(strValue, _T("Some song"));
}

TEST(TStringTests, Replace_EmptyStringWithPtrToString)
{
	TString strValue;
	strValue.Replace(_T(""), _T("o"));
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, Replace_EmptyStringWithNullPtrToString)
{
	TString strValue;
	strValue.Replace(_T(""), NULL);
	EXPECT_EQ(strValue, _T(""));
}

// get at
TEST(TStringTests, GetAtWithBool)
{
	TString strValue(_T("Some string"));
	wchar_t wch = 0;

	EXPECT_TRUE(strValue.GetAt(3, wch));
	EXPECT_EQ(wch, _T('e'));

	EXPECT_FALSE(strValue.GetAt(20, wch));
	EXPECT_EQ(wch, _T('\0'));
}

TEST(TStringTests, GetAtWithoutBool)
{
	TString strValue(_T("Some string"));

	EXPECT_EQ(strValue.GetAt(3), _T('e'));
	EXPECT_EQ(strValue.GetAt(20), _T('\0'));
}

// get buffer, release buffer
TEST(TStringTests, GetAndReleaseBuffer)
{
	TString strValue(_T("Some string"));
	wchar_t* pszBuffer = strValue.GetBuffer(5);
	pszBuffer[4] = _T('\0');
	strValue.ReleaseBuffer();
	
	EXPECT_EQ(strValue.GetLength(), 4);
}

TEST(TStringTests, GetAndReleaseBufferSetLength)
{
	TString strValue(_T("Some string"));
	wchar_t* pszBuffer = strValue.GetBuffer(5);
	pszBuffer[4] = _T('t');
	strValue.ReleaseBufferSetLength(1);

	EXPECT_EQ(strValue.GetLength(), 1);
	EXPECT_EQ(strValue, _T("S"));
}

// clear, isempty, getlength
TEST(TStringTests, ClearAndIsEmpty)
{
	TString strValue(_T("Some string"));

	strValue.Clear();

	EXPECT_EQ(strValue, _T(""));
	EXPECT_TRUE(strValue.IsEmpty());
}

TEST(TStringTests, GetLength)
{
	TString strValue(_T("Some string"));

	EXPECT_EQ(strValue.GetLength(), 11);
}

// corner cases and detected bugs
TEST(TStringTests, AssignEmptyStringToAlreadyInitializedTString)
{
	TString strValue(_T("Some string"));

	// with this we had infinite recurrence
	EXPECT_NO_THROW(strValue = _T(""));
	EXPECT_EQ(strValue, _T(""));
}

TEST(TStringTests, TrimRightSelf)
{
	TString strValue(_T("Some string"));

	strValue.TrimRightSelf(L"gn");
	EXPECT_EQ(strValue, L"Some stri");
}

TEST(TStringTests, ReplaceAffectingOtherStringInstancesBug)
{
	TString strValue(_T("Some string"));
	TString strSecond(strValue);

	strValue.Replace(_T("Some "), _T("No "));

	EXPECT_EQ(strSecond, _T("Some string"));
}
