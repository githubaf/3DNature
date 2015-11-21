// FrameStats.h

#ifndef NVW_FRAMESTATS_H
#define NVW_FRAMESTATS_H

#include <vector>

#define NVW_FRAMESTATS_SAMPLES_MAX	15

class NVFrameStats
	{
	private:
		std::vector<double> FrameHistory;
		int NumValidFrames;
		int LastSubScript;

	public:
		bool QueryAdequateSamples(void) {return(NumValidFrames == NVW_FRAMESTATS_SAMPLES_MAX);};
		NVFrameStats() {FrameHistory.resize(NVW_FRAMESTATS_SAMPLES_MAX); NumValidFrames = 0; LastSubScript = 0;};
		~NVFrameStats() {};

		void AddNewSample(double NewValue) {FrameHistory[(++LastSubScript) % NVW_FRAMESTATS_SAMPLES_MAX] = NewValue; if(NumValidFrames < NVW_FRAMESTATS_SAMPLES_MAX) NumValidFrames++;};
		double FetchLastSample(void) {if(NumValidFrames > 0) return(FrameHistory[LastSubScript]); else return(0.0f);};
		double FetchAverageTime(void);
	}; // NVFrameStats

#endif // NVW_FRAMESTATS_H
