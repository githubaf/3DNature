// FrameStats.cpp

#include "FrameStats.h"


NVFrameStats FrameStatisticsCounter;
NVFrameStats DrawStatisticsCounter;

double NVFrameStats::FetchAverageTime(void)
{
if(QueryAdequateSamples())
	{
	double FrameTimeSum = 0.0f;
	std::vector<double>::iterator FrameWalker;
	for(FrameWalker = FrameHistory.begin(); FrameWalker != FrameHistory.end(); FrameWalker++)
		{
		FrameTimeSum += *FrameWalker;
		} // for
	if(FrameTimeSum > 0)
		{
		return(FrameTimeSum / (double)NVW_FRAMESTATS_SAMPLES_MAX);
		} // if
	else
		{
		return(0.0f);
		} // else
	} // if
else
	{
	return(0.0f);
	} // else
} // NVFrameStats::FetchAverageTime
