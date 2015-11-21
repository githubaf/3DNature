// CPUSupport.cpp
// Code to muck about with the CPU and tell us things about it and the like
// Created from scratch on 1/2/06 by CXH

#include "CPUSupport.h"

#include <windows.h>

unsigned int  HTSupported(void);
unsigned char LogicalProcPerPhysicalProc(void);

static unsigned long int CachedNumberOfCPUs;

unsigned long int QueryNumberOfRealCPUs(void)
{
unsigned long int Result = 0;
SYSTEM_INFO SysInfo;

if(CachedNumberOfCPUs) return(CachedNumberOfCPUs);

GetSystemInfo(&SysInfo);

Result = SysInfo.dwNumberOfProcessors;

if(Result > 1)
	{
	// detect HyperThreading and decrease CPUs by half to account for it
	if (HTSupported() && (LogicalProcPerPhysicalProc() > 1)) // neglects to detect if HT is on and supported by OS
		{
		Result = Result >> 1; // halve the number of CPUs
		} // if
	} // if

return(CachedNumberOfCPUs = Result);
} // QueryNumberOfRealCPUs

// The code below is excerpted unmodified:
//-------------------------------------------------------------------------------------------------
//
// Copyright © 2001, Intel Corporation . Other brands and names may be claimed as the property of others. 
//
//
// CPU Counting Utility
// Date   : 10/30/2001
// Version: 1.4
// 
//
//
// File Name: CPUCount.cpp

#define HT_BIT             0x10000000     // EDX[28]  Bit 28 is set if HT is supported
#define FAMILY_ID          0x0F00         // EAX[11:8] Bit 8-11 contains family processor ID.
#define PENTIUM4_ID        0x0F00         
#define EXT_FAMILY_ID      0x0F00000      // EAX[23:20] Bit 20-23 contains extended family processor ID
#define NUM_LOGICAL_BITS   0x00FF0000     // EBX[23:16] Bit 16-23 in ebx contains the number of logical
                                          // processors per physical processor when execute cpuid with 
                                          // eax set to 1

#define INITIAL_APIC_ID_BITS  0xFF000000  // EBX[31:24] Bits 24-31 (8 bits) return the 8-bit unique 
                                          // initial APIC ID for the processor this code is running on.
                                          // Default value = 0xff if HT is not supported


// Status Flag
#define HT_NOT_CAPABLE           0
#define HT_ENABLED               1
#define HT_DISABLED              2
#define HT_SUPPORTED_NOT_ENABLED 3
#define HT_CANNOT_DETECT         4


unsigned int HTSupported(void)
{
   

	unsigned int Regedx      = 0,
		         Regeax      = 0,
		         VendorId[3] = {0, 0, 0};

	__try    // Verify cpuid instruction is supported
	{
		__asm
		{
			xor eax, eax          // call cpuid with eax = 0
        	cpuid                 // Get vendor id string
			mov VendorId, ebx
			mov VendorId + 4, edx
			mov VendorId + 8, ecx
			
			mov eax, 1            // call cpuid with eax = 1
			cpuid
			mov Regeax, eax      // eax contains family processor type
			mov Regedx, edx      // edx has info about the availability of hyper-Threading
 
		}
	}

	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return(0);                   // cpuid is unavailable
	}

    if (((Regeax & FAMILY_ID) ==  PENTIUM4_ID) || 
		(Regeax & EXT_FAMILY_ID))
	  if (VendorId[0] == 'uneG')
		if (VendorId[1] == 'Ieni')
			if (VendorId[2] == 'letn')
				return(Regedx & HT_BIT);    // Genuine Intel with hyper-Threading technology

	return 0;    // Not genuine Intel processor
  
}


unsigned char LogicalProcPerPhysicalProc(void)
{

	unsigned int Regebx = 0;
	if (!HTSupported()) return (unsigned char) 1;  // HT not supported
	                                               // Logical processor = 1
	__asm
	{
		mov eax, 1
		cpuid
		mov Regebx, ebx
	}

	return (unsigned char) ((Regebx & NUM_LOGICAL_BITS) >> 16);

}
