#pragma once

namespace chengine
{
	enum ECompareType
	{
		eCmp_Less = 0,
		eCmp_LessOrEqual = 1,
		eCmp_Equal = 2,
		eCmp_GreaterOrEqual = 3,
		eCmp_Greater = 4,
		eCmp_NotEqual = 5,

		eCmp_Last
	};

	template<class T>
	bool CompareByType(const T& value1, const T& value2, ECompareType eCmpType)
	{
		switch(eCmpType)
		{
		case eCmp_Less:
			return value1 < value2;
		case eCmp_LessOrEqual:
			return value1 <= value2;
		case eCmp_Equal:
			return value1 == value2;
		case eCmp_GreaterOrEqual:
			return value1 >= value2;
		case eCmp_Greater:
			return value1 > value2;
		case eCmp_NotEqual:
			return value1 != value2;
		}

		throw std::runtime_error("Invalid compare type");
	}
}
