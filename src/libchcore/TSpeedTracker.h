#ifndef __TSPEEDTRACKER_H__
#define __TSPEEDTRACKER_H__

#include "TString.h"

namespace chcore
{
	class TSpeedTracker
	{
	public:
		TSpeedTracker(unsigned long long ullTrackTime, unsigned long long ullSampleTime);
		TSpeedTracker(const TSpeedTracker&) = delete;

		TSpeedTracker& operator=(const TSpeedTracker&) = delete;

		void AddSample(unsigned long long ullValue, unsigned long long ullTimestamp);
		void Clear();

		// retrieves speed per second
		double GetSpeed() const;

		TString ToString() const;
		void FromString(const TString& strData);

	private:
		static double NormalizeValueByTime(unsigned long long ullValue, unsigned long long ullTime, unsigned long long ullNormalizeTime = 1000);

		size_t GetNextSampleIndexAndIncrease();
		void FinalizeIncompleteSample(double dSpeed, unsigned long long& ullInterval);
		void AddCompleteSamples(double dSpeed, unsigned long long& ullInterval);
		void PrepareIncompleteSample(unsigned long long ullInterval, double dSpeed);
		double CalculateIncompleteSampleNormalizedSpeed() const;

	private:
		// initialized in constructor (does not change throughout the whole lifetime)
		const size_t m_stRequiredSamples;				// how many samples of m_ullSampleTime do we want to keep?
		const unsigned long long m_ullSampleTime;		// interval covered by a single sample
		const double m_dSamplesPerSecond;				// how many samples fit in one second

		// vector of samples with pointer to the next element to be filled
		std::vector<double> m_vSamples;		// speed per sample
		size_t m_stNextSamplePos;				// points to the element with the oldest sample

		unsigned long long m_ullLastTimestamp;		// last time some sample was processed

		double m_dPartialSpeedNotInSamples;		// specifies count of data processed in the m_ullTimeIntervalNotInSamples interval
		unsigned long long m_ullTimeIntervalNotInSamples;	// interval that was not enough to add m_ullDataNotInSamples to samples
		unsigned long long m_ullZeroIntervalData;
	};
}

#endif
