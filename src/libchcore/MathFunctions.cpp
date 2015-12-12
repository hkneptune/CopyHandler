#include "stdafx.h"
#include "MathFunctions.h"
#include <boost/numeric/conversion/cast.hpp>

namespace chcore
{
	namespace Math
	{
		double Div64(unsigned long long ullNumber, unsigned long long ullDenominator)
		{
			if (ullDenominator == 0)
				return 0.0;

			const unsigned long long ullMaxInt32 = (unsigned long long)std::numeric_limits<int>::max();
			while (ullNumber > ullMaxInt32 || ullDenominator > ullMaxInt32)
			{
				ullNumber >>= 1;
				ullDenominator >>= 1;
			}

			return boost::numeric_cast<double>(ullNumber) / boost::numeric_cast<double>(ullDenominator);
		}
	}
}
