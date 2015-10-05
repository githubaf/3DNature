// UsefulTime.cpp
// Time-related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulTime.h"

/*===========================================================================*/

void GetTime(time_t &SetMe)
{

time(&SetMe);

} // GetTime

/*===========================================================================*/

double GetSystemTimeFP(void)
{
double Now;
struct _timeb TimeBuf;

_ftime(&TimeBuf);
Now = TimeBuf.time + TimeBuf.millitm * (1.0 / 1000.0); // one thousandth
return(Now);
} // GetSystemTimeFP

/*===========================================================================*/

// May fall through to GetSystemTimeFP on systems that don't provide 
// process accounting APIs
double GetProcessTimeFP(void)
{
double Now;
FILETIME Create, Exit, Kernel, User;
LARGE_INTEGER HugeNum;

GetProcessTimes(GetCurrentProcess(), &Create, &Exit, &Kernel, &User);
HugeNum.LowPart = User.dwLowDateTime;
HugeNum.HighPart = User.dwHighDateTime;
// Filetime is in units of 100ns, convert to decimal seconds by dividing by 10 million
Now = ((double)HugeNum.QuadPart) * 0.0000001;

return(Now);
} // GetProcessTimeFP

/*===========================================================================*/

const char *GetTimeString(time_t &MyTime)
{

return (ctime(&MyTime));

} // GetTimeString

/*===========================================================================*/


// timer functions intended for profiling only - adapted from M$ notes
LARGE_INTEGER TicksPerSec;
LARGE_INTEGER HiResStartTime, HiResStopTime;
double HiResElapsedTime = 0.0, TotalTime = 0.0;
unsigned int HiResElapsedTicks = 0, TotalTicks = 0;
int PerfFreqAdjust;


void StartHiResTimer(void)
{
int High32;

QueryPerformanceFrequency(&TicksPerSec);

HiResElapsedTime = 0.0;
HiResElapsedTicks = 0;
PerfFreqAdjust = 0;
High32 = TicksPerSec.HighPart;
while (High32)
	{
	High32 /= 2;
	PerfFreqAdjust++;
	} // while

// you really don't want to do this normally
//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
QueryPerformanceCounter(&HiResStartTime);
} // StartHiResTimer

/*===========================================================================*/


double StopHiResTimerSecs(void)
{
LARGE_INTEGER Freq = TicksPerSec;
double time;
unsigned int High32;
int ReduceMag = 0;

QueryPerformanceCounter(&HiResStopTime);
//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

High32 = HiResStopTime.HighPart - HiResStartTime.HighPart;
while (High32)
	{
	High32 >>= 1;
	ReduceMag++;
	} // while
if (PerfFreqAdjust || ReduceMag)
	{
	if (PerfFreqAdjust > ReduceMag)
		ReduceMag = PerfFreqAdjust;
	HiResStartTime.QuadPart = Int64ShrlMod32(HiResStartTime.QuadPart, ReduceMag);
	HiResStopTime.QuadPart = Int64ShrlMod32(HiResStopTime.QuadPart, ReduceMag);
	Freq.QuadPart = Int64ShrlMod32(Freq.QuadPart, ReduceMag);
	} // if
if (Freq.LowPart == 0)
	time = 0.0;
else
	time = ((double)(HiResStopTime.LowPart - HiResStartTime.LowPart)) / Freq.LowPart;

HiResElapsedTime = time;	// the compiler does a good job of optimizing out what we don't want optimized :)
return time;	// elapsed time in seconds
} // StopHiResTimerSecs

/*===========================================================================*/

unsigned int StopHiResTimer(void)
{
LARGE_INTEGER Freq = TicksPerSec;
unsigned int High32, Ticks;
int ReduceMag = 0;

QueryPerformanceCounter(&HiResStopTime);
//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

High32 = HiResStopTime.HighPart - HiResStartTime.HighPart;
while (High32)
	{
	High32 >>= 1;
	ReduceMag++;
	} // while
if (PerfFreqAdjust || ReduceMag)
	{
	if (PerfFreqAdjust > ReduceMag)
		ReduceMag = PerfFreqAdjust;
	HiResStartTime.QuadPart = Int64ShrlMod32(HiResStartTime.QuadPart, ReduceMag);
	HiResStopTime.QuadPart = Int64ShrlMod32(HiResStopTime.QuadPart, ReduceMag);
	Freq.QuadPart = Int64ShrlMod32(Freq.QuadPart, ReduceMag);
	} // if
	Ticks  = HiResStopTime.LowPart - HiResStartTime.LowPart;

HiResElapsedTicks = Ticks;	// the compiler does a good job of optimizing out what we don't want optimized :)
return Ticks;	// elapsed time in ticks
} // StopHiResTimer
