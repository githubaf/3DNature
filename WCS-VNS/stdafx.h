//#ifdef _WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Vista uses this variant (NTDDI_WIN2K = 0x05000000)
//#include <sdkddkver.h>                       
//#define NTDDI_VERSION	NTDDI_WIN2K	// Target the original Windows 2000 or newer

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <shlobj.h>	// for SHCreateDirectoryEx function
#include <basetsd.h> // for 64 bit numbers
#include <shellapi.h>

//#endif // _WIN32

#include <algorithm>
#include <assert.h>
#include <cassert>
#include <conio.h>
#include <ctype.h>
#include <direct.h>
#include <errno.h>
#include <float.h>
#include <intrin.h>
#include <limits.h>
#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
//#include <pmmintrin.h> // for SSE3
#include <process.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <winsock.h>
#include <xutility>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _OPENMP
#include <omp.h>
#endif // _OPENMP
