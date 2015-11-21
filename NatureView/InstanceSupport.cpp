// InstanceSupport.cpp
// Global junk to prevent having to pass the stoopid Win32 INSTANCE
// around everywhere we might need it.

#include "InstanceSupport.h"

static HINSTANCE _GlobalInstance;

void SetHInstance(HINSTANCE NewInstance)
{
_GlobalInstance = NewInstance;
} // SetHInstance

HINSTANCE GetHInstance(void)
{
return(_GlobalInstance);
} // GetHInstance
