#include "stdafx.h"
#include "TSpeedTracker.h"
#include <boost/numeric/conversion/cast.hpp>
#include <numeric>
#include "ErrorCodes.h"
#include "TCoreException.h"
#include "MathFunctions.h"
#include <boost/lexical_cast.hpp>
#include "TStringArray.h"

namespace chcore
{
	TSpeedTracker::TSpeedTracker(unsigned long long ullTrackTime, unsigned long long ullSampleTime) :
		m_stRequiredSamples(ullSampleTime ? boost::numeric_cast<size_t>(ullTrackTime / ullSampleTime) : 0),
		m_ullSampleTime(ullSampleTime),
		m_dSamplesPerSecond(ullSampleTime != 0 ? 1000.0 / ullSampleTime : 0.0),
		m_dPartialSpeedNotInSamples(0),
		m_ullTimeIntervalNotInSamples(0),
		m_stNextSamplePos(0),
		m_ullLastTimestamp(std::numeric_limits<unsigned long long>::max()),
		m_ullZeroIntervalData(0)
	{
		if(ullSampleTime == 0)
			throw TCoreException(eErr_InvalidArgument, L"ullSampleTime", LOCATION);
		if(m_stRequiredSamples == 0)
			throw TCoreException(eErr_InvalidArgument, L"m_stRequiredSamples", LOCATION);
		std::fill_n(std::inserter(m_vSamples, m_vSamples.end()), m_stRequiredSamples, 0.0);
	}

	void TSpeedTracker::Clear()
	{
		m_dPartialSpeedNotInSamples = 0;
		m_ullTimeIntervalNotInSamples = 0;
		m_stNextSamplePos = 0;
		m_ullLastTimestamp = std::numeric_limits<unsigned long long>::max();
		m_ullZeroIntervalData = 0;
		std::fill(m_vSamples.begin(), m_vSamples.end(), 0.0);
	}

	void TSpeedTracker::AddSample(unsigned long long ullValue, unsigned long long ullTimestamp)
	{
		// if this is the first sample ever added (after construction or after clear) then 
		// we don't have time interval yet - just remember the timestamp and ignore value
		if (m_ullLastTimestamp == std::numeric_limits<unsigned long long>::max())
		{
			m_ullLastTimestamp = ullTimestamp;
			return;
		}

		// sanity check - make sure the data is valid
		if (ullTimestamp < m_ullLastTimestamp)
			throw TCoreException(eErr_InvalidArgument, L"ullTimestamp", LOCATION);

		// calculate the interval since the time last sample was added
		unsigned long long ullInterval = ullTimestamp - m_ullLastTimestamp;
		m_ullLastTimestamp = ullTimestamp;

		if (ullInterval == 0)		// special case 0: if interval is 0 - put the data sample somewhere for future use
		{
			m_ullZeroIntervalData += ullValue;
			return;
		}
		else if (ullInterval >= m_ullSampleTime * m_stRequiredSamples)	// special case 1: interval is bigger than what we track
		{
			m_stNextSamplePos = 0;
			m_dPartialSpeedNotInSamples = 0.0;
			m_ullTimeIntervalNotInSamples = 0;
			m_ullZeroIntervalData = 0;

			double dSpeed = NormalizeValueByTime(ullValue, ullInterval, m_ullSampleTime);
			std::fill(m_vSamples.begin(), m_vSamples.end(), dSpeed);
			return;
		}
		else
		{
			// append the data from previous zero-interval samples
			ullValue += m_ullZeroIntervalData;
			m_ullZeroIntervalData = 0;
		}

		// calculate speed
		double dSpeed = NormalizeValueByTime(ullValue, ullInterval, m_ullSampleTime);

		// finalize the incomplete sample and adjust the input data
		FinalizeIncompleteSample(dSpeed, ullInterval);

		// deal with the full samples
		AddCompleteSamples(dSpeed, ullInterval);

		// and finally prepare the incomplete sample data for future use
		PrepareIncompleteSample(ullInterval, dSpeed);
	}

	double TSpeedTracker::GetSpeed() const
	{
		double dResult = 0.0;

		if (m_ullTimeIntervalNotInSamples != 0)
		{
			dResult = CalculateIncompleteSampleNormalizedSpeed();

			for (size_t stIndex = 0; stIndex < m_vSamples.size(); ++stIndex)
			{
				if (stIndex != m_stNextSamplePos)
					dResult += m_vSamples[stIndex];
			}
		}
		else
			dResult = std::accumulate(m_vSamples.begin(), m_vSamples.end(), 0.0);

		return dResult / m_vSamples.size() * m_dSamplesPerSecond;
	}

	size_t TSpeedTracker::GetNextSampleIndexAndIncrease()
	{
		size_t stResult = m_stNextSamplePos++;
		if (m_stNextSamplePos >= m_vSamples.size())
			m_stNextSamplePos = 0;
		return stResult;
	}

	double TSpeedTracker::NormalizeValueByTime(unsigned long long ullValue, unsigned long long ullTime, unsigned long long ullNormalizeTime)
	{
		return Math::Div64(ullNormalizeTime, ullTime) * ullValue;
	}

	void TSpeedTracker::FinalizeIncompleteSample(double dSpeed, unsigned long long& ullInterval)
	{
		if (m_ullTimeIntervalNotInSamples == 0 || m_dPartialSpeedNotInSamples == 0.0 || ullInterval == 0)
			return;

		// how much data do we need?
		unsigned long long ullIntervalStillNeeded = m_ullSampleTime - m_ullTimeIntervalNotInSamples;
		if (ullInterval < ullIntervalStillNeeded)
		{
			// we can only add the next partial sample, but cannot finalize
			m_ullTimeIntervalNotInSamples += ullInterval;
			m_dPartialSpeedNotInSamples += dSpeed * Math::Div64(ullInterval, m_ullSampleTime);
			ullInterval = 0;
		}
		else
		{
			// going to finalize sample
			m_dPartialSpeedNotInSamples += dSpeed * Math::Div64(ullIntervalStillNeeded, m_ullSampleTime);

			m_vSamples[GetNextSampleIndexAndIncrease()] = m_dPartialSpeedNotInSamples;
			ullInterval -= ullIntervalStillNeeded;

			m_dPartialSpeedNotInSamples = 0.0;
			m_ullTimeIntervalNotInSamples = 0;
		}
	}

	void TSpeedTracker::AddCompleteSamples(double dSpeed, unsigned long long& ullInterval)
	{
		size_t stSamplesCount = boost::numeric_cast<size_t>(std::min(ullInterval / m_ullSampleTime, (unsigned long long)m_stRequiredSamples));

		// fill the container with full samples
		while (stSamplesCount--)
		{
			m_vSamples[GetNextSampleIndexAndIncrease()] = dSpeed;
		}

		ullInterval = ullInterval % m_ullSampleTime;
	}

	double TSpeedTracker::CalculateIncompleteSampleNormalizedSpeed() const
	{
		// get the speed for incomplete sample
		double dIncompleteSamplePercentage = Math::Div64(m_ullTimeIntervalNotInSamples, m_ullSampleTime);
		double dResult = m_dPartialSpeedNotInSamples + (1.0 - dIncompleteSamplePercentage) * m_vSamples[m_stNextSamplePos];

		return dResult;
	}

	void TSpeedTracker::PrepareIncompleteSample(unsigned long long ullInterval, double dSpeed)
	{
		if (ullInterval > 0)
		{
			// we can only add the next partial sample, but cannot finalize
			m_ullTimeIntervalNotInSamples = ullInterval;
			m_dPartialSpeedNotInSamples = dSpeed * Math::Div64(ullInterval, m_ullSampleTime);
		}
	}

	TString TSpeedTracker::ToString() const
	{
		TString strData;

		strData += boost::lexical_cast<std::wstring>(m_stNextSamplePos).c_str();
		strData += _T(";");

		strData += boost::lexical_cast<std::wstring>(m_dPartialSpeedNotInSamples).c_str();
		strData += _T(";");
		strData += boost::lexical_cast<std::wstring>(m_ullTimeIntervalNotInSamples).c_str();
		strData += _T(";");
		strData += boost::lexical_cast<std::wstring>(m_ullZeroIntervalData).c_str();
		strData += _T(";");

		for(double dVal : m_vSamples)
		{
			strData += boost::lexical_cast<std::wstring>(dVal).c_str();
			strData += _T(";");
		}

		strData.TrimRightSelf(_T(";"));

		return strData;
	}

	void TSpeedTracker::FromString(const TString& strData)
	{
		TStringArray arrStrings;
		strData.Split(_T(";"), arrStrings);

		const size_t SerializedMembers = 4;
		if (arrStrings.GetCount() != m_stRequiredSamples + SerializedMembers)
			throw TCoreException(eErr_InvalidArgument, L"strData", LOCATION);

		Clear();

		m_stNextSamplePos = boost::lexical_cast<size_t>(arrStrings.GetAt(0).c_str());
		m_dPartialSpeedNotInSamples = boost::lexical_cast<double>(arrStrings.GetAt(1).c_str());
		m_ullTimeIntervalNotInSamples = boost::lexical_cast<unsigned long long>(arrStrings.GetAt(2).c_str());
		m_ullZeroIntervalData = boost::lexical_cast<unsigned long long>((PCTSTR)arrStrings.GetAt(3).c_str());

		for (size_t stIndex = 4; stIndex < arrStrings.GetCount(); ++stIndex)
		{
			m_vSamples[stIndex - 4] = boost::lexical_cast<double>(arrStrings.GetAt(stIndex).c_str());
		}
	}
}
