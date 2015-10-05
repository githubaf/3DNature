// UsefulTime.h
// Time-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_USEFULTIME_H
#define WCS_USEFULTIME_H

double GetSystemTimeFP(void);
// May fall through to GetSystemTimeFP on systems that don't provide 
// process accounting APIs
double GetProcessTimeFP(void);

void GetTime(time_t &SetMe);
const char *GetTimeString(time_t &MyTime);

// timer functions intended for profiling only
void StartHiResTimer(void);
double StopHiResTimerSecs(void);
unsigned int StopHiResTimer(void);

#endif // WCS_USEFULTIME_H
