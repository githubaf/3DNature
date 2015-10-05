// UsefulCPU.h
// CPU-architecture related code from Useful.h
// Written on 10/24/07 by FPW2

#ifndef WCS_USEFULCPU_H
#define WCS_USEFULCPU_H

class CPU_Caps
	{
	private:
		int  cpuInfo[4];	// 0 = eax, 1 = ebx, 2 = ecx, 3 = edx
		int  cpuInfo2[4];	// 0 = eax, 1 = ebx, 2 = ecx, 3 = edx
		char capString[80];
		char vendorString[16];

	public:
		CPU_Caps();
		~CPU_Caps();

		bool Has_POPCNT(void);
		bool Has_SSE(void);
		bool Has_SSE2(void);
		bool Has_SSE3(void);
		bool Has_SSSE3(void);
		bool Has_SSE4A(void);
		bool Has_SSE41(void);
		bool Has_SSE42(void);
		const char *GetCapString(void) { return(capString);};
	}; // CPU_Caps

#endif // WCS_USEFULCPU_H
