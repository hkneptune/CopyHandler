#include "stdafx.h"
#include "MathFunctions.h"

namespace chcore
{
	namespace Math
	{
		double Div64(unsigned long long ullNumber, unsigned long long ullDenominator)
		{
			if (ullDenominator == 0)
				return 0.0;

			return (double)ullNumber / (double)ullDenominator;
		}

		LIBCHCORE_API double Div64(unsigned long long ullNumber, double dDenominator)
		{
			return ullNumber / dDenominator;
		}
	}
}
