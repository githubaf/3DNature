// InstanceSupport.h
// Global junk to prevent having to pass the stoopid Win32 INSTANCE
// around everywhere we might need it.

#include <windows.h>


void SetHInstance(HINSTANCE NewInstance);
HINSTANCE GetHInstance(void);
