// UsefulCPU.cpp
// CPU-architecture related code
// Written on 10/24/07 by FPW2

#include "stdafx.h"
#include "UsefulCPU.h"

/*===========================================================================*/

CPU_Caps::CPU_Caps()
{

// get the manufacturer name, stepping, etc. (not that we really care)
__cpuid(cpuInfo, 0);
*(int *)vendorString = cpuInfo[1];
*(int *)(vendorString + 4) = cpuInfo[3];
*(int *)(vendorString + 8) = cpuInfo[2];
vendorString[12] = 0;

cpuInfo[3] = cpuInfo[2] = cpuInfo[1] = cpuInfo[0] = 0;
cpuInfo2[3] = cpuInfo2[2] = cpuInfo2[1] = cpuInfo2[0] = 0;

// get the CPU capabilities
__cpuid(cpuInfo, 1);
__cpuid(cpuInfo2, 0x80000001);	//lint !e569

memset(capString, 0, sizeof(capString));
if (Has_SSE())
	strcat(capString, "CPU has: SSE");
if (Has_SSE2())
	strcat(capString, ", SSE2");
if (Has_SSE3())
	strcat(capString, ", SSE3");
if (Has_SSSE3())
	strcat(capString, ", SSSE3");
if (Has_SSE4A())
	strcat(capString, ", SSE4a");
if (Has_SSE41())
	strcat(capString, ", SSE4.1");
if (Has_SSE42())
	strcat(capString, ", SSE4.2");
if (Has_POPCNT())
	strcat(capString, ", POPCNT");

} // CPU_Caps::CPU_Caps

/*===========================================================================*/

CPU_Caps::~CPU_Caps()
{
} // CPU_Caps::~CPU_Caps

/*===========================================================================*/

bool CPU_Caps::Has_POPCNT(void)
{
bool rVal = false;

if (cpuInfo[2] & 0x00800000)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_POPCNT

/*===========================================================================*/

bool CPU_Caps::Has_SSE(void)
{
bool rVal = false;

if (cpuInfo[3] & 0x02000000)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE

/*===========================================================================*/

bool CPU_Caps::Has_SSE2(void)
{
bool rVal = false;

if (cpuInfo[3] & 0x04000000)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE2

/*===========================================================================*/

bool CPU_Caps::Has_SSE3(void)
{
bool rVal = false;

if (cpuInfo[2] & 0x00000001)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE3

/*===========================================================================*/

bool CPU_Caps::Has_SSSE3(void)
{
bool rVal = false;

if (cpuInfo[2] & 0x00000200)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE3

/*===========================================================================*/

bool CPU_Caps::Has_SSE4A(void)
{
bool rVal = false;

if (cpuInfo[2] & 0x00000040)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE4A

/*===========================================================================*/

bool CPU_Caps::Has_SSE41(void)
{
bool rVal = false;

if (cpuInfo[2] & 0x00080000)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE41

/*===========================================================================*/

bool CPU_Caps::Has_SSE42(void)
{
bool rVal = false;

if (cpuInfo[2] & 0x00100000)
	rVal = true;

return(rVal);

} // CPU_Caps::Has_SSE42

/*===========================================================================*/
