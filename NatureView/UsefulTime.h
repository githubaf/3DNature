// UsefulTime.h
// Time-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULTIME_H
#define WCS_USEFULTIME_H

double GetSystemTimeFP(void);
// May fall through to GetSystemTimeFP on systems that don't provide 
// process accounting APIs
double GetProcessTimeFP(void);

void GetTime(unsigned long &SetMe);
const char *GetTimeString(unsigned long &MyTime);

// timer functions intended for profiling only
void StartHiResTimer(void);
double StopHiResTimerSecs(void);
unsigned int StopHiResTimer(void);

#endif // WCS_USEFULTIME_H
